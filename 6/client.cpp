/*
** client_uds_select.cpp -- UDS stream client with select()
** שולח: מטריצת שכנויות של הגרף (טקסט)
** מקבל: תשובת השרת (EULERIAN: YES/NO + המעגל אם קיים), ומדפיס במלואה
*/

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <errno.h>
#include <utility> // For std::pair


#include "Graph.hpp"
#define STDIN_FD 0
static const char* SOCKET_PATH = "mysocket";
static const size_t MAXDATASIZE = 1024; // גודל קריאה בכל recv


//Construct a random graph with V vertices and E edges
Graph generateRandomGraph(int V, int E, int seed, bool directed = false) {

    srand(seed);

    Graph g(V, directed);
   
    int edgesAdded = 0;
    while (edgesAdded < E) {
        int u = rand() % V; // 0 to V-1
        int v = rand() % V;
        int w = rand() % 10 + 1; // Random weight between 1 and 10
        
        if (u != v && !g.isEdgeConnected(u, v)) { // Avoid self-loops and duplicate edges
            g.addEdge(u, v, w);
            edgesAdded++;
        }

    }
    return g;
}

// Convert graph to adjacency matrix text representation
static std::string toAdjacencyMatrixText(const Graph& g) {
    const int V = g.getNumVertices();
    std::vector<std::vector<int>> mat(V, std::vector<int>(V, -1));
    //Matrix, edge from row to col
    for (int u = 0; u < V; ++u) { //Row
        for (std::pair<int,int> v : g.getNeighbors(u)) { //Col
            if (v.first >= 0 && v.first < V) {
                
                mat[u][v.first] = v.second; //Mark the edge in the matrix (u -> v), weight = v.second

                //For undirected graph, mark the reverse edge too (v -> u), weight = v.second
                if(!g.isDirected()){
                    mat[v.first][u] = v.second;
                } 
            }
        }
    }

    //Convert matrix to string
    std::ostringstream out;
    out<< V << '\n'; //First line is number of vertices
    out << (g.isDirected() ? "directed" : "undirected") << '\n'; //Second line is directed or not
    for (int i = 0; i < V; ++i) {
        for (int j = 0; j < V; ++j) {
            out << mat[i][j];
            if (j + 1 < V) out << ' ';
        }
        out << '\n';
    }
    return out.str();
}

// ---------- שליחה מלאה על גבי סוקט ----------
static bool send_all(int fd, const char* buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = ::send(fd, buf + sent, len - sent, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("send");
            return false;
        }
        sent += static_cast<size_t>(n);
    }
    return true;
}

int main(int argc, char* argv[]) {
 
    
    int V;
    int E;
    int seed;
    int opt;
    bool directed = false; // Default to not directed graph

    while ((opt = getopt(argc, argv, "v:e:s:d")) != -1) {
        switch (opt) {
            case 'v':
                V = std::stoi(optarg);
                break;
            case 'e':
                E = std::stoi(optarg);
                break;
            case 's':
                seed = std::stoi(optarg);
                break;
            case 'd':
                directed = true;
                break;
            case '?':
                if (optopt == 'v' || optopt == 'e' || optopt == 's') {
                    std::cerr << "Option -" << static_cast<char>(optopt) << " requires an argument." << std::endl;
                } else {
                    std::cerr << "Unknown option `-" << static_cast<char>(optopt) << "`." << std::endl;
                }
                return 1;
            default:
                abort(); // Stop the program on unexpected errors
        }
    }

    if (V <= 0) {
        std::cerr << "Error: Number of vertices must be positive." << std::endl;
        return 1;
    }
    if (E < 0) {
        std::cerr << "Error: Number of edges cannot be negative." << std::endl;
        return 1;
    }
    if (directed) {
        if (E > V * (V - 1)) {
            std::cerr << "Error: Too many edges for the number of vertices in a directed graph." << std::endl;
            return 1;
        }
    } else {
        if (E > V * (V - 1) / 2) {
            std::cerr << "Error: Too many edges for the number of vertices in an undirected graph." << std::endl;
            return 1;
        }
    }

    // Generate the random graph
    Graph g = generateRandomGraph(V, E, seed, directed);

    // Convert to adjacency matrix text
    std::string matrixText = toAdjacencyMatrixText(g);

    // חיבור ל-UDS
    int sockfd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return 1; }

    sockaddr_un remote{};
    remote.sun_family = AF_UNIX;
    std::strncpy(remote.sun_path, SOCKET_PATH, sizeof(remote.sun_path) - 1);

    if (::connect(sockfd, reinterpret_cast<sockaddr*>(&remote), sizeof(remote)) < 0) {
        perror("connect");
        ::close(sockfd);
        return 1;
    }

    // מדפיסים מה נשלח
    std::cout << "=== Sending adjacency matrix to server ===\n";
             // << matrixText << std::flush;

    // שולחים את המטריצה
    if (!send_all(sockfd, matrixText.c_str(), matrixText.size())) {
        ::close(sockfd);
        return 1;
    }
    // מסמנים שסיימנו לשלוח, כדי שהשרת יוכל לקרוא עד EOF
    ::shutdown(sockfd, SHUT_WR);

    // לולאת select: נאסוף את כל תשובת השרת עד שהוא סוגר
    std::string full_response;
    for (;;) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        struct timeval tv;
        tv.tv_sec = 5;      // טיים-אאוט סביר לקריאה
        tv.tv_usec = 0;

        int rc = ::select(sockfd + 1, &readfds, nullptr, nullptr, &tv);
        if (rc < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        if (rc == 0) {
            // פג טיים-אאוט — אפשר להמשיך להמתין, או לצאת.
            // נבחר להמשיך, כי השרת עשוי עדיין לעבד.
            continue;
        }

        if (FD_ISSET(sockfd, &readfds)) {
            char buf[MAXDATASIZE + 1];
            int n = ::recv(sockfd, buf, MAXDATASIZE, 0);
            if (n <= 0) {
                if (n == 0) {
                    // השרת סגר — כעת נדפיס את כל התשובה
                    std::cout << "=== Received response from server ===\n"
                              << full_response
                              << "[client] server closed connection\n";
                } else {
                    perror("recv");
                }
                break;
            }
            buf[n] = '\0';
            full_response.append(buf, n);
        }
    }

    ::close(sockfd);
    return 0;
}
