#include "session.h"

#include "server.h"

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
                fmt::println("Client {} disconnected", self->getEndpoint());
                return;
            }
            if (ec) {
                fmt::println(stderr,
                             "Read header error ({}): {}",
                             self->getEndpoint(),
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
                fmt::print("Client {} disconnected\n", self->getEndpoint());
                fmt::println("{}", ec.what());
                return;
            }
            if (ec) {
                fmt::println(stderr, "[Server] Error: {}", ec.what());
                return;
            }
            Message msg;
            if (msg.ParseFromArray(self->body_buf_.data(),
                                   self->body_buf_.size())) {
                fmt::println("{}", self->toPrintable(msg));
                self->nextState(msg);
                if (self->isOpen()) {
                    self->readHeader();
                }
            } else {
                fmt::println(stderr, "[Server] Failed to parse message");
            }
        });
}

void Session::sendMessage(const Message& msg_proto) {
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

bool Session::registerClient(const std::string& name) {
    if (name.empty()) {
        sendMessage(MessageBuilder::registrationDenied(name));
        return false;
    }
    if (getServer()->contains(name)) {
        sendMessage(MessageBuilder::registrationDenied(name));
        return false;
    }
    getServer()->save(name, shared_from_this());
    sendMessage(MessageBuilder::registrationConfirmed(name));
    return true;
}

bool Session::deleteClient(const std::string& name) {
    return getServer()->deleteClient(name);
}

std::string Session::getEndpoint() const {
    return socket_.remote_endpoint().address().to_string() + ':' +
           std::to_string(socket_.remote_endpoint().port());
}

void Session::close() {
    fmt::print("Client {} disconnected\n", getEndpoint());
    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close(ec);
}

inline std::string Session::toPrintable(const Message& msg) {
    std::string result;
    std::time_t t = static_cast<std::time_t>(msg.timestamp() / 1000);
    char        buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    result += fmt::format("{}", buf);
    result += fmt::format(" - [{}] ", getEndpoint());
    result += fmt::format(" - '{}'", magic_enum::enum_name(msg.type()));
    // result += fmt::format(" -> [{}]", msg.to_user());
    return result;
}
