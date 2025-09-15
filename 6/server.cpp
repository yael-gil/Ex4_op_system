/*
** server_uds.cpp -- UDS server with select()
** קלט: מטריצת שכנויות טקסטואלית (שורות, רווחים, \n)
** פלט: "EULERIAN: YES\n" + פלט findEulerCircuit(os) או "EULERIAN: NO\n"
** בנוסף: השרת מדפיס למסך את המטריצה שקיבל ואת התשובה שהוא שולח
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

#include "Graph.hpp"

static const char* SOCKET_PATH = "mysocket"; /*
*/

int main() {

// fd_set:   (File Descriptors) מבנה ייחודי שמחזיק "קבוצת תיאורי קבצים ".
// כל ביט במבנה מייצג האם מתבצע מעקב אחרי תיאור קובץ מסוים לקריאה או לכתיבה.
//  select() משמש את הפונקציה  כדי לדעת איזה מהסוקטים מוכן לפעולה.
fd_set master, read_fds;

// master → הקבוצה הקבועה שאנחנו עוקבים אחריה כל הזמן 
// read_fds →  select() עותק זמני שנוצר בכל קריאה ל- 
//, משום שהפונקציה משנה את התוכן של הקבוצה.
int fdmax, listener, newfd;

// fdmax → שומר את המספר הגבוה ביותר של תיאור קובץ (FD) שנמצא במעקב.
// listener → סוקט ההאזנה הראשי של השרת, דרכו מגיעים חיבורים חדשים.
// newfd → סוקט חדש שנוצר עבור כל חיבור נכנס לאחר הקריאה ל-accept().
struct sockaddr_un local{}, remote{};

// sockaddr_un -  UDS (Unix Domain Sockets) מבנה כתובת ייחודי לפרוטוקול .
// מבנה זה מכיל:
//    • sun_family  סוג הכתובת (AF_UNIX).
//    • sun_path  הנתיב בקובץ הסוקט (לדוגמה: "mysocket").
socklen_t addrlen;  
// accept() משתנה זה משמש לשמירת אורך מבנה הכתובת שמועבר לקריאה ל


  // FD_ZERO(&master):
// מאפס (מנקה) את קבוצת הסוקטים הראשית (master).
// פעולה זו מסמנת שכל תיאורי הקבצים (FDs) בקבוצה אינם במעקב כרגע.
FD_ZERO(&master);

// FD_ZERO(&read_fds):
// מאפס (מנקה) גם את קבוצת הסוקטים הזמנית (read_fds),
// שמשמשת כל קריאה ל-select() כדי לבדוק מי מהסוקטים מוכן.
FD_ZERO(&read_fds);

// מוחק את קובץ הסוקט אם הוא כבר קיים במערכת.
// ב-UDS חובה לנקות את הקובץ הישן לפני קריאת bind(), אחרת הקריאה תיכשל עם השגיאה EADDRINUSE.

::unlink(SOCKET_PATH);

    listener = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (listener < 0) { perror("socket"); exit(1); }

    local.sun_family = AF_UNIX;
    std::strncpy(local.sun_path, SOCKET_PATH, sizeof(local.sun_path) - 1);

    if (::bind(listener, reinterpret_cast<struct sockaddr*>(&local), sizeof(local)) < 0) {
        perror("bind"); ::close(listener); exit(1);
    }
    if (::listen(listener, 10) == -1) {
        perror("listen"); ::close(listener); exit(1);
    }

    FD_SET(listener, &master);
    fdmax = listener;

    std::printf("Server is listening on UNIX socket: %s\n", SOCKET_PATH);

    for (;;) {
        read_fds = master;
        if (::select(fdmax + 1, &read_fds, nullptr, nullptr, nullptr) == -1) {
            if (errno == EINTR) continue;
            perror("select"); break;
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
                // 1) קוראים את כל הבקשה (עד EOF)
                std::string request;
                char rbuf[4096];
                for (;;) {
                    ssize_t n = ::recv(i, rbuf, sizeof(rbuf), 0);
                    if (n == 0) break;                  // EOF
                    if (n < 0) {
                        if (errno == EINTR) continue;
                        perror("recv"); break;
                    }
                    request.append(rbuf, rbuf + n);
                }
                if (request.empty()) {
                    ::close(i); FD_CLR(i, &master);
                    std::printf("Empty request. Closed socket %d\n", i);
                    continue;
                }

                // 2) מדפיסים את המטריצה שקיבלנו
                std::printf("=== Received adjacency matrix (text) ===\n%.*s\n",
                            int(request.size()), request.c_str());

                // 3) פרסינג טקסט -> מטריצה
                std::vector<std::vector<int>> adj;
                {
                    std::istringstream iss(request);
                    std::string line;
                    while (std::getline(iss, line)) {
                        if (line.empty()) continue;
                        std::istringstream lss(line);
                        std::vector<int> row; int v;
                        while (lss >> v) row.push_back(v);
                        if (!row.empty()) adj.push_back(row);
                    }
                }

                bool square = !adj.empty();
                for (const auto& row : adj) {
                    if (row.size() != adj.size()) { square = false; break; }
                }

                std::string response;
                if (!square) {
                    response = "EULERIAN: NO\nInvalid adjacency matrix.\n";
                } else {
                    // 4) בונים גרף לא-מכוון מתוך המטריצה
                    const int n = (int)adj.size();
                    Graph graph(n);
                    for (int r = 0; r < n; ++r) {
                        for (int c = r + 1; c < n; ++c) {
                            if (adj[r][c] > 0 && !graph.isEdgeConnected(r, c)) {
                                graph.addEdge(r, c);
                            }
                        }
                    }

                    // 5) מדפיסים את המסלול ל-ostringstream ושולחים
                    std::ostringstream oss;
                    graph.findEulerCircuit(oss); // אם לא אוילרי — לא ידפיס כלום
                    std::string printed = oss.str();

                    if (!printed.empty()) {
                        response = "EULERIAN: YES\n" + printed;
                    } else {
                        response = "EULERIAN: NO\n";
                    }
                }

                // 6) מדפיסים את התשובה ושולחים ללקוח
                std::printf("=== Response to client ===\n%.*s",
                            int(response.size()), response.c_str());

                if (::send(i, response.c_str(), response.size(), 0) == -1) {
                    perror("send");
                }

                ::close(i); FD_CLR(i, &master);
                std::printf("Finished processing client on socket %d\n", i);
            }
        }
    }

    for (int fd = 0; fd <= fdmax; ++fd) if (FD_ISSET(fd, &master)) ::close(fd);
    ::unlink(SOCKET_PATH);
    return 0;
}
