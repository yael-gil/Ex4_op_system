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
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include "Graph.hpp"
#include "Algorithms.hpp"
#include "Factory.hpp"   // AlgorithmFactory::createAlgorithm
#define SOCKET_PATH "mysocket"
#define BACKLOG 64
#define NUM_THREADS 8

// ------------ Leader-Follower ------------
static std::mutex              lf_mtx;
static std::condition_variable lf_cv;
static bool                    leader_token = true;

// ------------ net utils ------------
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
static void send_line_threadsafe(int fd, const std::string& s, std::mutex& mtx) {
    std::lock_guard<std::mutex> lk(mtx);
    (void)send_all(fd, s.c_str(), s.size());
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

// ------------ parse: -a <ALGO> -v <V> -e <E> -s <S> ------------
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
        }
    }
    if (!hasA) { err = "missing -a <algorithm>"; return false; }
    if (!hasV || !hasE || !hasS) { err = "missing one of -v/-e/-s"; return false; }
    if (V <= 0) { err = "V must be positive"; return false; }
    if (E < 0)  { err = "E cannot be negative"; return false; }
    return true;
}
// run all 4 algos in LF (each in its own thread)
static void run_all_algorithms_LF(int client_fd, int V, int E, int S) {
    // Runs the four algorithms with one thread per algorithm (LF = send results to the client as soon as each finishes).
    // client_fd  - socket descriptor for the UDS connection to the client
    // V, E, S    - parameters for random graph generation: number of vertices (V), number of edges (E), random seed (S)

    // Twin graphs from the same seed:
    Graph gUndir = Graph::generateRandomGraph(V, E, S, /*directed=*/false); // undirected graph (for MST/HAMILTON/MAXCLIQUE)
    Graph gDir   = Graph::generateRandomGraph(V, E, S, /*directed=*/true);  // directed graph (for SCC)

    std::mutex send_mtx; // mutex guarding writes to the socket to prevent interleaved output from different threads.

    // Send the generated graphs to the client
    {
        std::ostringstream head; // temporary string builder for batching the header output
        head << "=== Random Graphs (seed=" << S << ", V=" << V << ", E=" << E << ") ===\n";
        head << "--- Undirected ---\n" << graphToString(gUndir); // stringify the undirected graph
        head << "--- Directed ---\n"   << graphToString(gDir);   // stringify the directed graph
        head << "=== Running 4 algorithms (LF) ===\n";           // indicate that LF execution/streaming starts now
        send_line_threadsafe(client_fd, head.str(), send_mtx);   // thread-safe send to the client
    }

    // A lambda that runs a single algorithm on a given graph and streams its result immediately upon completion
    auto run_and_stream = [&](const char* tag, const std::string& algoName, const Graph& src){
        Graph local = src; // make a local copy of the graph if the algorithm mutates internally, it won't affect others
        auto algo = AlgorithmFactory::createAlgorithm(algoName); // dynamically create the algorithm by name
        if (!algo) {
            // unknown algorithm name report an error to the client
            send_line_threadsafe(client_fd, std::string("[") + tag + "] ERROR: unknown algorithm '" + algoName + "'\n", send_mtx);
            return; // stop this thread
        }
        try {
            std::string out = algo->run(local); // run the algorithm on the local copy and get textual output
            if (!out.empty() && out.back() != '\n') out.push_back('\n'); // ensure it ends with a newline
            // prepend a tag header so the client knows which algorithm produced this output
            send_line_threadsafe(client_fd, std::string("[") + tag + "]\n" + out, send_mtx);
        } catch (const std::exception& ex) {
            // handle standard exceptions send error message to the client
            send_line_threadsafe(client_fd, std::string("[") + tag + "] ERROR: exception: " + ex.what() + "\n", send_mtx);
        } catch (...) {
            // handle any other exception
            send_line_threadsafe(client_fd, std::string("[") + tag + "] ERROR: unknown exception\n", send_mtx);
        }
    };

    // Four threads whoever finishes first sends first
    // Each std::thread receives the lambda + its parameters. std::cref passes a const reference to src (which is copied at lambda start).
    std::thread t_scc( run_and_stream, "SCC",       "SCC",       std::cref(gDir)   ); // SCC runs on the directed graph
    std::thread t_mst( run_and_stream, "MST",       "MST",       std::cref(gUndir) ); // MST runs on the undirected graph
    std::thread t_ham( run_and_stream, "HAMILTON",  "HAMILTON",  std::cref(gUndir) ); // HAMILTON on the undirected graph
    std::thread t_mc ( run_and_stream, "MAXCLIQUE", "MAXCLIQUE", std::cref(gUndir) ); // MAXCLIQUE on the undirected graph

    t_scc.join(); t_mst.join(); t_ham.join(); t_mc.join(); // wait for all four threads before signaling DONE
    send_line_threadsafe(client_fd, "=== DONE ===\n", send_mtx); // final line sent to the client
}

