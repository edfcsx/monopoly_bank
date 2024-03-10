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
    ListenIncomingMessages();

    cout << "[Server] new connection on ip: " << m_sock->remote_endpoint().address().to_string() << endl;
    m_messagesOut.push_back(nlohmann::json{{ "code", SERVER_CODES::AUTHENTICATE_SUCCESS }});
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

void Connection::ListenIncomingMessages()
{
    asio::async_read(*m_sock, m_request, asio::transfer_exactly(2),
     [this](const boost::system::error_code & ec, std::size_t bytes_transferred) {
        if (ec.value() != 0) {
            cout << "[Server] failed to read request: " << ec.message() << endl;
            return;
        }

        std::istream stream(&m_request);
        std::array<unsigned char, 2> first_bytes{};
        stream.read((char *)&first_bytes[0], 2);

        unsigned char opcode = first_bytes[0];

        if (opcode < 128) {
            cout << "[Server] close connection if unmasked message from client";
            // @todo: implements close connection handshake
            m_sock->close();
        }

        std::size_t length = first_bytes[1] & 0x7F;
        m_request.consume(bytes_transferred);

        // 4 bytes to read mask
        asio::async_read(*m_sock, m_request, asio::transfer_exactly(4 + length),
         [this, length](const boost::system::error_code & ec, std::size_t bytes_transferred) {
            // read mask
            std::istream stream(&m_request);
            std::array<unsigned char, 4> mask{};
            stream.read((char *)&mask[0], 4);

             std::vector<unsigned char> data(length);
             stream.read((char *)data.data(), length);

             for (std::size_t i = 0; i < length; ++i) {
                 data[i] ^= mask[i % 4];
             }

             std::string message(data.begin(), data.end());
             cout << "[Server] received message: " << message << endl;

             ListenIncomingMessages();
        });
    });
}

bool Connection::IsOpen() const {
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
