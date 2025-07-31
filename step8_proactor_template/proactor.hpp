#pragma once

#include <pthread.h>
#include <unordered_map>
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>

using proactorFunc = void* (*)(int sockfd);

class Proactor {
private:
    std::unordered_map<pthread_t, int> active_threads;

public:
    pthread_t startProactor(int listener_fd, proactorFunc threadFunc) {
        pthread_t tid;

        auto *args = new std::pair<int, proactorFunc>(listener_fd, threadFunc);

        if (pthread_create(&tid, nullptr, [](void *arg_ptr) -> void* {
            auto *data = static_cast<std::pair<int, proactorFunc>*>(arg_ptr);
            int listener = data->first;
            proactorFunc func = data->second;
            delete data;

            while (true) {
                int client_fd = accept(listener, nullptr, nullptr);
                if (client_fd < 0) {
                    perror("[SERVER] accept failed");
                    continue;
                }

                // נוצר לקוח חדש
                pthread_t client_tid;
                auto *client_args = new std::pair<int, proactorFunc>(client_fd, func);

                pthread_create(&client_tid, nullptr, [](void *ptr) -> void* {
                    auto *cdata = static_cast<std::pair<int, proactorFunc>*>(ptr);
                    int fd = cdata->first;
                    proactorFunc func = cdata->second;
                    delete cdata;

                    return func(fd);
                }, client_args);

                pthread_detach(client_tid);
            }

            return nullptr;
        }, args) == 0)
        {
            active_threads[tid] = listener_fd;
            return tid;
        }

        return -1;
    }

    int stopProactor(pthread_t tid) {
        if (active_threads.count(tid)) {
            int res = pthread_cancel(tid);
            active_threads.erase(tid);
            return res;
        }
        return -1;
    }
};
