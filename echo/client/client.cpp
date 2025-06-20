#include "client.h"

void Client::start() {
    auto self = shared_from_this();
    fmt::println("=========Client started=========");
    socket_.async_connect(endpoint_, [self](boost::system::error_code ec) {
        if (!ec) {
            fmt::println("[Client] Connected to server.");
            self->read_header();
        } else {
            fmt::println(stderr,
                         "[Client] Failed to connect: {}",
                         ec.message());
        }
    });
}

void Client::read_header() {
    auto self = shared_from_this();

    auto body_length = std::make_shared<uint32_t>(0);
    auto header_buf  = boost::asio::buffer(body_length.get(), sizeof(uint32_t));
    boost::asio::async_read(
        socket_,
        header_buf,
        [self, body_length](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                *body_length = ntohl(*body_length);
                self->read_body(*body_length);
            }
        });
}

void Client::read_body(uint32_t length) {
    auto self     = shared_from_this();
    auto body_buf = std::make_shared<std::vector<char>>(length);
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(*body_buf),
        [self, body_buf](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                std::string_view sv(body_buf->data(), body_buf->size());
                fmt::println("[Message received]: {}", sv);
                self->read_header();
            }
        });
}

void Client::run_input_thread() {
    std::thread([self = shared_from_this()] {
        std::string line;
        while (std::getline(std::cin, line)) {
            self->send_message(line);
        }
    }).detach();
}

void Client::send_message(const std::string& original) {
    auto self = shared_from_this();
    auto msg  = std::make_shared<std::string>();
    uint32_t len = htonl(original.size());
    msg->append(reinterpret_cast<const char*>(&len), sizeof(len));
    msg->append(original);

    boost::asio::async_write(
        self->socket_,
        boost::asio::buffer(*msg),
        [self, msg](boost::system::error_code, std::size_t) {});
}
