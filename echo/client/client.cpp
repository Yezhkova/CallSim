#include "client.h"

void Client::start()
{
    auto self = shared_from_this();
    socket_.async_connect(endpoint_,
                          [self](boost::system::error_code ec)
                          {
                              if (!ec)
                              {
                                  self->do_read();
                              }
                          });
}

void Client::run_input_thread()
{
    std::thread([self = shared_from_this()]
                {
            std::string line;
            while (std::getline(std::cin, line)) {
                boost::asio::post(self->socket_.get_executor(), [self, line]() {
                    auto msg = std::make_shared<std::string>(line + "\n");
                    boost::asio::async_write(self->socket_, boost::asio::buffer(*msg),
                        [self, msg](boost::system::error_code, std::size_t) {});
                });
            } })
        .detach();
}

void Client::do_read()
{
    auto self = shared_from_this();
    auto buf = std::make_shared<boost::asio::streambuf>();
    boost::asio::async_read_until(socket_, *buf, "\n",
                                  [self, buf](boost::system::error_code ec, std::size_t)
                                  {
                                      if (!ec)
                                      {
                                          std::istream is(buf.get());
                                          std::string line;
                                          std::getline(is, line);
                                          std::cout << "[Client received]: " << line << std::endl;
                                          self->do_read();
                                      }
                                  });
}
