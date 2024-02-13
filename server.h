#ifndef SERVER_H_
#define SERVER_H_

#include <memory>
#include <vector>
#include <thread>
#include <list>
#include <unordered_map>

#include "types.h"
#include "networking.h"
#include "connection.h"
#include "player.h"

class Server
{
public:
    Server();
    ~Server();

    void Start(uint port_num, uint thread_pool_size);
    void Stop();

    void SetConnectionsLimit(uint limit);
    void DumpClosedConnections();
private:
    asio::io_service m_ios;
    std::unique_ptr<asio::io_service::work> m_work;
    std::vector<std::unique_ptr<std::thread>> m_thread_pool;
    asio::ip::tcp::acceptor m_acceptor;
    std::atomic<bool> m_isStopped;

    uint m_connections_limit;
    std::list<Connection *> m_connections;
    std::unordered_map<std::string, Player *> m_players;
private:
    void InitAcceptConnections();
    static void RejectConnection(std::shared_ptr<asio::ip::tcp::socket> sock, SERVER_CODES code);
    void RequestAuthorization(std::shared_ptr<asio::ip::tcp::socket> sock);
};

#endif // SERVER_H_