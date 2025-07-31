#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <mutex>
#include <thread>
#include <atomic>
#include <csignal>
#include <cctype>
#include <condition_variable>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../common/include/Graph.hpp"
#include "../common/include/Point.hpp"
#include "../common/include/CH.hpp"
#include "../step8_proactor_template/proactor.hpp"

using namespace std;
using namespace common;

#define PORT 9034
#define BUFSIZE 1024

Graph graph;
mutex graph_mutex;

unordered_map<int, string> partial_inputs;
map<int, int> expected_points_map;
map<int, bool> awaiting_points_map;
set<int> clients;

bool graph_busy = false;
int current_owner_fd = -1;
atomic<bool> running(true);

/// ---- producer-consumer vars ----
condition_variable area_cv;
bool graph_updated = false;
bool from_ch_command = false;   // דגל האם העדכון נגרם מפקודת CH

/// ---- trim & sanitize ----
string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

string sanitize(const string &s) {
    string clean;
    clean.reserve(s.size());
    for (unsigned char c : s) {
        if (isprint(c) || isspace(c)) {
            clean.push_back(c);
        }
    }
    return clean;
}

/// ---- area monitor thread ----
void area_monitor_thread() {
    bool was_large = false;

    while (running) {
        unique_lock<mutex> lock(graph_mutex);
        area_cv.wait(lock, [] { return graph_updated; });
        graph_updated = false;

        auto hull = compute_convex_hull(graph.get_points());
        double area = compute_area(hull);

        // הדפסה רק אם נשלחה פקודת CH
        if (from_ch_command) {
            if (area >= 100 && !was_large) {
                cout << "[SERVER] At Least 100 units belongs to CH" << endl;
                was_large = true;
            } else if (area < 100 && was_large) {
                cout << "[SERVER] At Least 100 units no longer belongs to CH" << endl;
                was_large = false;
            }
            from_ch_command = false;
        }
    }
}

