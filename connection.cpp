#include <iostream>
#include "connection.h"

using std::cout;
using std::endl;

Connection::Connection(std::shared_ptr<asio::ip::tcp::socket> sock, std::shared_ptr<std::unordered_map<string, Player *>> players) :
    m_sock(sock),
    m_isOpen(true)
{
    ListenIncomingMessages();

    cout << "[Server] new connection on ip: " << m_sock->remote_endpoint().address().to_string() << endl;
    m_messagesOut.push_back(nlohmann::json{{"code", SERVER_CODES::AUTHENTICATE_SUCCESS }});
}

Connection::~Connection()
{
    Close();
}

void Connection::Close()
{
//    cout << "[Server] closed connection on ip: " << m_sock->remote_endpoint().address().to_string() << endl;
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
    asio::async_read_until(*m_sock, m_request, '\n',
    [this](const boost::system::error_code & ec, std::size_t bytes_transferred) {
        if (ec.value() != 0) {
            cout << "[Server] failed to read request: " << ec.message() << endl;
            return;
        }

        std::string message;
        std::istream is(&m_request);
        std::getline(is, message);

        try {
            nlohmann::json j = nlohmann::json::parse(message);
            m_messagesIn.push_back(j);
        }
        catch (nlohmann::json::parse_error & e) {
            cout << "[Server] failed to parse message: " << e.what() << endl;
        }

        m_request.consume(m_request.size());
        ListenIncomingMessages();
    });
}

bool Connection::IsOpen() const {
    return m_sock && m_isOpen;
}

void Connection::DispatchMessages() {
    // first process outgoing messages
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

    // then process incoming messages
    if (!m_messagesIn.empty()) {
        for (auto & message : m_messagesIn) {
            uint code = message["code"];

            switch (code) {
                case SERVER_CODES::PING:
                    m_messagesOut.push_back(nlohmann::json{{"code", SERVER_CODES::PING_RESPONSE, "message", "pong" }});
                    break;
                default:
                    break;
            }
        }

        m_messagesIn.clear();
    }
}
