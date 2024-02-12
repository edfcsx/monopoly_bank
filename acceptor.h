#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include "asio_common.h"
#include "types.h"

class Acceptor
{
public:
    Acceptor(asio::io_service & ios, uint port_num);
    ~Acceptor();

    void Start();
    void Stop();

private:
    asio::io_service & m_ios;
    asio::ip::tcp::acceptor m_acceptor;
    std::atomic<bool> m_isStopped;

    void InitAcceptor();
    void OnAccept(const boost::system::error_code & ec, std::shared_ptr< asio::ip::tcp::socket> sock);
};

#endif // _ACCEPTOR_H_