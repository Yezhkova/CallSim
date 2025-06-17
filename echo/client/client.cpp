#include "client.h"

void Client::start()
{
    auto self = shared_from_this();
    fmt::println("=========Client started=========");
    socket_.async_connect(endpoint_,
                          [self](boost::system::error_code ec)
                          {
                              if (!ec)
                              {
                                  fmt::println("[Client] Connected to server.");
                                  self->read_header();
                              }
                              else
                              {
                                  fmt::println(stderr, "[Client] Failed to connect: {}", ec.message());
                              }
                          });
}

void Client::read_header()
{
    auto self = shared_from_this();
    auto header_buf = std::make_shared<std::array<char, 4>>();

    boost::asio::async_read(socket_, boost::asio::buffer(*header_buf),
                            [self, header_buf](boost::system::error_code ec, std::size_t)
                            {
                                if (!ec)
                                {
                                    uint32_t body_length = 0;
                                    std::memcpy(&body_length, header_buf->data(), 4);
                                    // body_length = ntohl(body_length);
                                    self->read_body(body_length);
                                }
                            });
}

void Client::read_body(std::size_t length)
{
    auto self = shared_from_this();
    auto body_buf = std::make_shared<std::vector<char>>(length);
    boost::asio::async_read(socket_, boost::asio::buffer(*body_buf),
                            [self, body_buf](boost::system::error_code ec, std::size_t)
                            {
                                if (!ec)
                                {
                                    std::string msg(body_buf->begin(), body_buf->end());
                                    fmt::println("[Message received]: {}", msg);
                                    self->read_header();
                                }
                            });
}

void Client::run_input_thread()
{
    std::thread([self = shared_from_this()]
                {
        std::string line;
        while (std::getline(std::cin, line)) {
            auto msg = std::make_shared<std::string>();
            uint32_t len = static_cast<uint32_t>(line.size());
            fmt::println("[Client sending: {} bytes]",len);
            char header[4];
            std::memcpy(header, &len, 4);
            msg->assign(header, header + 4);
            msg->append(line);

            boost::asio::post(self->socket_.get_executor(), [self, msg]() {
                boost::asio::async_write(self->socket_, boost::asio::buffer(*msg),
                    [self, msg](boost::system::error_code, std::size_t) {});
            });
        } })
        .detach();
}
