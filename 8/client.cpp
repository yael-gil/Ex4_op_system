

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
#include <fstream>

static const char* SOCKET_PATH = "mysocket";
static const size_t MAXDATASIZE = 1024;

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

void print_usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [OPTIONS]\n"
              << "\nOptions:\n"
              << "  -a <algorithm>  Algorithm to run:\n"
              << "                  EULERIAN    - Find Eulerian circuit\n"
              << "                  SCC         - Strongly Connected Components\n"
              << "                  MST         - Minimum Spanning Tree\n"
              << "                  MAXCLIQUE   - Maximum Clique\n"
              << "                  HAMILTON    - Hamilton Circuit\n"
              << "  -v <vertices>   Number of vertices (for random graph)\n"
              << "  -e <edges>      Number of edges (for random graph)\n"
              << "  -s <seed>       Random seed\n"
              << "  -m <file>       Read adjacency matrix from file\n"
              << "  -i              Read adjacency matrix from stdin\n"
              << "  -h              Show this help\n"
              << "\nExamples:\n"
              << "  " << prog_name << " -a EULERIAN -v 5 -e 6 -s 42\n"
              << "  " << prog_name << " -a SCC -v 4 -e 5 -s 123\n"
              << "  " << prog_name << " -a MST -v 6 -e 8 -s 456\n"
              << "  " << prog_name << " -a MAXCLIQUE -v 5 -e 7 -s 789\n"
              << "  " << prog_name << " -a HAMILTON -v 4 -e 6 -s 101\n"
              << "  echo '0 1 1\\n1 0 1\\n1 1 0' | " << prog_name << " -a EULERIAN -i\n"
              << "  " << prog_name << " -a MST -m matrix.txt\n";
}



int send_request_and_receive_response(const std::string& request) {
    // חיבור ל-UDS
    int sockfd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }
    
    sockaddr_un remote{};
    remote.sun_family = AF_UNIX;
    std::strncpy(remote.sun_path, SOCKET_PATH, sizeof(remote.sun_path) - 1);
    
    if (::connect(sockfd, reinterpret_cast<sockaddr*>(&remote), sizeof(remote)) < 0) {
        perror("connect");
        ::close(sockfd);
        return 1;
    }
    
    // מדפיסים מה נשלח
    std::cout << "=== Sending request to server ===\n" << request << std::flush;
    
    // שולחים את הבקשה
    if (!send_all(sockfd, request.c_str(), request.size())) {
        ::close(sockfd);
        return 1;
    }
    
    // מסמנים שסיימנו לשלוח
    ::shutdown(sockfd, SHUT_WR);
    
    // קבלת תשובה מהשרת
    std::string full_response;
    for (;;) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        
        struct timeval tv;
        tv.tv_sec = 10; // טייםאאוט של 10 שניות
        tv.tv_usec = 0;
        
        int rc = ::select(sockfd + 1, &readfds, nullptr, nullptr, &tv);
        if (rc < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        if (rc == 0) {
            // פג טייםאאוט
            std::cerr << "Timeout waiting for server response\n";
            break;
        }
        
        if (FD_ISSET(sockfd, &readfds)) {
            char buf[MAXDATASIZE + 1];
            int n = ::recv(sockfd, buf, MAXDATASIZE, 0);
            if (n <= 0) {
                if (n == 0) {
                    // השרת סגר  הדפסת התשובה המלאה
                    std::cout << "=== Received response from server ===\n" 
                              << full_response 
                              << "[client] Connection closed by server\n";
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

int main(int argc, char* argv[]) {
    std::string algorithm;
    int V = 0, E = 0, seed = 0;
    std::string matrix_file;
    bool use_file = false;
    bool use_stdin = false;
    
    int opt;
    while ((opt = ::getopt(argc, argv, "a:v:e:s:m:ih")) != -1) {
        switch (opt) {
            case 'a':
                algorithm = optarg;
                break;
            case 'v':
                V = std::stoi(optarg);
                break;
            case 'e':
                E = std::stoi(optarg);
                break;
            case 's':
                seed = std::stoi(optarg);
                break;
            case 'm':
                matrix_file = optarg;
                use_file = true;
                break;
            case 'i':
                use_stdin = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case '?':
                print_usage(argv[0]);
                return 1;
            default:
                return 1;
        }
    }
    
    if (algorithm.empty()) {
        std::cerr << "Error: Algorithm must be specified with -a\n";
        print_usage(argv[0]);
        return 1;
    }
    
    // בדיקת תקינות האלגוריתם
    if (algorithm != "EULERIAN" && algorithm != "SCC" && algorithm != "MST" && 
        algorithm != "MAXCLIQUE" && algorithm != "HAMILTON") {
        std::cerr << "Error: Unknown algorithm '" << algorithm << "'\n";
        std::cerr << "Supported algorithms: EULERIAN, SCC, MST, MAXCLIQUE, HAMILTON\n";
        return 1;
    }
    
    std::string request;
    
     if (use_stdin || (V == 0 && E == 0)) {
        
        
        std::ostringstream req_stream;
        req_stream << "ALGORITHM:" << algorithm << "\n" ;
        request = req_stream.str();
    } else {
        // בדיקת פרמטרים לגרף אקראי
        if (V <= 0) {
            std::cerr << "Error: V must be positive when generating random graph.\n";
            return 1;
        }
        if (E < 0) {
            std::cerr << "Error: E cannot be negative.\n";
            return 1;
        }
        if (seed == 0) {
            seed = time(nullptr); // ברירת מחדל
        }
        
        // בניית בקשה עם פרמטרים
        std::ostringstream req_stream;
        req_stream << "-a " << algorithm
           << " -v " << V
           << " -e " << E
           << " -s " << seed << "\n";
request = req_stream.str();

    }
    
    return send_request_and_receive_response(request);
}