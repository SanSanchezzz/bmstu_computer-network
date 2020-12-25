#include <memory>

#include "engine.h"

class Server {
public:
    Server(int port);
    void run();

private:
    std::unique_ptr<Engine> m_Engine;
};
