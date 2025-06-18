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
                                    body_length = ntohl(body_length);
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
                                Message msg;
                                if (msg.ParseFromArray(body_buf->data(), body_buf->size()))
                                {
                                    fmt::println("[Message received]: {}", msg.text());
                                    self->read_header();
                                }
                                else
                                {
                                    fmt::println(stderr, "Clientparsing error: {}", ec.what());
                                }
                            });
}

void Client::run_input_thread()
{
    std::thread([self = shared_from_this()]
                {
                    std::string line;
                    while (std::getline(std::cin, line))
                    {
                        Message msg_proto;
                        msg_proto.set_client(fmt::format("{}:{}"
                            , self->socket_.local_endpoint().address().to_string()
                            , self->socket_.local_endpoint().port()));
                        msg_proto.set_text(line);

                        std::string serialized;
                        if (!msg_proto.SerializeToString(&serialized))
                        {
                            fmt::println(stderr, "[Client] Failed to serialize protobuf");
                            continue;
                        }
                        auto msg = std::make_shared<std::string>();
                        uint32_t len = htonl(msg_proto.ByteSizeLong());
                        msg->append(reinterpret_cast<const char*>(&len), sizeof(len));
                        msg->append(serialized);

                        boost::asio::post(self->socket_.get_executor(), [self, msg]()
                                          { boost::asio::async_write(self->socket_, boost::asio::buffer(*msg),
                                                                     [self, msg](boost::system::error_code, std::size_t) {}); });
                    } })
        .detach();
}
