#include <iostream>
#include <string>
#include <unistd.h>     // for STDIN_FILENO
#include <functional>
#include "reactor.hpp"  // ודא שזה הנתיב הנכון לקובץ שלך

int main() {
    Reactor<std::function<void(int)>> reactor;

    reactor.add_fd(STDIN_FILENO, [](int fd) {
        (void)fd;
        std::string input;
        std::getline(std::cin, input);
        std::cout << "You typed: " << input << std::endl;
    });

    std::cout << "Reactor running... type something:\n";
    reactor.run();  // לא יוצא אף פעם – עצירה ידנית עם Ctrl+C

    return 0;
}