/// ---- handle commands ----
string handle_command(int fd, const string &input_raw) {
    string input = sanitize(trim(input_raw));
    if (input.empty()) return "";

    cout << "[SERVER] Client " << fd << " -> " << input << endl;

    stringstream ss(input);
    string command;
    ss >> command;

    // --- Newgraph ---
    if (command == "Newgraph" || command == "newgraph") {
        int n;
        if (!(ss >> n)) return "Error: Usage: Newgraph <n>\n";

        {
            lock_guard<mutex> lock(graph_mutex);
            if (graph_busy && fd != current_owner_fd) {
                cout << "[SERVER] Client " << fd << " tried Newgraph while graph is busy" << endl;
                return "Graph is busy, wait for current initialization to finish.\n";
            }

            graph.clear();
            expected_points_map[fd] = n;
            awaiting_points_map[fd] = true;
            graph_busy = true;
            current_owner_fd = fd;
        }

        cout << "[SERVER] Newgraph started by client " << fd << " expecting " << n << " points." << endl;
        return "OK: Send " + to_string(n) + " points (x,y)\n";
    }

    // --- CH ---
    if (command == "CH" || command == "ch") {
        vector<Point> points;
        {
            lock_guard<mutex> lock(graph_mutex);
            points = graph.get_points();

            if (fd == current_owner_fd) {
                awaiting_points_map[fd] = false;
                expected_points_map.erase(fd);
                graph_busy = false;
                current_owner_fd = -1;
            }

            graph_updated = true;
            from_ch_command = true;    // מסמן שהעדכון הגיע מפקודת CH
            area_cv.notify_all();
        }

        cout << "[SERVER] Client " << fd << " requested CH" << endl;

        auto hull = compute_convex_hull(points);
        stringstream out;
        out << "Convex Hull:\n";
        for (const auto &p : hull)
            out << p.x << "," << p.y << "\n";
        out << "Area of convex hull: " << compute_area(hull) << "\n";
        return out.str();
    }

    // --- Points during Newgraph ---
    {
        lock_guard<mutex> lock(graph_mutex);

        if (graph_busy && awaiting_points_map[current_owner_fd]) {
            string coords = input;

            if (fd != current_owner_fd && input.find("Newpoint") != 0) {
                cout << "[SERVER] Client " << fd << " tried invalid command during Newgraph" << endl;
                return "Error: Only 'Newpoint x,y' is allowed for other clients during Newgraph.\n";
            }

            if (input.find("Newpoint") == 0) {
                size_t pos = input.find(' ');
                if (pos == string::npos)
                    return "Error: Expected point format after Newpoint\n";
                coords = input.substr(pos + 1);
            }

            double x, y; char comma;
            stringstream ssp(coords);
            bool parsed = false;
            if ((ssp >> x >> comma >> y) && comma == ',') parsed = true;
            else {
                ssp.clear(); ssp.str(coords);
                if (ssp >> x >> y) parsed = true;
            }
            if (!parsed)
                return "Error: Expected point format: x,y or x y\n";

            graph.add_point(Point(x, y));
            expected_points_map[current_owner_fd]--;

            cout << "[SERVER] Client " << fd << " added point (" << x << "," << y << ")" << endl;

            if (expected_points_map[current_owner_fd] <= 0) {
                awaiting_points_map[current_owner_fd] = false;
                expected_points_map.erase(current_owner_fd);
                graph_busy = false;
                current_owner_fd = -1;

                cout << "[SERVER] Graph initialized with all points." << endl;
                return "Graph initialized with all points.\n";
            }

            return "OK: Point added. Waiting for " + to_string(expected_points_map[current_owner_fd]) + " more...\n";
        }
    }

    // --- Newpoint רגיל ---
    if (command == "Newpoint" || command == "newpoint") {
        size_t pos = input.find(' ');
        if (pos == string::npos)
            return "Error: Usage: Newpoint <x>,<y>\n";

        string coords = input.substr(pos + 1);
        double x, y; char comma;
        stringstream ssp(coords);
        bool parsed = false;
        if ((ssp >> x >> comma >> y) && comma == ',') parsed = true;
        else {
            ssp.clear(); ssp.str(coords);
            if (ssp >> x >> y) parsed = true;
        }
        if (!parsed)
            return "Error: Usage: Newpoint <x>,<y> or Newpoint <x> <y>\n";

        {
            lock_guard<mutex> lock(graph_mutex);
            graph.add_point(Point(x, y));
        }

        cout << "[SERVER] Client " << fd << " added point (" << x << "," << y << ")" << endl;
        return "OK: Point added\n";
    }
    if (command == "Removepoint" || command == "removepoint") {
    double x, y; 
    char comma;
    if (!(ss >> x >> comma >> y) || comma != ',') {
        // ניסיון קריאה בפורמט x y
        ss.clear(); 
        ss.str(input);
        string tmp;
        ss >> tmp >> x >> y;
        if (ss.fail())
            return "Error: Usage: Removepoint <x>,<y> or Removepoint <x> <y>\n";
    }

    {
        lock_guard<mutex> lock(graph_mutex);
        graph.remove_point(Point(x, y));
    }

    cout << "[SERVER] Client " << fd << " removed point (" << x << "," << y << ")" << endl;
    return "OK: Point removed\n";
}

    cout << "[SERVER] Unknown command from client " << fd << endl;
    return "Error: Unknown command\n";
}
/// ---- threadEntry ----
void* threadEntry(int fd) {
    send(fd, "Welcome to CH Producer-Consumer server. Send commands.\n", 55, 0);

    char buf[BUFSIZE];
    while (running) {
        ssize_t n = recv(fd, buf, BUFSIZE - 1, 0);
        if (n <= 0) break;

        buf[n] = '\0';
        string input(buf);
        input = trim(input);
        if (input.empty()) continue;

        string response = handle_command(fd, input);
        if (!response.empty()) {
            if (send(fd, response.c_str(), response.size(), 0) < 0) {
                perror("[SERVER] send failed");
                break;
            }
        }
    }

    close(fd);
    return nullptr;
}

/// ---- main ----
int main() {
    signal(SIGPIPE, SIG_IGN); // להתעלם מ-Broken pipe

    int listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_fd < 0) { perror("socket"); return 1; }

    int yes = 1;
    setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(listener_fd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) { perror("bind"); return 1; }
    if (listen(listener_fd, 10) < 0) { perror("listen"); return 1; }

    cout << "[SERVER] Step 10 Producer-Consumer server running on port " << PORT << "...\n";

    // שרשור נפרד למעקב אחרי CH
    thread monitor(area_monitor_thread);

    Proactor proactor;
    pthread_t tid = proactor.startProactor(listener_fd, threadEntry);

    pthread_join(tid, nullptr);
    running = false;
    area_cv.notify_all();

    monitor.join();
    close(listener_fd);
    return 0;
}
