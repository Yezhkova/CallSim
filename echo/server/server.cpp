#include <fmt/core.h>

#include "../proto/message.pb.h"
#include <boost/asio.hpp>

using Tcp = boost::asio::ip::tcp;

class ServerSession : public std::enable_shared_from_this<ServerSession> {
   private:
    Tcp::socket       socket_;
    std::vector<char> body_buf_;
    std::string       remote_address_;
    uint16_t          remote_port_;

   public:
    explicit ServerSession(Tcp::socket&& socket) : socket_(std::move(socket)) {
        remote_address_ = socket_.remote_endpoint().address().to_string();
        remote_port_    = socket_.remote_endpoint().port();
    }

    void start() { read_header(); }

   private:
    void read_header() {
        auto self        = shared_from_this();
        auto body_length = std::make_shared<uint32_t>(0);
        auto header_buf =
            boost::asio::buffer(body_length.get(), sizeof(uint32_t));

        boost::asio::async_read(
            socket_,
            header_buf,
            [self, body_length](boost::system::error_code ec, std::size_t) {
                if (ec == boost::asio::error::eof ||
                    ec == boost::asio::error::connection_reset) {
                    fmt::println("Client {}:{} disconnected",
                                 self->remote_address_,
                                 self->remote_port_);
                    return;
                }
                if (ec) {
                    fmt::println(stderr,
                                 "Read header error ({}:{}): {}",
                                 self->remote_address_,
                                 self->remote_port_,
                                 ec.message());
                    return;
                }
                *body_length = ntohl(*body_length);
                self->read_body(body_length);
            });
    }

    void read_body(std::shared_ptr<uint32_t> length) {
        auto self = shared_from_this();
        body_buf_.resize(*length);

        boost::asio::async_read(
            socket_,
            boost::asio::buffer(body_buf_),
            [self](boost::system::error_code ec, std::size_t) {
                if (ec == boost::asio::error::eof ||
                    ec == boost::asio::error::connection_reset) {
                    auto ep = self->socket_.remote_endpoint();
                    fmt::println("Client {}:{} disconnected",
                                 ep.address().to_string(),
                                 ep.port());
                    return;
                }
                if (ec) {
                    fmt::println(stderr, "[Server] Error: {}", ec.what());
                    return;
                }

                Message msg;
                if (msg.ParseFromArray(self->body_buf_.data(),
                                       self->body_buf_.size())) {
                    fmt::println("[{}]: {}", msg.client(), msg.text());
                    Message reply;
                    reply.set_text("Re: " + msg.text());
                    std::string serialized;
                    if (!reply.SerializeToString(&serialized)) {
                        fmt::println(stderr,
                                     "[Server] Failed to serialize response");
                        return;
                    }
                    self->send_response(serialized);
                } else {
                    fmt::println(stderr, "[Server] Failed to parse message");
                }
            });
    }

    void send_response(const std::string& response) {
        auto     self = shared_from_this();
        auto     msg  = std::make_shared<std::string>();
        uint32_t len  = htonl(response.size());
        msg->append(reinterpret_cast<const char*>(&len), sizeof(len));
        msg->append(response);
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(*msg),
            [self](boost::system::error_code ec, std::size_t) {
                if (!ec)
                    self->read_header();
                else
                    fmt::println(stderr,
                                 "[Server] Failed to write: {}",
                                 ec.message());
            });
    }
};

void do_accept(std::shared_ptr<Tcp::acceptor> acceptor);

void handle_accept(std::shared_ptr<Tcp::acceptor> acceptor,
                   boost::system::error_code      ec,
                   Tcp::socket                    socket) {
    if (!ec) {
        auto remote = socket.remote_endpoint();
        fmt::println("Accepted connection from {}:{}",
                     remote.address().to_string(),
                     remote.port());
        std::make_shared<ServerSession>(std::move(socket))->start();
    } else {
        fmt::println(stderr, "Accept error: {}", ec.message());
    }
    do_accept(acceptor);
}

void do_accept(std::shared_ptr<Tcp::acceptor> acceptor) {
    acceptor->async_accept(
        [acceptor](boost::system::error_code ec, Tcp::socket socket) {
            handle_accept(acceptor, ec, std::move(socket));
        });
}

void run_server(boost::asio::io_context& io, uint16_t port) {
    auto acceptor =
        std::make_shared<Tcp::acceptor>(io, Tcp::endpoint(Tcp::v4(), port));
    fmt::println("=========Server started=========");
    do_accept(acceptor);
}
