#include <boost/asio.hpp>
#include <fmt/core.h>
#include "../proto/message.pb.h"

using boost::asio::ip::tcp;

class ServerSession : public std::enable_shared_from_this<ServerSession>
{
private:
    tcp::socket socket_;
    std::shared_ptr<std::array<char, 4>> header_buf_;
    std::shared_ptr<std::vector<char>> body_buf_;
    std::string remote_address_;
    unsigned short remote_port_;

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
                                    body_length = ntohl(body_length);
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
                                        auto ep = self->socket_.remote_endpoint();
                                        fmt::print("Client {}:{} disconnected\n", ep.address().to_string(), ep.port());
                                        return;
                                    }
                                    if (ec)
                                    {
                                        fmt::println(stderr, "[Server] Error: {}", ec.what());
                                        return;
                                    }

                                    Message msg;
                                    if (msg.ParseFromArray(self->body_buf_->data(), self->body_buf_->size()))
                                    {
                                        fmt::println("[{}]: {}", msg.client(), msg.text());
                                        Message reply;
                                        reply.set_text("Re: " + msg.text());
                                        auto serialized = std::make_shared<std::string>();
                                        if (!reply.SerializeToString(serialized.get()))
                                        {
                                            fmt::println(stderr, "[Server] Failed to serialize response");
                                            return;
                                        }
                                        self->send_response(serialized);
                                    }
                                    else
                                    {
                                        fmt::println(stderr, "[Server] Failed to parse message");
                                    }
                                });
    }

    void send_response(std::shared_ptr<std::string> message)
    {
        auto self = shared_from_this();
        auto msg = std::make_shared<std::string>();
        uint32_t sth = htonl(message->size());
        msg->append(reinterpret_cast<char *>(&sth), sizeof(sth));
        msg->append(*message);
        boost::asio::async_write(socket_, boost::asio::buffer(*msg),
                                 [self](boost::system::error_code ec, std::size_t)
                                 {
                                     if (!ec)
                                         self->read_header();
                                     else
                                         fmt::println(stderr, "[Server] Failed to write: {}", ec.message());
                                 });
    }
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
