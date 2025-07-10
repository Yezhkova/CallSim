#include "server.h"

void Server::start() {
    fmt::println("=========Server started at port {}=========", port_);
    doAccept();
}

void Server::doAccept() {
    auto self   = shared_from_this();
    auto socket = std::make_shared<Tcp::socket>(io_);
    acceptor_.async_accept([self](boost::system::error_code ec,
                                  Tcp::socket               socket) {
        if (!ec) {
            auto session = std::make_shared<Session>(std::move(socket), self);
            session->start();
        } else {
            fmt::println(stderr, "Accept error: {}", ec.message());
        }
        self->doAccept();
    });
}

void Server::stop() {
    clients_.clear();
    boost::system::error_code ec;
    acceptor_.cancel(ec);
    acceptor_.close(ec);
    io_.stop();
}

bool Server::contains(const std::string& clientLogin) const {
    return (clients_.find(clientLogin) != clients_.end());
}

void Server::save(const std::string&       clientLogin,
                  std::shared_ptr<Session> session) {
    clients_[clientLogin] = std::move(session);
}

bool Server::deleteClient(const std::string& clientLogin) {
    return clients_.erase(clientLogin);
}

std::shared_ptr<Session> Server::getSession(const std::string& name) const {
    if (auto it = clients_.find(name); it != clients_.end()) {
        return it->second;
    }
    return nullptr;
}
