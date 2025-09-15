/*
** server_uds.cpp -- UDS server with select() and Algorithm Factory
** קלט: בקשת אלגוריתם + פרמטרים (או מטריצת שכנויות)
** פלט: תוצאת האלגוריתם המבוקש
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include "Factory.hpp"

static const char* SOCKET_PATH = "mysocket";

int main() {
    // File descriptor sets for select()
    fd_set master, read_fds;
    int fdmax, listener, newfd;
    struct sockaddr_un local{}, remote{};
    socklen_t addrlen;
    
    // Initialize file descriptor sets
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    
    // Remove any existing socket file
    ::unlink(SOCKET_PATH);
    
    // Create listener socket
    listener = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        exit(1);
    }
    
    // Setup socket address
    local.sun_family = AF_UNIX;
    std::strncpy(local.sun_path, SOCKET_PATH, sizeof(local.sun_path) - 1);
    
    // Bind socket to address
    if (::bind(listener, reinterpret_cast<struct sockaddr*>(&local), sizeof(local)) < 0) {
        perror("bind");
        ::close(listener);
        exit(1);
    }
    
    // Start listening
    if (::listen(listener, 10) == -1) {
        perror("listen");
        ::close(listener);
        exit(1);
    }
    
    // Add listener to master set
    FD_SET(listener, &master);
    fdmax = listener;
    
    std::printf("Algorithm Server is listening on UNIX socket: %s\n", SOCKET_PATH);
    
    // Main server loop
    for (;;) {
        read_fds = master; // Copy master set
        
        if (::select(fdmax + 1, &read_fds, nullptr, nullptr, nullptr) == -1) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        
        // Check all file descriptors
        for (int i = 0; i <= fdmax; ++i) {
            if (!FD_ISSET(i, &read_fds)) continue;
            
            if (i == listener) {
                // Handle new connection
                addrlen = sizeof(remote);
                newfd = ::accept(listener, reinterpret_cast<struct sockaddr*>(&remote), &addrlen);
                if (newfd == -1) {
                    perror("accept");
                    continue;
                }
                
                FD_SET(newfd, &master);
                if (newfd > fdmax) fdmax = newfd;
                
                std::printf("New client connected on socket %d\n", newfd);
            } else {
                // Handle data from existing client
                std::string request;
                char rbuf[4096];
                
                // Read complete request until EOF
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
                
                std::printf("=== Received request ===\n%.*s\n", 
                           int(request.size()), request.c_str());
                
                // Parse the request
                ClientRequest req = parseRequest(request);
                std::string response;
                
                if (!req.valid) {
                    response = "ERROR: " + req.error + "\n" + 
                } else {
                    // Create the requested algorithm
                    auto algorithm = createAlgorithm(req.algorithm);
                    if (!algorithm) {
                        response = "ERROR: Unknown algorithm '" + req.algorithm + "'\n" + 
                    } else {
                        std::printf("Running %s algorithm...\n", algorithm->getName().c_str());
                        
                        // Execute algorithm based on request type
                        if (req.hasMatrix) {
                            response = algorithm->execute(req.matrix);
                        } else {
                            response = algorithm->execute(req.vertices, req.edges, req.seed);
                        }
                    }
                }
                
                // Send response to client
                std::printf("=== Response to client ===\n%.*s", 
                           int(response.size()), response.c_str());
                
                if (::send(i, response.c_str(), response.size(), 0) == -1) {
                    perror("send");
                }
                
                // Close connection with this client
                ::close(i);
                FD_CLR(i, &master);
                std::printf("Finished processing client on socket %d\n", i);
            }
        }
    }
    
    // Cleanup
    for (int fd = 0; fd <= fdmax; ++fd) {
        if (FD_ISSET(fd, &master)) ::close(fd);
    }
    ::unlink(SOCKET_PATH);
    
    return 0;
}