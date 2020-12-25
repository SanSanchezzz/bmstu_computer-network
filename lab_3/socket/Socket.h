#include <unistd.h>  // close()
#include <memory>
#include <string>

std::string int2ipv4(uint32_t ip);

class Socket {
public:
    Socket() : m_Sd(-1) {}
    Socket(int sd) : m_Sd(sd) {}
    ~Socket() {
        if (m_Sd > 0)
            ::close(m_Sd);
    }

public:
    int sd() { return m_Sd; };
    void connect(const std::string& host, int port) throw(std::exception);
    void connect(const std::string& host,
                 int port,
                 int timeout) throw(std::exception);
    void send(const std::string& s) const throw(std::exception);
    bool hasData() const throw(std::exception);
    std::string recv_loop() const throw(std::exception);
//    std::string recvTimed(int timeout) throw(std::exception);
    void setRcvTimeout(int sec, int microsec) const throw(std::exception);
    void setNonBlocked(bool opt) const throw(std::exception);
    static void setReuseAddr(int sd) throw(std::exception);
    void createServerSocket(uint32_t port,
                            uint32_t queue_size) throw(std::exception);
    std::shared_ptr<Socket> accept() const throw(std::exception);
    void close() { ::close(m_Sd); }

    void httpQuery(
            const std::string& query,
            std::function<void(const std::string& s)> cb) const throw(std::exception);

private:
    int m_Sd;
};
