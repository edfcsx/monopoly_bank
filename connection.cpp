#include <iostream>
#include "connection.h"

using std::cout;
using std::endl;

Connection::Connection(std::shared_ptr<asio::ip::tcp::socket> sock) :
    m_sock(sock),
    m_isOpen(true)
{
    IOListener();

    cout << "[Server] new connection on ip: " << m_sock->remote_endpoint().address().to_string() << endl;
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

void Connection::IOListener()
{
    asio::async_read_until(*m_sock, m_request, '\n',
    [this](const boost::system::error_code & ec, std::size_t bytes_transferred) {
        if (ec.value() != 0) {
            cout << "[Server] failed to read request: " << ec.message() << endl;
            Close();
            return;
        }

        m_response = std::string{"responding request....\n"};
        m_request.consume(bytes_transferred);

        // initiate asynchronous write operation and call OnResponseSent upon completion
        asio::async_write(*m_sock, asio::buffer(m_response),
    [this](const boost::system::error_code & ec, std::size_t bytes_transferred) {
            if (ec.value() != 0)
                cout << "[Server] failed to write response: " << ec.message() << endl;

            IOListener();
        });
    });
}

bool Connection::IsOpen() const {
    return m_sock && m_isOpen;
}
