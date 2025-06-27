#pragma once

#include <fmt/core.h>

#include "session.h"
#include <boost/asio.hpp>

using Tcp = boost::asio::ip::tcp;
using Map = std::unordered_map<std::string, std::shared_ptr<Session>>;

class Server : public std::enable_shared_from_this<Server> {
   private:
    Map                      clients_;
    boost::asio::io_context& io_;
    uint16_t                 port_;
    Tcp::acceptor            acceptor_;

   public:
    Server(boost::asio::io_context& io, uint16_t port)
      : io_(io), port_(port), acceptor_(io_, Tcp::endpoint(Tcp::v4(), port)){};
    void start();
    void doAccept();
    void ioRun();

    bool contains(const std::string& clientLogin);
    void save(const std::string& clientLogin, std::shared_ptr<Session> session);
    bool deleteClient(const std::string& clientLogin);
    std::shared_ptr<Session> getSession(const std::string& name) const;
};
