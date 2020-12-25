#include <assert.h>
#include <errno.h>
#include <netinet/in.h>  // htons(), INADDR_ANY
#include <sys/poll.h>
#include <sys/socket.h>  // socket()

#include <unistd.h>  // close()
#include <stdio.h>
#include <stdint.h>  // uint32_t
#include <string.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <thread>
#include <vector>

#include "poll_async.h"

using namespace std::placeholders;

void update_stat(void)
{
    char str[1024] = {0};
    time_t t = time(NULL);
    FILE *f = NULL;
    struct tm res;
    static int days[7] = {0};
    static int hours[24] = {0};
    static int total_requests = 0;
    char buf[1024];
    const char *day_names[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    localtime_r(&t, &res);

    hours[res.tm_hour] += 1;
    days[res.tm_wday] += 1;
    total_requests++;

    snprintf(buf, sizeof(buf), "HOUR  COUNT OF REQUESTS  PERS OF REQUESTS\n");
    strcat(str, buf);
    memset(buf, '\0', sizeof(buf));
    for (int i = 0; i < 24; i++)
    {
        snprintf(buf, sizeof(buf), "%d            %d          %g %% \n", i, hours[i], hours[i] / (double)total_requests * 100);
        strcat(str, buf);
        memset(buf, '\0', sizeof(buf));
    }

    snprintf(buf, sizeof(buf), "DAY  COUNT OF REQUESTS  PERS OF REQUESTS\n");
    strcat(str, buf);
    memset(buf, '\0', sizeof(buf));

    for (int i = 0; i < 7; i++)
    {
        snprintf(buf, sizeof(buf), "%s            %d           %g %% \n", day_names[i], days[i], days[i] / (double)total_requests * 100);
        strcat(str, buf);
        memset(buf, '\0', sizeof(buf));
    }

    f = fopen("stats", "w");
    if (f)
    {
        fprintf(f, "%s\n", str);
    }
    fclose(f);
}

char *get_current_date(char *str, int len)
{
    time_t t = time(NULL);
    struct tm res;

    strftime(str, len, RFC1123FMT, localtime_r(&t, &res));
    return str;
}

// CONNECTION

Connection::Connection(int sd, EventLoop* loop) : m_Sd(sd), m_EventLoop(loop) {
  std::cerr << "+Connection: " << m_Sd << "\n";
}
Connection::~Connection() {
  std::cerr << "~Connection: " << m_Sd << "\n";
  close(m_Sd);
}

void Connection::read() {
  auto cb = [con = shared_from_this()](int error) { con->readHandler(error); };

  m_EventLoop->asyncRead(m_Sd, m_ReadBuf, cb);
}

void Connection::readHandler(int error) {
//  char path[1024];
//  char method[3];
    char date[128];
  std::string s = m_ReadBuf;

  if (s.empty()) {
    std::cerr << "thread: " << pthread_self()
              << ". readHandler: abort connection! [sd: " << m_Sd << "]"
              << std::endl;
    return;
  }

  std::string tmp = s;
  while (tmp[tmp.size() - 1] == '\r' || tmp[tmp.size() - 1] == '\n')
    tmp.pop_back();

  std::size_t del1 = tmp.find(" ");
  std::size_t del2 = tmp.substr(tmp.find(" "), sizeof(tmp)).find(" ");
  std::string method = tmp.substr(0, del1);
  std::string path = tmp.substr(del1, del2);
//  std::size_t pos = tmp.find(" ");
//  method = tmp.
  update_stat();

  std::cerr << "thread: " << pthread_self() << ". readHandler: " << s.size()
            << " bytes [" << tmp << "] [sd: " << m_Sd << "]" << std::endl;
//  write("serverus: " + path + "\n");
  std::string status_code = method.compare("GET") == 0 ? "200 OK" : "400 Bad Request";
  std::string response("\nHTTP/1.1 " + status_code +
                      "\r\nServer: myhttp/1.0.0 (MacOs)\r\n"
                      "Date: " + get_current_date(date, sizeof(date)) +
                      "\r\nContent-Type: text/html\r\n"
                      "Content-Length: %ld\r\n"
                      "Connection: Closed\r\n\r\n");
  write(response);

//  if (method == "GET") {
//      write("\nHTTP/1.1 200 OK\r\n"
//            "Server: myhttpd/1.0.0 (Ubuntu64)\r\n"
//            "Date: %s\r\n"
//            "Content-Type: text/html\r\n"
//            "Content-Length: %ld\r\n"
//            "Connection: Closed\r\n\r\n");
//  }
//  else {
//      write("\nHTTP/1.1 400 Bad Request\r\n"
//            "Server: myhttpd/1.0.0 (Ubuntu64)\r\n"
//            "Date: %s\r\n"
//            "Content-Type: text/html\r\n"
//            "Content-Length: %ld\r\n"
//            "Connection: Closed\r\n\r\n");
//  }
}

void Connection::write(const std::string& s) {
  m_WriteBuf = s;  // save string in memory

  auto cb = [con = shared_from_this()](int error) { con->writeHandler(error); };

  m_EventLoop->asyncWrite(m_Sd, m_WriteBuf, cb);
}

void Connection::writeHandler(int error) {
  read();
}

// EVENT LOOP

void EventLoop::deleteClient(int sd) {
  std::unique_lock<std::mutex> lock(
      m_WantWorkQueueMutex);  // without this lock it fault very easy

  auto f = [&sd](const Event& event) { return event._client.sd == sd; };
  auto iterator =
      std::find_if(m_ClientsWantWork.begin(), m_ClientsWantWork.end(), f);
  std::cerr << "delete: " << sd << "\n";
  assert(iterator != m_ClientsWantWork.end());

  m_ClientsWantWork.erase(iterator);
}

void EventLoop::asyncRead(int sd,
                          std::string& str,
                          std::function<void(int)> cb) {
  Event e(Client(sd, client_state_t::WANT_READ), str);
  e._callback = cb;

  std::unique_lock<std::mutex> lock(m_WantWorkQueueMutex);
  m_ClientsWantWork.emplace_back(e);
}

void EventLoop::asyncWrite(int sd,
                           const std::string& str,
                           std::function<void(int)> cb) {
  Event e(Client(sd, client_state_t::WANT_WRITE),
          const_cast<std::string&>(str));  // XXX: const_cast hack !!!
  e._callback = cb;

  std::unique_lock<std::mutex> lock(m_WantWorkQueueMutex);
  m_ClientsWantWork.emplace_back(e);
}

int EventLoop::manageConnections()
/*
    push into _queue READ or WRITE events
*/
{
  std::cerr << "start connection manager thread: " << pthread_self()
            << std::endl;
  struct pollfd fds[32768];  // 2^15

  while (true) {
    std::vector<int> disconnected_clients;

    if (m_ClientsWantWork.empty()) {
      // no clients
      usleep(1000);
      continue;
    }

    for (size_t i = 0; i < m_ClientsWantWork.size(); ++i) {
      fds[i].fd = m_ClientsWantWork[i]._client.sd;

      if (m_ClientsWantWork[i]._client.state == client_state_t::WANT_READ)
        fds[i].events = POLLIN;
      else
        fds[i].events = POLLOUT;
      fds[i].revents = 0;
    }

    int poll_ret =
        poll(fds, m_ClientsWantWork.size(), /* timeout in msec */ 10);

    if (poll_ret == 0) {
      // nothing activity from any clients
      continue;
    } else if (poll_ret == -1) {
      std::cerr << "poll error!\n";
      return -1;
    }

    for (size_t i = 0; i < m_ClientsWantWork.size(); ++i) {
      if (fds[i].revents == 0)
        continue;

      if (fds[i].revents & POLLHUP) {
        // e.g. previous write() was in a already closed sd
        std::cerr << "client hup\n";
        disconnected_clients.push_back(fds[i].fd);
      } else if (fds[i].revents & POLLIN) {
        if (m_ClientsWantWork[i]._client.state != client_state_t::WANT_READ)
          continue;

        {
          std::unique_lock<std::mutex> lock(m_HaveWorkQueueMutex);
          m_ClientsHaveWork.push(m_ClientsWantWork[i]);
        }
        deleteClient(fds[i].fd);
      } else if (fds[i].revents & POLLOUT) {
        if (m_ClientsWantWork[i]._client.state != client_state_t::WANT_WRITE)
          continue;

        {
          std::unique_lock<std::mutex> lock(m_HaveWorkQueueMutex);
          m_ClientsHaveWork.push(m_ClientsWantWork[i]);
        }
        deleteClient(fds[i].fd);
      } else if (fds[i].revents & POLLNVAL) {
        // e.g. if set clos'ed descriptor in poll
        std::cerr << "POLLNVAL !!! need remove this descriptor: " << fds[i].fd
                  << std::endl;
        disconnected_clients.push_back(fds[i].fd);
      } else {
        if (fds[i].revents & POLLERR)
          std::cerr << "WARNING> revents = POLLERR. [SD = " << fds[i].fd << "]"
                    << std::endl;
        else
          std::cerr << "WARNIG> revent = UNKNOWN_EVENT: " << fds[i].revents
                    << " [SD = " << fds[i].fd << "]" << std::endl;
      }
    }

    // remove disconnected clients
    for (size_t i = 0; i < disconnected_clients.size(); ++i)
      deleteClient(disconnected_clients[i]);
  }
  return 0;
}

namespace {
std::mutex conn_manager_mutex;
}

void EventLoop::run()
/*
    pop() from _queue, doing real read() or write()
*/

{
  std::unique_ptr<std::thread> conn_manager;
  {
    std::lock_guard<std::mutex> lock(conn_manager_mutex);
    static int connection_manager_already_started = 0;
    if (connection_manager_already_started == 0) {
      conn_manager.reset(new std::thread(&EventLoop::manageConnections, this));
      usleep(1000);
      ++connection_manager_already_started;
    }
  }


  std::cerr << "start worker thread: " << pthread_self() << "\n";
  while (true) {
    std::string dummy;
    Event event(Client(-1), dummy);
    {
      std::unique_lock<std::mutex> lock(m_HaveWorkQueueMutex);
      if (m_ClientsHaveWork.empty()) {
        lock.unlock();
        usleep(1000);
        continue;
      }

      event = m_ClientsHaveWork.front();
      m_ClientsHaveWork.pop();
    }

    if (event._client.state == client_state_t::WANT_READ) {
      char buf[256];
      int r = read(event._client.sd, buf, sizeof(buf));

      if (r < 0) {
        std::cerr << "some read error!\n";
        sleep(1);
        close(event._client.sd);
        continue;
      }
      buf[r] = '\0';

      if (r > 0) {
        event._data.get().assign(buf, buf + r);
        std::cout << "before call callback read\n";
        event._callback(errno);
      } else if (r == 0) {
        event._data.get().assign("");
        event._callback(errno);
      }
    } else if (event._client.state == client_state_t::WANT_WRITE) {
      write(event._client.sd, event._data.get().c_str(),
            event._data.get().size());
      std::cout << "before call callback write\n";
      event._callback(errno);
    } else {
      throw std::runtime_error("unknown client state");
    }
  }

  conn_manager->join();
}

std::shared_ptr<Connection> accept_work(int sd, EventLoop* ev) {
  int timeout = 3000;  // msec

  std::shared_ptr<Connection> conn;

  struct pollfd fd;
  fd.fd = sd;
  fd.events = POLLIN;
  fd.revents = 0;

  while (1) {
    int poll_ret = poll(&fd, 1, /* timeout in msec */ timeout);

    if (poll_ret == 0) {
      continue;
    } else if (poll_ret == -1) {
      throw std::runtime_error("poll error");
    }

    struct sockaddr_in client;
    memset(&client, 0, sizeof(client));
    socklen_t cli_len = sizeof(client);

    int cli_sd = accept(sd, (struct sockaddr*)&client, &cli_len);
    conn = std::make_shared<Connection>(cli_sd, ev);
    return conn;
  }
}

void AsyncPollEngine::run() {
  std::cerr << "async poll server starts" << std::endl;

  EventLoop& ev = EventLoop::eventLoop();
  std::vector<std::thread> event_loop_threads;

  for (int i = 0; i < 4; ++i) {
    auto thread_f = [&]() { ev.run(); };
    event_loop_threads.emplace_back(std::thread(thread_f));
    usleep(1000);
  }

  while (true) {
    std::shared_ptr<Connection> conn = accept_work(listener(), &ev);
    if (conn) {
      conn->read();
    }
  }
}
