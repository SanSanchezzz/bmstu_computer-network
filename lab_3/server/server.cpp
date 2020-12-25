#include "server.h"
#include "poll_async.h"

namespace {
    std::unique_ptr<Engine> get_engine(int port) {
        std::unique_ptr<Engine> ret;

        ret.reset(new AsyncPollEngine(port));

        return ret;
    }
}  // namespace

Server::Server(int port) {
    m_Engine = get_engine(port);
}

void Server::run() {
    m_Engine->run();
}
