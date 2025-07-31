#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <map>
#include <csignal>

#include "../common/include/Graph.hpp"
#include "../common/include/Point.hpp"
#include "../common/include/CH.hpp"
#include "../step5_reactor_template/reactor.hpp"

using namespace std;
using namespace common;

#define PORT 9034
#define BUFSIZE 1024

Graph graph;
Reactor<std::function<void(int)>> reactor;

// מצב פר לקוח
std::unordered_map<int, std::string> partial_inputs;
std::map<int, int> expected_points_map;
std::map<int, bool> awaiting_points_map;

// חסימה לוגית לכל פעולה
bool graph_busy = false;
int current_owner_fd = -1;

void release_lock(int fd) {
    if (fd == current_owner_fd) {
        graph_busy = false;
        current_owner_fd = -1;
    }
    awaiting_points_map.erase(fd);
    expected_points_map.erase(fd);
    partial_inputs.erase(fd);
}

std::string handle_command(int fd, const std::string& input) {
    // חסימת לקוחות אחרים אם הגרף תפוס
    if (graph_busy && fd != current_owner_fd) {
        return "Graph is busy, please wait...\n";
    }

    std::stringstream ss(input);
    std::string command;
    ss >> command;

    // --- PRIORITY: Newgraph ---
    if (command == "Newgraph") {
        int n;
        if (!(ss >> n)) return "Error: Usage: Newgraph <n>\n";

        // הפסק מצב קודם ואפס נעילה
        graph.clear();
        expected_points_map[fd] = n;
        awaiting_points_map[fd] = true;
        graph_busy = true;
        current_owner_fd = fd;

        return "OK: Send " + std::to_string(n) + " point(s) in format x,y or x y\n";
    }

    // --- PRIORITY: CH ---
    if (command == "CH") {
        // CH ניתן להרצה גם באמצע מצב awaiting_points
        auto hull = compute_convex_hull(graph.get_points());

        // שחרור נעילה אחרי החישוב
        release_lock(fd);

        std::stringstream out;
        out << "Convex Hull:\n";
        for (const auto& p : hull)
            out << p.x << "," << p.y << "\n";
        out << "Area of convex hull: " << compute_area(hull) << "\n";
        return out.str();
    }

    // אם הלקוח במצב הכנסת נקודות
    if (awaiting_points_map[fd]) {
        std::string coords = input;

        if (input.find("Newpoint") == 0) {
            size_t pos = input.find(' ');
            if (pos == std::string::npos)
                return "Error: Expected point format after Newpoint\n";
            coords = input.substr(pos + 1);
        }

        double x, y;
        char comma;
        bool parsed = false;

        {
            std::stringstream ss(coords);
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
            release_lock(fd);
            return "Graph initialized with all points.\n";
        }

        return "OK: Point added. Waiting for " + std::to_string(expected_points_map[fd]) + " more...\n";
    }

    // הפקודות הרגילות (לא במצב awaiting_points)
    if (command == "Newpoint") {
        double x, y;
        char comma;
        size_t pos = input.find(' ');
        if (pos == std::string::npos)
            return "Error: Usage: Newpoint <x>,<y> or Newpoint <x> <y>\n";
        std::string coords = input.substr(pos + 1);

        {
            std::stringstream ss(coords);
            if ((ss >> x >> comma >> y) && comma == ',') {
                graph.add_point(Point(x, y));
                return "OK: Point added\n";
            }
        }
        {
            std::stringstream ss(coords);
            if ((ss >> x >> y)) {
                graph.add_point(Point(x, y));
                return "OK: Point added\n";
            }
        }

        return "Error: Usage: Newpoint <x>,<y> or Newpoint <x> <y>\n";
    } else if (command == "Removepoint") {
        double x, y;
        char comma;
        if (!(ss >> x >> comma >> y) || comma != ',') {
            ss.clear();
            ss.str(input);
            std::string tmp;
            ss >> tmp >> x >> y;
            if (ss.fail())
                return "Error: Usage: Removepoint <x>,<y> or Removepoint <x> <y>\n";
        }
        graph.remove_point(Point(x, y));
        return "OK: Point removed\n";
    } else {
        return "Error: Unknown command\n";
    }
}

void handle_client(int fd) {
    char buf[BUFSIZE] = {0};
    int nbytes = recv(fd, buf, BUFSIZE - 1, 0);
    if (nbytes <= 0) {
        // אם הלקוח מת -> שחרר נעילה אם הוא הבעלים
        release_lock(fd);

        close(fd);
        reactor.remove_fd(fd);
        return;
    }

    partial_inputs[fd] += std::string(buf);
    size_t pos;
    while ((pos = partial_inputs[fd].find('\n')) != std::string::npos) {
        std::string line = partial_inputs[fd].substr(0, pos);
        partial_inputs[fd].erase(0, pos + 1);
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        std::string response = handle_command(fd, line);
        send(fd, response.c_str(), response.size(), 0);
    }
}

int main() {
    // טיפול ב־Ctrl+C בשרת עצמו -> סגירה יפה
    signal(SIGINT, [](int) {
        std::cout << "\nServer interrupted. Shutting down...\n";
        exit(0);
    });

    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        return 1;
    }

    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    struct sockaddr_in serv_addr = {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(listener, 10) < 0) {
        perror("listen");
        return 1;
    }

    reactor.add_fd(listener, [listener](int fd) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int client_fd = accept(fd, (struct sockaddr*)&client_addr, &addrlen);
        if (client_fd >= 0) {
            reactor.add_fd(client_fd, handle_client);
            const std::string welcome = "Welcome to CH server. Send commands.\n";
            send(client_fd, welcome.c_str(), welcome.size(), 0);
        }
    });

    std::cout << "Step 6 server running on port " << PORT << "...\n";
    reactor.run();
    return 0;
}
