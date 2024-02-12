#include <iostream>
#include "server.h"

using std::cout;
using std::endl;

Server::Server()
{
    m_work.reset(new asio::io_service::work(m_ios));
}

Server::~Server()
{
    Stop();
}

void Server::Start(uint port_num, uint thread_pool_size)
{
    cout << "[Server] starting server" << endl;

    if (thread_pool_size <= 0)
        throw std::runtime_error("No thread pool available for server");

    // create and start acceptor
    m_acceptor.reset(new Acceptor(m_ios, port_num));
    m_acceptor->Start();

    cout << "[Server] starting server with " << thread_pool_size << " threads." << endl;

    // create and start specified number of threads
    for (uint i = 0; i < thread_pool_size; ++i)
    {
        std::unique_ptr<std::thread> th(new std::thread([this]() { m_ios.run(); }));
        m_thread_pool.push_back(std::move(th));
    }

    cout << "[Server] server started" << endl;
}

void Server::Stop()
{
    m_acceptor->Stop();
    m_ios.stop();

    for (auto& th : m_thread_pool)
    {
        if (th->joinable())
            th->join();
    }

    m_thread_pool.clear();
    cout << "[Server] stopping server" << endl;
}
