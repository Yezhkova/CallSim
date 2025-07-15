#include "session.h"

#include "server.h"

Session::~Session() {
    fmt::println("Session destroyed. Left Online: {} client(s)",
                 getServer()->getClients().size());
};

void Session::readHeader() {
    auto self        = shared_from_this();
    auto body_length = std::make_shared<uint32_t>(0);
    auto header_buf  = boost::asio::buffer(body_length.get(), sizeof(uint32_t));

    boost::asio::async_read(
        socket_,
        header_buf,
        [self, body_length](boost::system::error_code ec, std::size_t) {
            if (ec == boost::asio::error::eof ||
                ec == boost::asio::error::connection_reset) {
                fmt::println("Client {} disconnected", self->getData());
                self->nextState(MessageBuilder::exitQuery(self->username_));
                return;
            }
            if (ec) {
                fmt::println(stderr,
                             "Read header error ({}): {}",
                             self->getData(),
                             ec.message());
                return;
            }
            *body_length = ntohl(*body_length);
            self->readBody(body_length);
        });
}

void Session::readBody(std::shared_ptr<uint32_t> length) {
    auto self = shared_from_this();
    body_buf_.resize(*length);

    boost::asio::async_read(
        socket_,
        boost::asio::buffer(body_buf_),
        [self](boost::system::error_code ec, std::size_t) {
            if (ec == boost::asio::error::eof ||
                ec == boost::asio::error::connection_reset) {
                fmt::println("Client {} disconnected", self->getData());
                return;
            }
            if (ec) {
                fmt::println(stderr, "[Server] Error: {}", ec.what());
                return;
            }
            Message msg;
            if (msg.ParseFromArray(self->body_buf_.data(),
                                   self->body_buf_.size())) {
                fmt::println("{}", msg);
                try {
                    self->nextState(msg);
                } catch (const InvalidTransitionException& ex) {
                    self->sendMessageToClient(
                        MessageBuilder::operationDenied(msg.type()));
                } catch (const std::exception& ex) {
                    fmt::println(stderr, "[Server] {}", ex.what());
                    self->close();
                }
                if (self->isOpen()) {
                    self->readHeader();
                }
            } else {
                fmt::println(stderr, "[Server] Failed to parse message");
            }
        });
}

void Session::sendMessageToClient(const Message& msg_proto) {
    auto        self = shared_from_this();
    std::string serialized;
    if (!msg_proto.SerializeToString(&serialized)) {
        fmt::println(stderr, "[Server] Failed to serialize protobuf");
        return;
    }
    auto     msg = std::make_shared<std::string>();
    uint32_t len = htonl(msg_proto.ByteSizeLong());
    msg->append(reinterpret_cast<const char*>(&len), sizeof(len));
    msg->append(serialized);
    boost::asio::async_write(socket_,
                             boost::asio::buffer(*msg),
                             [self](boost::system::error_code ec, std::size_t) {
                                 if (ec) {
                                     fmt::println(
                                         stderr,
                                         "[Server] Failed to write: {}",
                                         ec.message());
                                 }
                             });
}

void Session::sendMessageToSubscriberServer(const std::string& name,
                                            const Message&     msg) {
    auto it = getServer()->getSession(name);
    if (!it) {
        return;
    }
    it->nextState(msg);
}
void Session::sendMessageToSubscriberClient(const std::string& name,
                                            const Message&     msg) {
    auto it = getServer()->getSession(name);
    if (!it) {
        return;
    }
    it->sendMessageToClient(msg);
}

bool Session::registerClient(const std::string& name) {
    if (name.empty()) {
        sendMessageToClient(MessageBuilder::registrationDenied(name));
        return false;
    }
    if (getServer()->contains(name)) {
        sendMessageToClient(MessageBuilder::registrationDenied(name));
        return false;
    }
    getServer()->save(name, shared_from_this());
    sendMessageToClient(MessageBuilder::registrationConfirmed(name));
    username_ = name;
    return true;
}

bool Session::callClient(const std::string& sender,
                         const std::string& receiver) {
    auto receiver_it = getServer()->getSession(receiver);
    if (receiver_it && receiver_it != shared_from_this()) {
        receiver_it->nextState(MessageBuilder::answerQuery(sender, receiver));

        receiver_it->timer_->expires_after(std::chrono::seconds(10));
        receiver_it->timer_->async_wait(
            [receiver_it, receiver](const boost::system::error_code& ec) {
                if (ec) {
                    fmt::println("'Answer' timer: {}", ec.message());
                    return;
                }
                fmt::println("Timeout reached, automatic Reject emitted.");
                receiver_it->nextState(MessageBuilder::rejectQuery(receiver));
            });
        return true;
    }
    return false;
}

std::shared_ptr<Server> Session::getServer() const {
    auto s = server_.lock();
    if (!s) {
        throw std::runtime_error("Server reference is expired");
    }
    return s;
}

bool Session::deleteClient(const std::string& name) {
    return getServer()->deleteClient(name);
}

std::string Session::getData() const {
    return fmt::format("{}:{} ({})",
                       socket_.remote_endpoint().address().to_string(),
                       socket_.remote_endpoint().port(),
                       username_);
}

boost::asio::io_context& Session::getContext() const {
    return getServer()->getContext();
};

std::shared_ptr<boost::asio::steady_timer> Session::getTimer() const {
    return timer_;
}

void Session::close() {
    fmt::println("Session::close() : {}", getData());
    boost::system::error_code ec;
    socket_.shutdown(Tcp::socket::shutdown_receive, ec);
    socket_.close(ec);
}
