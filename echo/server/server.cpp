#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include <array>
#include <vector>
#include <fmt/core.h>
#include <regex>

using boost::asio::ip::tcp;

class ServerSession : public std::enable_shared_from_this<ServerSession>
{
public:
    ServerSession(tcp::socket socket)
        : socket_(std::move(socket))
    {
        remote_address_ = socket_.remote_endpoint().address().to_string();
        remote_port_ = socket_.remote_endpoint().port();
    }

    void start()
    {
        read_header();
    }

private:
    void read_header()
    {
        auto self = shared_from_this();
        header_buf_ = std::make_shared<std::array<char, 4>>();

        boost::asio::async_read(socket_, boost::asio::buffer(*header_buf_),
                                [self](boost::system::error_code ec, std::size_t)
                                {
                                    if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset)
                                    {
                                        fmt::println("Client {}:{} disconnected", self->remote_address_, self->remote_port_);
                                        return;
                                    }
                                    if (ec)
                                    {
                                        fmt::println(stderr, "Read header error ({}:{}): {}", self->remote_address_, self->remote_port_, ec.message());
                                        return;
                                    }

                                    uint32_t body_length = 0;
                                    std::memcpy(&body_length, self->header_buf_->data(), 4);
                                    fmt::println("[{}:{}] {} bytes", self->remote_address_, self->remote_port_, body_length);
                                    self->read_body(body_length);
                                });
    }

    void read_body(std::size_t length)
    {
        auto self = shared_from_this();
        body_buf_ = std::make_shared<std::vector<char>>(length);

        boost::asio::async_read(socket_, boost::asio::buffer(*body_buf_),
                                [self](boost::system::error_code ec, std::size_t)
                                {
                                    if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset)
                                    {
                                        fmt::println("Client {}:{} disconnected", self->remote_address_, self->remote_port_);
                                        return;
                                    }
                                    if (ec)
                                    {
                                        fmt::println(stderr, "Read body error ({}:{}): {}", self->remote_address_, self->remote_port_, ec.message());
                                        return;
                                    }

                                    std::string message(self->body_buf_->begin(), self->body_buf_->end());
                                    fmt::println("[Message received from {}:{}]: {}", self->remote_address_, self->remote_port_, message);

                                    std::string reply = "Re: " + message;
                                    self->send_response(reply);
                                });
    }

    void send_response(const std::string &message)
    {
        auto self = shared_from_this();
        auto msg = std::make_shared<std::string>();

        uint32_t len = static_cast<uint32_t>(message.size());
        char header[4];
        std::memcpy(header, &len, 4);
        msg->assign(header, header + 4);
        msg->append(message);

        boost::asio::async_write(socket_, boost::asio::buffer(*msg),
                                 [self, msg](boost::system::error_code ec, std::size_t)
                                 {
                                     if (ec)
                                     {
                                         fmt::println(stderr, "Write error ({}:{}): {}", self->remote_address_, self->remote_port_, ec.message());
                                         return;
                                     }
                                     self->read_header();
                                 });
    }

    tcp::socket socket_;
    std::shared_ptr<std::array<char, 4>> header_buf_;
    std::shared_ptr<std::vector<char>> body_buf_;
    std::string remote_address_;
    unsigned short remote_port_;
};

void run_server(boost::asio::io_context &io, unsigned short port)
{
    auto acceptor = std::make_shared<tcp::acceptor>(io, tcp::endpoint(tcp::v4(), port));
    fmt::println("=========Server started=========");
    auto do_accept = std::make_shared<std::function<void()>>();
    *do_accept = [acceptor, do_accept]()
    {
        acceptor->async_accept(
            [acceptor, do_accept](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    auto remote = socket.remote_endpoint();
                    fmt::println("Accepted connection from {}:{}", remote.address().to_string(), remote.port());
                    std::make_shared<ServerSession>(std::move(socket))->start();
                }
                else
                {
                    fmt::println(stderr, "Accept error: {}", ec.message());
                }
                (*do_accept)();
            });
    };

    (*do_accept)();
}
