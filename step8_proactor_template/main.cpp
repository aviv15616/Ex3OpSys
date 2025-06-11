#include "proactor.hpp"

#include <iostream>
#include <unistd.h>
#include <string>
#include <netinet/in.h>  // sockaddr_in, INADDR_ANY, htons
#include <sys/socket.h>  // socket, bind, listen, accept, send
#include <arpa/inet.h>   // htons
#include <cstring>       // memset

using namespace std;

void* handle_client(int client_fd) {
    std::string message = "Hello from proactor!\n";
    send(client_fd, message.c_str(), message.size(), 0);
    close(client_fd);
    return nullptr;
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9034);

    if (bind(listener, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(listener, 5) < 0) {
        perror("listen");
        return 1;
    }

    cout << "Waiting for client...\n";

    sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_fd = accept(listener, (sockaddr*)&client_addr, &len);
    if (client_fd < 0) {
        perror("accept");
        return 1;
    }

    Proactor p;
    pthread_t tid = p.startProactor(client_fd, handle_client);
    pthread_join(tid, nullptr);

    return 0;
}
