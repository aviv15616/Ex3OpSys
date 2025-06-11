// step10_producer_consumer - main entry point
// step10_proactor_server.cpp
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <fcntl.h>

#include "../common/include/Graph.hpp"
#include "../common/include/Point.hpp"
#include "../common/include/CH.hpp"

using namespace std;
using namespace common;

#define PORT 9034
#define BUFSIZE 1024
#define THREAD_POOL_SIZE 4

Graph graph;
mutex graph_mutex;
bool graph_busy = false;
int current_owner_fd = -1;
unordered_map<int, int> expected_points;
unordered_map<int, bool> awaiting_points;

condition_variable area_cv;
bool graph_updated = false;
bool running = true;

struct Task {
    int client_fd;
    string command;
};

queue<Task> task_queue;
mutex queue_mutex;
condition_variable cv;

void area_monitor_thread() {
    bool was_large = false;
    while (true) {
        unique_lock<mutex> lock(graph_mutex);
        area_cv.wait(lock, [] { return graph_updated; });
        graph_updated = false;

        auto hull = compute_convex_hull(graph.get_points());
        double area = compute_area(hull);

        if (area >= 100 && !was_large) {
            cout << "At Least 100 units belongs to CH" << endl;
            was_large = true;
        } else if (area < 100 && was_large) {
            cout << "At Least 100 units no longer belongs to CH" << endl;
            was_large = false;
        }
    }
}

