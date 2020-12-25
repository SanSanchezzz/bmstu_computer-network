#include <iostream>
#include <memory>  // shared_ptr
#include <stdexcept>
#include <cstdlib>
#include <utility>

#include "../socket/Socket.h"

struct HttpQuery {
    HttpQuery(std::string  q, std::string  h) : query(std::move(q)), host(std::move(h)) {}
    std::string query;
    std::string host;
};

std::shared_ptr<HttpQuery> make_query(const std::string& response) {
    std::string::size_type pos = response.find("//");
    if (pos == std::string::npos)
        throw std::runtime_error("bad query");

    // from "http://www.any.com/sdsad" --> host = 'www.any.com'
    std::string host =
            response.substr(pos + 2, response.find("/", pos + 2) - pos - 2);
    if (host.empty())
        throw std::runtime_error("bad query");

    std::string query("GETJHAFHAFJKS " + response + " HTTP/1.1\r\n");
    query += "Host: " + host + "\r\n";
    query += "\r\n";

    std::shared_ptr<HttpQuery> q = std::make_shared<HttpQuery>(query, host);
    return q;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " query" << std::endl;
        std::cerr << "e.g.: http://ya.ru 8080" << std::endl;

        return 0;
    }

    try {
        std::shared_ptr<HttpQuery> q = make_query(argv[1]);
        Socket s;

        int port = atoi(argv[2]);
        if (port == 0)
            std::cerr << "Port must be int" << std::endl;

        s.connect(q->host, port);
        s.setRcvTimeout(/*sec*/ 1, /*microsec*/ 0);

        s.httpQuery(q->query, [](const std::string& resp) -> void {
            std::string::size_type pos = resp.find("\r\n\r\n");
            if (pos != std::string::npos)
                std::cerr << "headers:" << std::endl << resp.substr(0, pos) << std::endl;
//                throw std::runtime_error("exception: error getting headers");

        });
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
