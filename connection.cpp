#include <iostream>
#include <string>
#include <utility>
#include "connection.h"

using std::cout;
using std::endl;
using std::string;

Connection::Connection(tcp_socket socket, server::protocol p) :
    m_sock(std::move(socket)),
    m_isOpen(true),
    m_protocol(p)
{
    cout << "[Server] new connection on ip: " << m_sock->remote_endpoint().address().to_string() << endl;

    if (m_protocol == server::protocol::raw) {
        listen_raw_messages();
    } else if (m_protocol == server::protocol::websocket) {
        listen_websocket_messages();
    }

    m_ip = m_sock->remote_endpoint().address().to_string();
}

Connection::~Connection()
{
    close_connection();
}

void Connection::close_connection()
{
    m_isOpen = false;

    if (m_sock->is_open())
    {
        boost::system::error_code ec;

        // cancel async operations
        try {
            m_sock->cancel(ec);
        } catch (std::exception & e) {
            cout << "[Server] failed to cancel async operations: " << e.what() << endl;
        }

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
            close_websocket(connection::status::PROTOCOL_ERROR, "unmasked message from client");
            return;
        }

        m_message.type = first_bytes[0] & 0x0F;
        m_message.unique = (first_bytes[0] & 0x80) == 0x80;

        if (m_message.type != connection::opcode::TEXT) {
            close_websocket(connection::status::UNSUPPORTED_DATA, "message type is not text");
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
        close_websocket(connection::status::MESSAGE_TOO_BIG, "message length is too big");
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

         push_in_message(std::string(data.begin(), data.end()));
         listen_websocket_messages();
     });
}

void Connection::listen_raw_messages() {
    m_message.clear_buffer();

    asio::async_read_until(*m_sock, m_message.buffer, "\n",
     [this](const boost::system::error_code & ec, std::size_t bytes_transferred) {
        if (ec.value() != 0) {
            cout << "[Server] failed to read request: " << ec.message() << endl;
            return;
        }

        m_message.type = connection::opcode::RAW;
        m_message.length = bytes_transferred;

        std::istream stream(&m_message.buffer);
        std::string data;
        std::getline(stream, data);

         push_in_message(data);
        listen_raw_messages();
    });
}

bool Connection::is_open() const {
    return m_sock && m_isOpen;
}

void Connection::send_data(const std::string & data, connection::opcode c, connection::status s, connection::success_send_callback on_success) {
    m_send_frame.clear();

    // first byte: FIN + opcode
    m_send_frame.push_back(0x80 | c);  // FIN = 1 (mensagem final)

    // second byte: MASK + length
    if (data.size() <= 125) {
        m_send_frame.push_back(data.size()); // MASK = 0 (mensagem não mascarada), length <= 125
    } else if (data.size() <= 65535) {
        m_send_frame.push_back(126); // MASK = 0, length = 126
        m_send_frame.push_back((data.size() >> 8) & 0xFF); // coloca os 8 bits mais significativos
        m_send_frame.push_back(data.size() & 0xFF);    // coloca os 8 bits menos significativos
    } else {
        m_send_frame.push_back(127); // MASK = 0, length => 65535
        for (int i = 7; i >= 0; --i) {
            m_send_frame.push_back((data.size() >> (8 * i)) & 0xFF);  // coloca os 8 bits mais significativos
        }
    }

    if (s != connection::status::DEF_STATUS) {
        m_send_frame.push_back((s >> 8) & 0xFF);
        m_send_frame.push_back(s & 0xFF);
    }

    // append the data
    m_send_frame.insert(m_send_frame.end(), data.begin(), data.end());

    // send the m_send_frame
    asio::async_write(*m_sock, asio::buffer(m_send_frame),
    [on_success, data](const boost::system::error_code & ec, std::size_t bytes_transferred) {
        if (ec.value() != 0) {
            cout << "[Server] failed to write message: " << ec.message() << endl;
            return;
        }

        if (on_success != nullptr)
            on_success();

        std::cout << "[Server] sent message: " << data << ", " << bytes_transferred << " bytes" << std::endl;
    });
}

void Connection::send_data(const std::string & data) {
    asio::async_write(*m_sock, asio::buffer(data),
    [data](const boost::system::error_code & ec, std::size_t bytes_transferred) {
        if (ec.value() != 0) {
            cout << "[Server] failed to write message: " << ec.message() << endl;
            return;
        }

        std::cout << "[Server] sent message: " << bytes_transferred << " bytes" << std::endl;
    });
}

void Connection::close_websocket(connection::status s, const std::string & reason)
{
    send_data(reason, connection::opcode::CLOSE, s, [this]() {
        close_connection();
    });
}

void Connection::set_playing(bool p)
{
    m_playing = p;
}

void Connection::push_in_message(const std::string & data)
{
    std::lock_guard<std::mutex> lock(m_messages_in_lock);

    try {
        auto json = nlohmann::json::parse(data);

        if (json.contains("code")) {
            json.push_back({ "ip", m_ip });
            json.push_back({ "is_player", m_playing });
            m_messages_in.push_back(json);
        }
    } catch (nlohmann::json::parse_error & e) {
        cout << "[Server] failed to parse message: " << e.what() << endl;
    }
}

void Connection::push_out_message(nlohmann::json message)
{
    std::lock_guard<std::mutex> lock(m_messages_out_lock);
    m_messages_out.push_back(message);
}

void Connection::send_out_messages() {
    std::lock_guard<std::mutex> lock(m_messages_out_lock);

    for (auto & message : m_messages_out) {
        if (m_protocol == server::protocol::websocket)
            send_data(message.dump(), connection::opcode::TEXT);
        else if (m_protocol == server::protocol::raw)
            send_data(message.dump() + "\n");
    }

    m_messages_out.clear();
}

std::vector<nlohmann::json> Connection::get_in_messages() {
    std::lock_guard<std::mutex> lock(m_messages_in_lock);
    return m_messages_in;
}

void Connection::clear_in_messages() {
    std::lock_guard<std::mutex> lock(m_messages_in_lock);
    m_messages_in.clear();
}