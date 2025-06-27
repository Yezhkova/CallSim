#include "client.h"

#include "messageBuilder.h"

void Client::start() {
    auto self = shared_from_this();
    socket_.async_connect(endpoint_, [self](boost::system::error_code ec) {
        if (!ec) {
            fmt::println(
                "To register, enter 'register <your login>', or 'exit'");
            self->readHeader();
        } else {
            fmt::println(stderr,
                         "[Client] Failed to connect: {}",
                         ec.message());
        }
    });
}

void Client::readHeader() {
    auto self        = shared_from_this();
    auto body_length = std::make_shared<uint32_t>(0);
    auto header_buf  = boost::asio::buffer(body_length.get(), sizeof(uint32_t));
    boost::asio::async_read(
        socket_,
        header_buf,
        [self, body_length](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                *body_length = ntohl(*body_length);
                self->readBody(*body_length);
            }
        });
}

void Client::readBody(uint32_t length) {
    auto self     = shared_from_this();
    auto body_buf = std::make_shared<std::vector<char>>(length);
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(*body_buf),
        [self, body_buf](boost::system::error_code ec, std::size_t) {
            Message msg;
            if (msg.ParseFromArray(body_buf->data(), body_buf->size())) {
                fmt::println("{}", toPrintable(msg));
                self->nextState(msg);
                self->readHeader();
            } else {
                fmt::println(stderr, "Client parsing error: {}", ec.what());
            }
        });
}

void Client::sendMessage(const Message& msg_proto) {
    auto        self = shared_from_this();
    std::string serialized;
    if (!msg_proto.SerializeToString(&serialized)) {
        fmt::println(stderr, "[Client] Failed to serialize protobuf");
        return;
    }
    auto     msg = std::make_shared<std::string>();
    uint32_t len = htonl(msg_proto.ByteSizeLong());
    msg->append(reinterpret_cast<const char*>(&len), sizeof(len));
    msg->append(serialized);

    boost::asio::async_write(
        self->socket_,
        boost::asio::buffer(*msg),
        [self, msg](boost::system::error_code ec, std::size_t) {
            if (ec) {
                fmt::println("[Client::sendMessage] error: {}", ec.what());
            }
        });
}

void Client::onRegister(const std::string& name) {
    Message msg = MessageBuilder()
                      .type(Register)
                      .from(name)
                      .payload(fmt::format(
                          "{}:{} is trying to register",
                          socket_.local_endpoint().address().to_string(),
                          std::to_string(socket_.local_endpoint().port())))
                      .build();
    nextState(msg);
}
void Client::onExit() {

}