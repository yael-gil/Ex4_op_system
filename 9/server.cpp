/*
** server.cpp -- UDS Pipeline server: 4 Active Objects (MST -> SCC -> HAMILTON -> MAXCLIQUE)
** Prints what was received from client and what is sent back.
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <atomic>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#include "Graph.hpp"
#include "Algorithms.hpp"
#include "Factory.hpp"   // headers available in your project (not strictly required here)
#include "MST.hpp"
#include "SCC.hpp"
#include "Hamilton.hpp"
#include "MaxClique.hpp"

#define SOCKET_PATH "mysocket"
#define BACKLOG 64
static std::atomic<bool> should_exit{false};

// ======================= Utilities =======================

static bool send_all(int fd, const char* buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = ::send(fd, buf + sent, len - sent, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        sent += (size_t)n;
    }
    return true;
}

static std::string graphToString(const Graph& g) {
    std::ostringstream os;
    for (int i = 0; i < g.getNumVertices(); ++i) {
        os << "Vertex " << i << ":";
        for (auto nb : g.getNeighbors(i)) {
            os << " " << nb.first << "(w=" << nb.second << ")";
        }
        os << "\n";
    }
    return os.str();
}

// Parse a single-line request: -a <ALGO> -v <V> -e <E> -s <S>
static bool handle_request_text(const std::string& req, std::string& alg, int& V, int& E, int& S, std::string& err) {
    std::istringstream iss(req);
    std::vector<std::string> args; std::string tok;
    while (iss >> tok) args.push_back(tok);

    bool hasA=false, hasV=false, hasE=false, hasS=false;
    for (size_t i=0; i<args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "-a" && i+1 < args.size()) { alg = args[++i]; hasA = true; }
        else if (a == "-v" && i+1 < args.size()) {
            try { V = std::stoi(args[++i]); hasV = true; } catch(...) { err = "invalid -v"; return false; }
        } else if (a == "-e" && i+1 < args.size()) {
            try { E = std::stoi(args[++i]); hasE = true; } catch(...) { err = "invalid -e"; return false; }
        } else if (a == "-s" && i+1 < args.size()) {
            try { S = std::stoi(args[++i]); hasS = true; } catch(...) { err = "invalid -s"; return false; }
        } else {
            // ignore unknown tokens
        }
    }
    if (!hasA) { err = "missing -a <algorithm>"; return false; }
    if (!hasV || !hasE || !hasS) { err = "missing one of -v/-e/-s"; return false; }
    if (V <= 0) { err = "V must be positive"; return false; }
    if (E < 0)  { err = "E cannot be negative"; return false; }
    return true;
}

// ======================= Blocking Queue =======================

template <typename T>
class BlockingQueue {
public:
    void push(T x) {
        std::unique_lock<std::mutex> lk(m_);
        q_.push(std::move(x));
        cv_.notify_one();
    }
    T pop() {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [&]{ return !q_.empty() || should_exit.load(); });
        if (should_exit.load() && q_.empty()) {
            return T{}; // default object on shutdown
        }
        T x = std::move(q_.front());
        q_.pop();
        return x;
    }
    void shutdown() {
        std::unique_lock<std::mutex> lk(m_);
        cv_.notify_all();
    }
private:
    std::mutex m_;
    std::condition_variable cv_;
    std::queue<T> q_;
};

// ======================= Job & Queues =======================

struct Job {
    int id = 0;                       // Job ID (per-client request)
    int client_fd = -1;               // client's socket fd (for logging)

    int V=0, E=0, S=0;                // graph params
    std::shared_ptr<Graph> gUndir;    // undirected graph (MST/HAMILTON/MAXCLIQUE)
    std::shared_ptr<Graph> gDir;      // directed graph (SCC)

    std::string header;               // "=== Random Graphs (Pipeline) === ..." text
    std::string mst, scc, ham, mc;    // per-algorithm outputs

    // cv-based sync with the client handler thread:
    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;
    std::string reply;                // final aggregated reply (header + results)
};

using JobPtr = std::shared_ptr<Job>;

static BlockingQueue<JobPtr> Q_mst;
static BlockingQueue<JobPtr> Q_scc;
static BlockingQueue<JobPtr> Q_ham;
static BlockingQueue<JobPtr> Q_mc;

static std::atomic<int>  g_nextJobId{1};

// ======================= Pipeline Stages =======================

static void stage_mst() {
    MST alg; // uses Graph& -> std::string
    while (!should_exit.load()) {
        JobPtr job = Q_mst.pop();
        if (should_exit.load() || !job) break;

        std::printf("[MST] start job %d\n", job->id);
        try {
            job->mst = alg.run(*job->gUndir);
        } catch (const std::exception& ex) {
            job->mst = std::string("ERROR: ") + ex.what() + "\n";
        } catch (...) {
            job->mst = "ERROR: unknown exception\n";
        }
        std::printf("[MST] end   job %d\n", job->id);

        std::printf("[MST] push  job %d -> SCC\n", job->id);
        Q_scc.push(job);
    }
}

static void stage_scc() {
    SCC alg;
    while (!should_exit.load()) {
        JobPtr job = Q_scc.pop();
        if (should_exit.load() || !job) break;

        std::printf("[SCC] start job %d\n", job->id);
        try {
            job->scc = alg.run(*job->gDir);
        } catch (const std::exception& ex) {
            job->scc = std::string("ERROR: ") + ex.what() + "\n";
        } catch (...) {
            job->scc = "ERROR: unknown exception\n";
        }
        std::printf("[SCC] end   job %d\n", job->id);

        std::printf("[SCC] push  job %d -> HAMILTON\n", job->id);
        Q_ham.push(job);
    }
}

static void stage_ham() {
    Hamilton alg;
    while (!should_exit.load()) {
        JobPtr job = Q_ham.pop();
        if (should_exit.load() || !job) break;

        std::printf("[HAMILTON] start job %d\n", job->id);
        try {
            job->ham = alg.run(*job->gUndir);
        } catch (const std::exception& ex) {
            job->ham = std::string("ERROR: ") + ex.what() + "\n";
        } catch (...) {
            job->ham = "ERROR: unknown exception\n";
        }
        std::printf("[HAMILTON] end   job %d\n", job->id);

        std::printf("[HAMILTON] push  job %d -> MAXCLIQUE\n", job->id);
        Q_mc.push(job);
    }
}

static void stage_mc() {
    MaxClique alg;
    while (!should_exit.load()) {
        JobPtr job = Q_mc.pop();
        if (should_exit.load() || !job) break;

        std::printf("[MAXCLIQUE] start job %d\n", job->id);
        try {
            job->mc = alg.run(*job->gUndir);
        } catch (const std::exception& ex) {
            job->mc = std::string("ERROR: ") + ex.what() + "\n";
        } catch (...) {
            job->mc = "ERROR: unknown exception\n";
        }
        std::printf("[MAXCLIQUE] end   job %d\n", job->id);

        // Build final reply and notify client thread.
        {
            std::lock_guard<std::mutex> lk(job->mtx);
            std::ostringstream out;
            out << job->header
                << "=== Results ===\n"
                << "[MST]\n"       << job->mst
                << "[SCC]\n"       << job->scc
                << "[HAMILTON]\n"  << job->ham
                << "[MAXCLIQUE]\n" << job->mc
                << "=== DONE ===\n";
            job->reply = out.str();
            job->done = true;
        }
        std::printf("[MAXCLIQUE] notify client for job %d\n", job->id);
        job->cv.notify_one();
    }
}

// ======================= Client Handling =======================

static void handle_client(int client_fd) {
    // read full request (single-line text until EOF)
    std::string req;
    {
        char buf[4096];
        for (;;) {
            ssize_t n = ::recv(client_fd, buf, sizeof(buf), 0);
            if (n == 0) break;                       // EOF
            if (n < 0) { if (errno == EINTR) continue; break; }
            req.append(buf, buf + n);
        }
    }

    // Trim trailing newlines/spaces for prettier logging (optional)
    while (!req.empty() && (req.back()=='\n' || req.back()=='\r' || std::isspace((unsigned char)req.back())))
        req.pop_back();

    std::printf("<<< [fd %d -> server] received %zu bytes:\n%s\n", client_fd, req.size(), req.c_str());

    // parse
    std::string alg; int V=0, E=0, S=0; std::string err;
    if (!handle_request_text(req, alg, V, E, S, err)) {
        std::ostringstream os;
        os << "ERROR: " << err << "\n"
           << "Usage: -a <ALGO> -v <V> -e <E> -s <S>\n";
        std::string msg = os.str();
        (void)send_all(client_fd, msg.c_str(), msg.size());
        ::close(client_fd);
        return;
    }

    // Create job
    JobPtr job = std::make_shared<Job>();
    job->id = g_nextJobId.fetch_add(1, std::memory_order_relaxed);
    job->client_fd = client_fd;
    job->V = V; job->E = E; job->S = S;

    std::printf("[JOB %d] created for client fd %d (V=%d,E=%d,S=%d)\n", job->id, client_fd, V, E, S);

    // Generate twin graphs
    try {
        job->gUndir = std::make_shared<Graph>( Graph::generateRandomGraph(V, E, S, /*directed=*/false) );
        job->gDir   = std::make_shared<Graph>( Graph::generateRandomGraph(V, E, S, /*directed=*/true) );
    } catch (const std::exception& ex) {
        std::string msg = std::string("ERROR: generateRandomGraph failed: ") + ex.what() + "\n";
        (void)send_all(client_fd, msg.c_str(), msg.size());
        ::close(client_fd);
        return;
    }

    // Build header (graphs overview printed back to client)
    {
        std::ostringstream head;
        head << "=== Random Graphs (Pipeline) ===\n";
        head << "--- Undirected ---\n" << graphToString(*job->gUndir);
        head << "--- Directed ---\n"   << graphToString(*job->gDir);
        job->header = head.str();
    }

    std::printf("[JOB %d] enqueue to MST stage\n", job->id);
    Q_mst.push(job);

    // Wait until the last stage completes and sets reply
    std::string reply;
    {
        std::unique_lock<std::mutex> lk(job->mtx);
        job->cv.wait(lk, [&]{ return job->done; });
        reply = job->reply;
    }

    std::printf("[JOB %d] reply ready (%zu bytes), sending to fd %d\n",
                job->id, reply.size(), client_fd);

    (void)send_all(client_fd, reply.c_str(), reply.size());
    ::shutdown(client_fd, SHUT_RDWR);
    ::close(client_fd);

    std::printf("[JOB %d] client fd %d closed\n", job->id, client_fd);
}

