#ifndef SERVER_H_
#define SERVER_H_

#include <memory>
#include <vector>
#include <thread>
#include <list>
#include <unordered_map>

#include "networking.h"
#include "player.h"
#include "json.hpp"
#include "icommand.h"
#include "connection_manager.h"

class Server
{
public:
    static Server& getInstance() {
        static Server instance;
        return instance;
    }
private:
    Server();
    ~Server();

    // Delete copy and move constructors and assign operators
    // to prevent any copies of your singleton
    Server(Server const&);
    void operator=(Server const&);
public:
    ConnectionManager m_connections;
private:
    asio::io_service m_ios;
    std::unique_ptr<asio::io_service::work> m_work;
    std::vector<std::unique_ptr<std::thread>> m_thread_pool;
    std::atomic<bool> m_isStopped;
    std::unordered_map<server::protocol, std::unique_ptr<tcp_acceptor>> m_acceptors;
    std::unordered_map<server::actions, std::unique_ptr<Icommand>> m_commands;
public:
    void Start(uint thread_pool_size);
    void Stop();
    void process_io_messages();
private:
    void listen(server::protocol protocol);
};

#endif // SERVER_H_