// Leader-Follower
static void worker_thread(int listener_fd) {
    // Thread function for the worker pool using the Leader–Follower pattern.
    // Each thread: waits to become the "Leader", performs a single accept(), promotes a new Follower to Leader,
    // and then becomes the Worker that handles this connected client (read request, run LF, send results).

    for (;;) { // infinite loop each thread handles clients one after another
        //1 Acquire leader
        std::unique_lock<std::mutex> lk(lf_mtx); // lock the LF mutex to check/wait for the token
        while (!leader_token) lf_cv.wait(lk);     // if no token wait (this thread is a Follower now)
        leader_token = false; // this thread becomes the Leader takes the token to perform an exclusive accept()
        lk.unlock();          // release the lock before accept to avoid blocking other threads' progress

        // 2 accept
        struct sockaddr_un remote{};                         // remote address (UDS)
        socklen_t addrlen = sizeof(remote);                  // size of the address structure
        int client_fd = ::accept(listener_fd, reinterpret_cast<struct sockaddr*>(&remote), &addrlen);
        if (client_fd < 0) {
            // if accept failed, return the token so another thread can become Leader
            std::lock_guard<std::mutex> lk2(lf_mtx);
            leader_token = true;
            lf_cv.notify_one(); // wake one waiting thread to take the Leader role
            continue;           // retry
        }

        // 3 promote next leader
        {
            // hand off the Leader token to another waiting thread this thread now turns into the Worker
            std::lock_guard<std::mutex> lk2(lf_mtx);
            leader_token = true;  // release the token
            lf_cv.notify_one();   // wake one waiting thread to become the next Leader
        }

        // 4 handle client: read full request line, parse, run LF
        std::string req;                        // accumulates the entire request from the client
        char buf[4096];                         // socket read buffer

        for (;;) {                              // read until EOF (as long as the client sends)
            ssize_t n = ::recv(client_fd, buf, sizeof(buf), 0);
            if (n == 0) break;                  // EOF client finished sending
            if (n < 0) { if (errno == EINTR) continue; break; } // if interrupted, retry; otherwise it's an error
            req.append(buf, buf + n);           // append the newly read bytes to the request string
        }

        std::string alg; int V=0, E=0, S=0; std::string err; // parameters to extract from the CLI 
        if (!handle_request_text(req, alg, V, E, S, err)) {
            // parsing failed send usage error to the client
            std::ostringstream os;
            os << "ERROR: " << err << "\n"
               << "Usage: -a <ALGO> -v <V> -e <E> -s <S>\n";
            send_all(client_fd, os.str().c_str(), os.str().size());
            ::close(client_fd);
            continue; // go handle the next client
        }

        // Run the four algorithms and stream results as they finish
        run_all_algorithms_LF(client_fd, V, E, S); // sends outputs to the client live, in completion order

        ::shutdown(client_fd, SHUT_RDWR); // gracefully shut down both directions (polite; optional in many cases)
        ::close(client_fd);               // close the client's socket
    }
}

int main() {
    // main function sets up a UDS listener, builds a Leader–Follower thread pool, and waits forever.

    ::unlink(SOCKET_PATH); // remove any stale socket file from previous runs (so bind succeeds)

    int listener = ::socket(AF_UNIX, SOCK_STREAM, 0); // create a UNIX Domain stream socket 
    if (listener < 0) { perror("socket"); return 1; }

    struct sockaddr_un local{};                      // local address structure for UDS (filesystem path)
    local.sun_family = AF_UNIX;                      // address family: UNIX
    std::strncpy(local.sun_path, SOCKET_PATH, sizeof(local.sun_path)-1); // copy the path (socket filename)

    if (::bind(listener, reinterpret_cast<struct sockaddr*>(&local), sizeof(local)) < 0) {
        // bind the socket to the filesystem path (creates the socket file)
        perror("bind");
        ::close(listener);
        return 1;
    }
    if (::listen(listener, BACKLOG) < 0) {
        // start listening for incoming connections; BACKLOG controls the pending queue depth
        perror("listen");
        ::close(listener);
        return 1;
    }

    std::printf("LF server on UDS: %s (threads=%d)\n", SOCKET_PATH, NUM_THREADS);

    std::vector<std::thread> pool; // thread pool container
    pool.reserve(NUM_THREADS);     // pre allocate to avoid relocations
    for (int i=0; i<NUM_THREADS; ++i) {
        pool.emplace_back(worker_thread, listener); // spawn a thread running worker_thread(listener)
    }
    for (auto& t : pool) t.join(); // main waits for all threads (in practice, this never returns)

    ::close(listener);   // cleanup will only run if the loop ever ends
    ::unlink(SOCKET_PATH); // remove the socket file from the filesystem
    return 0;
}
