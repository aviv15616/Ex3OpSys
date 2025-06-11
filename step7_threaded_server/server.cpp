// step7_threaded_server.cpp
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <map>
#include <mutex>
#include <thread>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>

#include "../common/include/Graph.hpp"
#include "../common/include/Point.hpp"
#include "../common/include/CH.hpp"

using namespace std;
using namespace common;

#define PORT 9034
#define BUFSIZE 1024

Graph graph;
mutex graph_mutex;

unordered_map<int, string> partial_inputs;
map<int, int> expected_points_map;
map<int, bool> awaiting_points_map;
bool graph_busy = false;
int current_owner_fd = -1;

string handle_command(int fd, const string& input) {
    lock_guard<mutex> lock(graph_mutex);  // הגנה על כל שינוי בגרף ובמפות

    if (graph_busy && fd != current_owner_fd)
        return "Graph is busy, try again later.\n";

    if (awaiting_points_map[fd]) {
        string coords = input;
        if (input.find("Newpoint") == 0) {
            size_t pos = input.find(' ');
            if (pos == string::npos) return "Error: Expected point format after Newpoint\n";
            coords = input.substr(pos + 1);
        }

        double x, y;
        char comma;
        bool parsed = false;
        stringstream ss(coords);
        if ((ss >> x >> comma >> y) && comma == ',') parsed = true;
        else {
            ss.clear(); ss.str(coords);
            if (ss >> x >> y) parsed = true;
        }
        if (!parsed) return "Error: Expected point format: x,y or x y\n";

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

    stringstream ss(input);
    string command;
    ss >> command;

    if (command == "Newgraph") {
        int n;
        if (!(ss >> n)) return "Error: Usage: Newgraph <n>\n";
        if (graph_busy) return "Graph is busy, try again later.\n";

        graph.clear();
        expected_points_map[fd] = n;
        awaiting_points_map[fd] = true;
        graph_busy = true;
        current_owner_fd = fd;
        return "OK: Send " + to_string(n) + " point(s) in format x,y or x y\n";
    }
    else if (command == "Newpoint") {
        size_t pos = input.find(' ');
        if (pos == string::npos) return "Error: Usage: Newpoint <x>,<y> or Newpoint <x> <y>\n";
        string coords = input.substr(pos + 1);

        double x, y; char comma;
        stringstream ss(coords);
        if ((ss >> x >> comma >> y) && comma == ',') {
            graph.add_point(Point(x, y));
            return "OK: Point added\n";
        }
        ss.clear(); ss.str(coords);
        if ((ss >> x >> y)) {
            graph.add_point(Point(x, y));
            return "OK: Point added\n";
        }
        return "Error: Usage: Newpoint <x>,<y> or Newpoint <x> <y>\n";
    }
    else if (command == "Removepoint") {
        double x, y; char comma;
        if (!(ss >> x >> comma >> y) || comma != ',') {
            ss.clear(); ss.str(input);
            string tmp; ss >> tmp >> x >> y;
            if (ss.fail()) return "Error: Usage: Removepoint <x>,<y> or Removepoint <x> <y>\n";
        }
        graph.remove_point(Point(x, y));
        return "OK: Point removed\n";
    }
    else if (command == "CH") {
        auto hull = compute_convex_hull(graph.get_points());
        stringstream out;
        out << "Convex Hull:\n";
        for (const auto& p : hull)
            out << p.x << "," << p.y << "\n";
        out << "Area of convex hull: " << compute_area(hull) << "\n";
        return out.str();
    }
    else {
        return "Error: Unknown command\n";
    }
}

void handle_client(int fd) {
    char buf[BUFSIZE] = {0};
    while (true) {
        int nbytes = recv(fd, buf, BUFSIZE - 1, 0);
        if (nbytes <= 0) break;

        partial_inputs[fd] += string(buf, nbytes);
        size_t pos;
        while ((pos = partial_inputs[fd].find('\n')) != string::npos) {
            string line = partial_inputs[fd].substr(0, pos);
            partial_inputs[fd].erase(0, pos + 1);

            // סינון תווים לא רלוונטיים
            line.erase(remove(line.begin(), line.end(), '\r'), line.end());
            line.erase(remove(line.begin(), line.end(), '\n'), line.end());

            // ✅ דילוג על שורות ריקות
            if (line.empty()) continue;

            string response = handle_command(fd, line);
            send(fd, response.c_str(), response.size(), 0);
        }
    }

    close(fd);
    lock_guard<mutex> lock(graph_mutex);
    partial_inputs.erase(fd);
    expected_points_map.erase(fd);
    awaiting_points_map.erase(fd);
    if (fd == current_owner_fd) {
        graph_busy = false;
        current_owner_fd = -1;
    }
}

int main() {
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

    cout << "Step 7 threaded server running on port " << PORT << "...\n";

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int client_fd = accept(listener, (struct sockaddr*)&client_addr, &addrlen);
        if (client_fd >= 0) {
            thread t(handle_client, client_fd);
            t.detach();
            const string welcome = "Welcome to CH server. Send commands.\n";
            send(client_fd, welcome.c_str(), welcome.size(), 0);
        }
    }

    return 0;
}
