// step5_reactor_template - main entry point
#pragma once
#include <unordered_map>
#include <functional>
#include <sys/select.h>
#include <unistd.h>

template<typename Callback>
class Reactor {
private:
    std::unordered_map<int, Callback> fd_callbacks;
    fd_set master_set;
    int fd_max = 0;
    bool running = false;

public:
    Reactor() {
        FD_ZERO(&master_set);
    }

    void add_fd(int fd, Callback cb) {
        fd_callbacks[fd] = cb;
        FD_SET(fd, &master_set);
        if (fd > fd_max) fd_max = fd;
    }

    void remove_fd(int fd) {
        fd_callbacks.erase(fd);
        FD_CLR(fd, &master_set);
    }

    void run() {
        running = true;
        while (running) {
            fd_set read_fds = master_set;
            if (select(fd_max + 1, &read_fds, nullptr, nullptr, nullptr) == -1) {
                perror("select");
                break;
            }

            for (int fd = 0; fd <= fd_max; ++fd) {
                if (FD_ISSET(fd, &read_fds) && fd_callbacks.count(fd)) {
                    fd_callbacks[fd](fd); // Call the registered callback
                }
            }
        }
    }

    void stop() {
        running = false;
    }
};
