// פroactor8.hpp - תבנית של שלב 8 (Proactor)
#pragma once

#include <pthread.h>
#include <unordered_map>
#include <functional>
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>

using proactorFunc = void* (*)(int sockfd);

class Proactor {
private:
    std::unordered_map<pthread_t, int> active_threads;

public:
    pthread_t startProactor(int sockfd, proactorFunc threadFunc) {
        pthread_t tid;
        int* arg = new int(sockfd);  // allocate int on heap to pass pointer
        if (pthread_create(&tid, nullptr, [](void* arg_ptr) -> void* {
                int fd = *((int*)arg_ptr);
                delete (int*)arg_ptr;
                extern void* threadEntry(int); // must be implemented externally
                return threadEntry(fd);
            }, (void*)arg) == 0) {
            active_threads[tid] = sockfd;
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
