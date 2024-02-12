#ifndef _SERVER_H_
#define _SERVER_H_

#include <memory>
#include <vector>
#include <thread>

#include "types.h"
#include "asio_common.h"
#include "acceptor.h"

class Server
{
public:
    Server();
    ~Server();

    void Start(uint port_num, uint thread_pool_size);
    void Stop();

private:
    asio::io_service m_ios;
    std::unique_ptr<asio::io_service::work> m_work;
    std::unique_ptr<Acceptor> m_acceptor;
    std::vector<std::unique_ptr<std::thread>> m_thread_pool;
};

#endif // _SERVER_H_