
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include "Graph.hpp"
#include "Algorithms.hpp"
#include "Factory.hpp"   // AlgorithmFactory::createAlgorithm

static const char* SOCKET_PATH = "mysocket";

std::string graphToString(const Graph& g) {
    std::ostringstream os;
    for (int i = 0; i < g.getNumVertices(); ++i) {
        os << "Vertex " << i << ":";
        for (auto neighbor : g.getNeighbors(i)) {
            os << " " << neighbor.first << "(w=" << neighbor.second << ")";
        }
        os << "\n";
    }
    return os.str();
}


// המרת מחרוזת לאותיות גדולות 
static std::string to_upper(std::string s) {
    for (char& c : s) c = char(std::toupper((unsigned char)c));
    return s;
}

//  עזר: טקסט עזרה 
static std::string usage_text(const char* prog_name) {
    std::ostringstream os;
    os << "Usage: " << prog_name << " [OPTIONS]\n"
       << "\nOptions:\n"
       << "  -a <algorithm>  Algorithm to run:\n"
       << "                  EULERIAN    - Find Eulerian circuit (responds YES/NO)\n"
       << "                  SCC         - Strongly Connected Components\n"
       << "                  MST         - Minimum Spanning Tree\n"
       << "                  MAXCLIQUE   - Maximum Clique\n"
       << "                  HAMILTON    - Hamilton Circuit\n"
       << "  -v <vertices>   Number of vertices (for random graph)\n"
       << "  -e <edges>      Number of edges (for random graph)\n"
       << "  -s <seed>       Random seed\n"
       << "  -h              Show this help\n"
       << "\nExamples:\n"
       << "  " << prog_name << " -a EULERIAN -v 5 -e 6 -s 42\n"
       << "  " << prog_name << " -a SCC -v 4 -e 5 -s 123\n"
       << "  " << prog_name << " -a MST -v 6 -e 8 -s 456\n"
       << "  " << prog_name << " -a MAXCLIQUE -v 5 -e 7 -s 789\n"
       << "  " << prog_name << " -a HAMILTON -v 4 -e 6 -s 101\n";
    return os.str();
}

static std::string handle_request_text(const std::string& request) {
    std::istringstream iss(request);
    std::vector<std::string> args;
    std::string tok;
    while (iss >> tok) args.push_back(tok);

    const char* progname = "algorithm-server";
    if (args.empty()) {
        return "ERROR: Empty request\n" + usage_text(progname);
    }

    // דגלים
    std::string alg;
    int V = 0, E = 0, S = 0;
    bool hasA=false, hasV=false, hasE=false, hasS=false, wantHelp=false;

    for (size_t i=0; i<args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "-h" || a == "--help") {
            wantHelp = true;
        } else if (a == "-a" && i+1 < args.size()) {
            alg = to_upper(args[++i]);
            hasA = true;
        } else if (a == "-v" && i+1 < args.size()) {
            try { V = std::stoi(args[++i]); hasV = true; }
            catch(...) { return "ERROR: invalid value for -v\n"; }
        } else if (a == "-e" && i+1 < args.size()) {
            try { E = std::stoi(args[++i]); hasE = true; }
            catch(...) { return "ERROR: invalid value for -e\n"; }
        } else if (a == "-s" && i+1 < args.size()) {
            try { S = std::stoi(args[++i]); hasS = true; }
            catch(...) { return "ERROR: invalid value for -s\n"; }
        } else if (a.rfind("-",0)==0) {
            return "ERROR: unknown option '" + a + "'\n" + usage_text(progname);
        } else {
        }
    }

    if (wantHelp) {
        return usage_text(progname);
    }

    if (!hasA) return "ERROR: missing -a <algorithm>\n" + usage_text(progname);
    if (!hasV || !hasE || !hasS)
        return "ERROR: missing one of -v/-e/-s (random graph required)\n" + usage_text(progname);

    if (V <= 0) return "ERROR: V must be positive\n";
    if (E < 0)  return "ERROR: E cannot be negative\n";

    //נדרש גרף מכוון עבור אלגוריתם SCC
    bool directed = (alg == "SCC");

    // בניית גרף אקראי דרך Graph::generateRandomGraph 
    Graph g = Graph::generateRandomGraph(V, E, S, directed);
    std::string graphStr = graphToString(g);


   
    auto algo = AlgorithmFactory::createAlgorithm(alg);
    if (!algo) {
        return "ERROR: Unknown algorithm '" + alg + "'\n" + usage_text(progname);
    }

    try {
        std::string out = algo->run(g);
        if (out.empty()) out = "OK\n";
        if (out.back() != '\n') out.push_back('\n');
        return graphStr + out;  // מצרפים את הגרף + תוצאה
    } catch (const std::exception& ex) {
        return std::string("ERROR: exception while running algorithm: ") + ex.what() + "\n";
    } catch (...) {
        return "ERROR: unknown exception while running algorithm\n";
    }
}

int main() {
    fd_set master, read_fds;
    int fdmax, listener, newfd;
    struct sockaddr_un local{}, remote{};
    socklen_t addrlen;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    ::unlink(SOCKET_PATH);

    listener = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (listener < 0) { perror("socket"); return 1; }

    local.sun_family = AF_UNIX;
    std::strncpy(local.sun_path, SOCKET_PATH, sizeof(local.sun_path) - 1);

    if (::bind(listener, reinterpret_cast<struct sockaddr*>(&local), sizeof(local)) < 0) {
        perror("bind");
        ::close(listener);
        return 1;
    }
    if (::listen(listener, 10) == -1) {
        perror("listen");
        ::close(listener);
        return 1;
    }

    FD_SET(listener, &master);
    fdmax = listener;

    std::printf("Algorithm Server is listening on UNIX socket: %s\n", SOCKET_PATH);

    for (;;) {
        read_fds = master;
        if (::select(fdmax + 1, &read_fds, nullptr, nullptr, nullptr) == -1) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        for (int i = 0; i <= fdmax; ++i) {
            if (!FD_ISSET(i, &read_fds)) continue;

            if (i == listener) {
                addrlen = sizeof(remote);
                newfd = ::accept(listener, reinterpret_cast<struct sockaddr*>(&remote), &addrlen);
                if (newfd == -1) { perror("accept"); continue; }
                FD_SET(newfd, &master);
                if (newfd > fdmax) fdmax = newfd;
                std::printf("New client connected on socket %d\n", newfd);
            } else {
                std::string request;
                char rbuf[4096];

                for (;;) {
                    ssize_t n = ::recv(i, rbuf, sizeof(rbuf), 0);
                    if (n == 0) break; // EOF
                    if (n < 0) {
                        if (errno == EINTR) continue;
                        perror("recv");
                        break;
                    }
                    request.append(rbuf, rbuf + n);
                }

                if (request.empty()) {
                    ::close(i);
                    FD_CLR(i, &master);
                    std::printf("Empty request. Closed socket %d\n", i);
                    continue;
                }

                std::printf("Received request:\n%.*s\n",
                            int(request.size()), request.c_str());

                std::string response = handle_request_text(request);

                std::printf("Response to client\n%.*s",
                            int(response.size()), response.c_str());

                if (::send(i, response.c_str(), response.size(), 0) == -1) {
                    perror("send");
                }
                ::close(i);
                FD_CLR(i, &master);
                std::printf("Finished processing client on socket %d\n", i);
            }
        }
    }

    for (int fd = 0; fd <= fdmax; ++fd) {
        if (FD_ISSET(fd, &master)) ::close(fd);
    }
    ::unlink(SOCKET_PATH);
    return 0;
}