void worker_thread() {
    while (running) {
        Task task;
        {
            unique_lock<mutex> lock(queue_mutex);
            cv.wait(lock, [] { return !task_queue.empty() || !running; });
            if (!running && task_queue.empty()) return;
            task = task_queue.front();
            task_queue.pop();
        }

        int fd = task.client_fd;
        string input = task.command;
        string response;

        {
            lock_guard<mutex> lock(graph_mutex);

            if (graph_busy && fd != current_owner_fd && input.find("Newgraph") != 0) {
                response = "Graph is busy, try again later.\n";
                send(fd, response.c_str(), response.size(), 0);
                continue;
            }

            if (awaiting_points[fd]) {
                string coords = input;
                if (input.find("Newpoint") == 0) {
                    size_t pos = input.find(' ');
                    if (pos == string::npos) {
                        response = "Error: Expected point format after Newpoint\n";
                        send(fd, response.c_str(), response.size(), 0);
                        continue;
                    }
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
                if (!parsed) {
                    response = "Error: Expected point format: x,y or x y\n";
                    send(fd, response.c_str(), response.size(), 0);
                    continue;
                }

                graph.add_point(Point(x, y));
                expected_points[fd]--;

                if (expected_points[fd] <= 0) {
                    awaiting_points[fd] = false;
                    expected_points.erase(fd);
                    graph_busy = false;
                    current_owner_fd = -1;
                    response = "Graph initialized with all points.\n";
                } else {
                    response = "OK: Point added. Waiting for " + to_string(expected_points[fd]) + " more...\n";
                }
                send(fd, response.c_str(), response.size(), 0);
                continue;
            }

            stringstream ss(input);
            string command;
            ss >> command;

            if (command == "Newgraph") {
                int n;
                if (!(ss >> n)) {
                    response = "Error: Usage: Newgraph <n>\n";
                } else if (graph_busy) {
                    response = "Graph is busy, try again later.\n";
                } else {
                    graph.clear();
                    graph_busy = true;
                    current_owner_fd = fd;
                    expected_points[fd] = n;
                    awaiting_points[fd] = true;
                    response = "OK: Send " + to_string(n) + " point(s) in format x,y or x y\n";
                }
            } else if (command == "Newpoint") {
                double x, y; char comma;
                if ((ss >> x >> comma >> y) && comma == ',') {
                    graph.add_point(Point(x, y));
                    response = "OK: Point added\n";
                } else {
                    ss.clear(); ss.str(input.substr(command.size()));
                    if ((ss >> x >> y)) {
                        graph.add_point(Point(x, y));
                        response = "OK: Point added\n";
                    } else {
                        response = "Error: Usage: Newpoint <x>,<y> or Newpoint <x> <y>\n";
                    }
                }
            } else if (command == "Removepoint") {
                double x, y; char comma;
                if ((ss >> x >> comma >> y) && comma == ',') {
                    graph.remove_point(Point(x, y));
                    response = "OK: Point removed\n";
                } else {
                    ss.clear(); ss.str(input.substr(command.size()));
                    if ((ss >> x >> y)) {
                        graph.remove_point(Point(x, y));
                        response = "OK: Point removed\n";
                    } else {
                        response = "Error: Usage: Removepoint <x>,<y> or Removepoint <x> <y>\n";
                    }
                }
            } else if (command == "CH") {
                auto hull = compute_convex_hull(graph.get_points());
                double area = compute_area(hull);
                stringstream out;
                out << "Convex Hull:\n";
                for (const auto &p : hull)
                    out << p.x << "," << p.y << "\n";
                out << "Area of convex hull: " << area << "\n";
                response = out.str();

                graph_updated = true;
                area_cv.notify_all();
            } else {
                response = "Error: Unknown command\n";
            }
        }

        send(fd, response.c_str(), response.size(), 0);
    }
}

bool set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return flags != -1 && fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

bool read_from_client(int fd, string &buffer) {
    char temp_buf[BUFSIZE];
    while (true) {
        ssize_t n = recv(fd, temp_buf, sizeof(temp_buf), 0);
        if (n > 0) buffer.append(temp_buf, n);
        else if (n == 0) return false;
        else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            return false;
        }
    }
    return true;
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) { perror("socket"); return 1; }

    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(listener, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { perror("bind"); return 1; }
    if (listen(listener, 10) < 0) { perror("listen"); return 1; }
    if (!set_nonblocking(listener)) { cerr << "Failed to set listener socket to non-blocking\n"; return 1; }

    cout << "Step 10 server running on port " << PORT << "...\n";

    vector<thread> workers;
    for (int i = 0; i < THREAD_POOL_SIZE; ++i)
        workers.emplace_back(worker_thread);

    thread area_monitor(area_monitor_thread);

    unordered_map<int, string> partial_inputs;
    fd_set master_set, read_fds;
    FD_ZERO(&master_set);
    FD_SET(listener, &master_set);
    int fdmax = listener;

    while (true) {
        read_fds = master_set;
        int ready = select(fdmax + 1, &read_fds, nullptr, nullptr, nullptr);
        if (ready < 0) { perror("select"); break; }

        for (int fd = 0; fd <= fdmax && ready > 0; ++fd) {
            if (FD_ISSET(fd, &read_fds)) {
                ready--;
                if (fd == listener) {
                    sockaddr_in client_addr{};
                    socklen_t addrlen = sizeof(client_addr);
                    int client_fd = accept(listener, (sockaddr *)&client_addr, &addrlen);
                    if (client_fd < 0) { perror("accept"); continue; }
                    if (!set_nonblocking(client_fd)) { cerr << "Failed to set client socket\n"; close(client_fd); continue; }
                    FD_SET(client_fd, &master_set);
                    if (client_fd > fdmax) fdmax = client_fd;
                    const string welcome = "Welcome to CH proactor server.\n";
                    send(client_fd, welcome.c_str(), welcome.size(), 0);
                } else {
                    bool ok = read_from_client(fd, partial_inputs[fd]);
                    if (!ok) {
                        close(fd); FD_CLR(fd, &master_set); partial_inputs.erase(fd); continue;
                    }
                    size_t pos;
                    while ((pos = partial_inputs[fd].find('\n')) != string::npos) {
                        string line = partial_inputs[fd].substr(0, pos);
                        partial_inputs[fd].erase(0, pos + 1);
                        line.erase(remove(line.begin(), line.end(), '\r'), line.end());
                        line.erase(remove(line.begin(), line.end(), '\n'), line.end());
                        if (line.empty()) continue;
                        {
                            lock_guard<mutex> lock(queue_mutex);
                            task_queue.push({fd, line});
                        }
                        cv.notify_one();
                    }
                }
            }
        }
    }

    running = false;
    cv.notify_all();
    for (auto &t : workers) t.join();
    // לא עוצרים את area_monitor כי הוא צריך לרוץ תמיד
    close(listener);
    return 0;
}
