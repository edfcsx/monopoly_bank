#include <iostream>
#include <string>
#include "connection.h"

using std::cout;
using std::endl;
using std::string;

Connection::Connection(tcp_socket socket, ConnProtocol p) :
    m_sock(std::move(socket)),
    m_isOpen(true),
    m_protocol(p)
{
    cout << "[Server] new connection on ip: " << m_sock->remote_endpoint().address().to_string() << endl;
    m_messagesOut.push_back(nlohmann::json{{ "code", SERVER_CODES::AUTHENTICATE_SUCCESS }});

    if (m_protocol == ConnProtocol::RAW) {
        listen_raw_messages();
    } else if (m_protocol == ConnProtocol::WEBSOCKET) {
        listen_websocket_messages();
    }
}

Connection::~Connection()
{
    Close();
}

void Connection::Close()
{
    m_isOpen = false;

    if (m_sock->is_open())
    {
        boost::system::error_code ec;
        m_sock->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        m_sock->close(ec);
    }
}

void Connection::listen_websocket_messages()
{
    m_message.reset();

    asio::async_read(*m_sock, m_message.buffer, asio::transfer_exactly(2),
     [this](const boost::system::error_code & ec, std::size_t bytes_transferred) {
        if (ec.value() != 0) {
            cout << "[Server] failed to read request: " << ec.message() << endl;
            return;
        }

        std::istream stream(&m_message.buffer);
        std::array<unsigned char, 2> first_bytes{};
        stream.read((char *)&first_bytes[0], 2);

        if (first_bytes[0] < 128) {
            cout << "[Server] close connection if unmasked message from client";
            // @todo: implements close connection handshake
            // send_close(1002, "unmasked message from client");
            m_sock->close();
            return;
        }

        m_message.type = first_bytes[0] & 0x0F;
        m_message.unique = (first_bytes[0] & 0x80) == 0x80;

        if (m_message.type != connection::opcode::TEXT) {
            cout << "[Server] close connection if message type is not text";
            // @todo: implements close connection handshake
            // send_close(1003, "message type is not text");
            return;
        }

        m_message.length = first_bytes[1] & 0x7F;
        m_message.clear_buffer();

        if (m_message.length == 126 || m_message.length == 127) {
            m_message.additional_bytes = m_message.length == 126 ? 2 : 8;

            asio::async_read(*m_sock, m_message.buffer, asio::transfer_exactly(m_message.additional_bytes),
             [this](const boost::system::error_code & ec, std::size_t bytes_transferred) {
                if (ec.value() != 0) {
                    cout << "[Server] failed to read request: " << ec.message() << endl;
                    return;
                }

                std::istream stream(&m_message.buffer);
                std::vector<unsigned char> bytes(m_message.additional_bytes);
                stream.read((char *)bytes.data(), m_message.additional_bytes);

                if (m_message.additional_bytes == 2) {
                    m_message.length = (bytes[0] << 8) + bytes[1];
                } else {
                    m_message.length = 0;

                    for (std::size_t i = 0; i < 8; ++i) {
                        m_message.length += bytes[i] << (8 * (7 - i));
                    }
                }

                read_websocket_message_content();
            });
        } else {
            read_websocket_message_content();
        }
    });
}

void Connection::read_websocket_message_content()
{
    m_message.clear_buffer();

    if (m_message.length > MAX_MESSAGE_SIZE) {
        cout << "[Server] close connection if message length is too big";
        // @TODO: make close connection
        return;
    }

    asio::async_read(*m_sock, m_message.buffer, asio::transfer_exactly(MSG_MASK_SIZE + m_message.length),
     [this](const boost::system::error_code & ec, std::size_t bytes_transferred) {
         if (ec.value() != 0) {
             cout << "[Server] failed to read request: " << ec.message() << endl;
             return;
         }

         std::istream stream(&m_message.buffer);
         std::array<unsigned char, MSG_MASK_SIZE> mask{};
         stream.read((char *) &mask[0], MSG_MASK_SIZE);

         std::vector<unsigned char> data(m_message.length);
         stream.read((char *) data.data(), m_message.length);

         for (std::size_t i = 0; i < m_message.length; ++i) {
             data[i] ^= mask[i % MSG_MASK_SIZE];
         }

         std::string message(data.begin(), data.end());
         cout << "[Server] received message: " << message << endl;
         send_data("Hello, client!", connection::opcode::TEXT);
         listen_websocket_messages();
     });
}

void Connection::listen_raw_messages() {
    asio::async_read_until(*m_sock, m_request, "\n",
     [this](const boost::system::error_code & ec, std::size_t bytes_transferred) {
        if (ec.value() != 0) {
            cout << "[Server] failed to read request: " << ec.message() << endl;
            return;
        }

        std::istream stream(&m_request);
        std::string message;
        std::getline(stream, message);

        cout << "[Server] received message: " << message << endl;

        try {
            auto j = nlohmann::json::parse(message);

            if (j.contains("code")) {

            }
        } catch (nlohmann::json::parse_error & e) {
            cout << "[Server] failed to parse message: " << e.what() << endl;
        }

        m_request.consume(m_request.size());
        listen_raw_messages();
    });
}

bool Connection::is_open() const {
    return m_sock && m_isOpen;
}

void Connection::DispatchMessages() {
    if (!m_messagesOut.empty()) {
        for (auto & message : m_messagesOut) {
            auto * msg = new string(message.dump() + "\n");

            asio::async_write(*m_sock, asio::buffer(*msg),
              [msg](const boost::system::error_code & ec, std::size_t bytes_transferred) {
                  if (ec.value() != 0) {
                      cout << "[Server] failed to write message: " << ec.message() << endl;
                  }

                  delete msg;
              });
        }

        m_messagesOut.clear();
    }
}

void Connection::Send(nlohmann::json message) {
    m_messagesOut.push_back(message);
}

void Connection::send_data(const std::string & data, connection::opcode c) {
    std::vector<unsigned char> frame;

    // first byte: FIN + opcode
    frame.push_back(0x80 | c);  // FIN = 1 (mensagem final)

    // second byte: MASK + length
    if (data.size() <= 125) {
        frame.push_back(data.size()); // MASK = 0 (mensagem não mascarada), length <= 125
    } else if (data.size() <= 65535) {
        frame.push_back(126); // MASK = 0, length = 126
        frame.push_back((data.size() >> 8) & 0xFF); // coloca os 8 bits mais significativos
        frame.push_back(data.size() & 0xFF);    // coloca os 8 bits menos significativos
    } else {
        frame.push_back(127); // MASK = 0, length => 65535
        for (int i = 7; i >= 0; --i) {
            frame.push_back((data.size() >> (8 * i)) & 0xFF);  // coloca os 8 bits mais significativos
        }
    }

    // append the data
    frame.insert(frame.end(), data.begin(), data.end());

    // send the frame
    asio::async_write(*m_sock, asio::buffer(frame),
    [frame](const boost::system::error_code & ec, std::size_t bytes_transferred) {
        if (ec.value() != 0) {
            cout << "[Server] failed to write message: " << ec.message() << endl;
        }

        std::cout << "[Server] sent message: " << bytes_transferred << " bytes" << std::endl;
    });
}