#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <functional>

using boost::asio::ip::tcp;

class ServerSession : public std::enable_shared_from_this<ServerSession>
{
public:
    explicit ServerSession(tcp::socket socket)
        : socket_(std::move(socket)) {}

    void start() { do_read(); }

private:
    void do_read()
    {
        auto self = shared_from_this();
        buf_ = std::make_shared<boost::asio::streambuf>();
        boost::asio::async_read_until(socket_, *buf_, "\n",
                                      [this, self](boost::system::error_code ec, std::size_t)
                                      {
                                          if (!ec)
                                          {
                                              std::istream is(buf_.get());
                                              std::string line;
                                              std::getline(is, line);
                                              std::cout << "[Server received]: " << line << std::endl;

                                              line = "Re: " + line + "\n";
                                              auto msg = std::make_shared<std::string>(line);
                                              boost::asio::async_write(socket_, boost::asio::buffer(*msg),
                                                                       [this, self, msg](boost::system::error_code, std::size_t)
                                                                       {
                                                                           do_read();
                                                                       });
                                          }
                                      });
    }

    tcp::socket socket_;
    std::shared_ptr<boost::asio::streambuf> buf_;
};

void run_server(boost::asio::io_context &io, unsigned short port)
{
    auto acceptor = std::make_shared<tcp::acceptor>(io, tcp::endpoint(tcp::v4(), port));

    auto do_accept = std::make_shared<std::function<void()>>();
    *do_accept = [acceptor, do_accept]()
    {
        acceptor->async_accept(
            [acceptor, do_accept](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    std::cout << "Accepted new connection\n";
                    std::make_shared<ServerSession>(std::move(socket))->start();
                }
                else
                {
                    std::cerr << "Accept error: " << ec.message() << "\n";
                }
                (*do_accept)(); // loop
            });
    };

    (*do_accept)(); // start accepting
}
