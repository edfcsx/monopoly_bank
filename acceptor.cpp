#include <iostream>
#include "acceptor.h"
#include "service.h"

using std::cout;
using std::endl;

Acceptor::Acceptor(asio::io_service & ios, uint port_num) :
        m_ios(ios),
        m_acceptor(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num)),
        m_isStopped(false)
{
    cout << "[Server] acceptor messages on port: " << port_num << endl;
}

Acceptor::~Acceptor()
{
    Stop();
}

void Acceptor::Start()
{
    m_acceptor.listen();
    InitAcceptor();
}

void Acceptor::Stop()
{
    cout << "[Server] stopping acceptor" << endl;
    m_isStopped.store(true);

    if (m_acceptor.is_open())
        m_acceptor.close();
}

void Acceptor::InitAcceptor()
{
    std::shared_ptr<asio::ip::tcp::socket> sock(new asio::ip::tcp::socket(m_ios));

    m_acceptor.async_accept(*sock.get(),
    [this, sock](const boost::system::error_code & ec) {
        OnAccept(ec, sock);
    });
}

void Acceptor::OnAccept(const boost::system::error_code & ec, std::shared_ptr<asio::ip::tcp::socket> sock)
{
    if (!ec)
        (new Service(sock))->StartHandling();
    else
        cout << "[Server] failed to accept connection: " << ec.message() << endl;

    // Init next async accept operation if acceptor has not been stopped yet
    if (!m_isStopped.load())
        InitAcceptor();
    else
        Stop();
}
