// step4_multiuser_server - main entry point
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>

#include "../common/include/Graph.hpp"
#include "../common/include/Point.hpp"
#include "../common/include/CH.hpp"

using namespace std;
using namespace common;

#define PORT 9034
#define BUFSIZE 1024

Graph graph;
int expected_points = 0;
bool awaiting_points = false;

// משתנים גלובליים לניהול מצב פר לקוח
std::map<int, int> expected_points_map;
std::map<int, bool> awaiting_points_map;

// משתנים לוגיים לשליטה בגישה לגרף
bool graph_busy = false;
int current_owner_fd = -1;

string handle_command(int fd, const string &input)
{
    // חסימת לקוחות אחרים בזמן שהגרף בתפוסה
    if (graph_busy && fd != current_owner_fd) {
        return "Another user is currently modifying or computing the graph. Please wait.\n";
    }

    // חילוץ פקודה ראשית מהקלט
    stringstream ss(input);
    string command;
    ss >> command;

    // אם המשתמש שולח Newgraph -> תמיד לאתחל מחדש, גם באמצע הכנסת נקודות
    if (command == "Newgraph") {
        int n;
        if (!(ss >> n))
            return "Error: Usage: Newgraph <n>\n";

        // איפוס מצב קודם
        graph.clear();
        expected_points_map[fd] = n;
        awaiting_points_map[fd] = true;
        graph_busy = true;
        current_owner_fd = fd;

        return "OK: Send " + to_string(n) + " point(s) in format x,y or x y\n";
    }

    // אם המשתמש שולח CH -> לחשב גם אם באמצע הכנסת נקודות
    if (command == "CH") {
        auto hull = compute_convex_hull(graph.get_points());

        // לאתחל מצב (כדי שהכנסה תסתיים)
        awaiting_points_map[fd] = false;
        expected_points_map.erase(fd);
        graph_busy = false;
        current_owner_fd = -1;

        stringstream out;
        out << "Convex Hull:\n";
        for (const auto &p : hull)
            out << p.x << "," << p.y << "\n";
        out << "Area of convex hull: " << compute_area(hull) << "\n";
        return out.str();
    }

    // אם מחכים לנקודות (ולא פקודת Newgraph/CH)
    if (awaiting_points_map[fd]) {
        string coords = input;
        if (input.find("Newpoint") == 0) {
            size_t pos = input.find(' ');
            if (pos == string::npos) {
                return "Error: Expected point format after Newpoint\n";
            }
            coords = input.substr(pos + 1);
        }

        double x, y;
        char comma;
        bool parsed = false;
        {
            stringstream ss(coords);
            if ((ss >> x >> comma >> y) && comma == ',') {
                parsed = true;
            } else {
                ss.clear();
                ss.str(coords);
                if (ss >> x >> y) {
                    parsed = true;
                }
            }
        }

        if (!parsed) {
            return "Error: Expected point format: x,y or x y\n";
        }

        graph.add_point(Point(x, y));
        expected_points_map[fd]--;

        if (expected_points_map[fd] <= 0) {
            awaiting_points_map[fd] = false;
            expected_points_map.erase(fd);
            graph_busy = false;
            current_owner_fd = -1;
            return "Graph initialized with all points.\n";
        }

        return "OK: Point added. Waiting for " + to_string(expected_points_map[fd]) + " more...\n";
    }

    // שאר הפקודות כרגיל
    if (command == "Newpoint") {
        // ...
    }
    else if (command == "Removepoint") {
        // ...
    }
    else {
        return "Error: Unknown command\n";
    }
}

int main()
{
    int listener, newfd, fdmax;
    struct sockaddr_in serv_addr;
    fd_set master, read_fds;
    char buf[BUFSIZE];

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        exit(1);
    }

    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    memset(&serv_addr, 0, sizeof serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr *)&serv_addr, sizeof serv_addr) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(listener, 10) < 0) {
        perror("listen");
        exit(1);
    }

    FD_ZERO(&master);
    FD_SET(listener, &master);
    fdmax = listener;

    cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        for (int i = 0; i <= fdmax; ++i) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == listener) {
                    socklen_t addrlen = sizeof serv_addr;
                    newfd = accept(listener, (struct sockaddr *)&serv_addr, &addrlen);
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master);
                        if (newfd > fdmax)
                            fdmax = newfd;
                        send(newfd, "Welcome to CH server. Send commands.\n", 37, 0);
                    }
                } else {
                    memset(buf, 0, BUFSIZE);
                    int nbytes = recv(i, buf, BUFSIZE - 1, 0);
                   if (nbytes <= 0) {
    // אם הלקוח היה הבעלים של הנעילה -> שחרר אותה
    if (i == current_owner_fd) {
        graph_busy = false;
        current_owner_fd = -1;
        awaiting_points_map[i] = false;
        expected_points_map.erase(i);
    }

    close(i);
    FD_CLR(i, &master);
}
 else {
                        string input(buf);
                        input.erase(remove(input.begin(), input.end(), '\r'), input.end());
                        input.erase(remove(input.begin(), input.end(), '\n'), input.end());

                        string response = handle_command(i, input);
                        send(i, response.c_str(), response.size(), 0);
                    }
                }
            }
        }
    }
    return 0;
}