// ======================= Main (UDS server) =======================

int main() {
    // line-buffered stdout (flush on '\n')
    setvbuf(stdout, nullptr, _IOLBF, 0);

    // Remove stale socket file and create listener
    ::unlink(SOCKET_PATH);

    int listener = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (listener < 0) { perror("socket"); return 1; }

    struct sockaddr_un local{};
    local.sun_family = AF_UNIX;
    std::strncpy(local.sun_path, SOCKET_PATH, sizeof(local.sun_path)-1);

    if (::bind(listener, reinterpret_cast<struct sockaddr*>(&local), sizeof(local)) < 0) {
        perror("bind");
        ::close(listener);
        return 1;
    }
    if (::listen(listener, BACKLOG) < 0) {
        perror("listen");
        ::close(listener);
        return 1;
    }

    std::printf("Pipeline-only server (UDS) on %s\n", SOCKET_PATH);

    // Start the 4 Active Objects (pipeline stages)
    std::thread t_mst(stage_mst);
    std::thread t_scc(stage_scc);
    std::thread t_ham(stage_ham);
    std::thread t_mc(stage_mc);

    // Accept loop: short-lived thread per client
    for (;;) {
        struct sockaddr_un remote{};
        socklen_t addrlen = sizeof(remote);
        int client_fd = ::accept(listener, reinterpret_cast<struct sockaddr*>(&remote), &addrlen);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            break;
        }
        std::thread(handle_client, client_fd).detach();
    }

    // Shutdown
    should_exit.store(true);
    Q_mst.shutdown(); Q_scc.shutdown(); Q_ham.shutdown(); Q_mc.shutdown();

    ::close(listener);
    ::unlink(SOCKET_PATH);

    t_mst.join(); t_scc.join(); t_ham.join(); t_mc.join();
    return 0;
}
