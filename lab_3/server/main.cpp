#include <iostream>
#include "server.h"

int main(int argc, char* argv[]) {
    try {
        Server serv(8080);
        serv.run();
        return 0;
    } catch (std::exception& e) {
        std::cerr << "Terminated: " << e.what() << std::endl;
    }
    return 0;
}
