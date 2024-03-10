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
    std::mutex m_playersMutex;
    std::mutex m_messageInMutex;
private:
    asio::io_service m_ios;
    std::unique_ptr<asio::io_service::work> m_work;
    std::vector<std::unique_ptr<std::thread>> m_thread_pool;
    std::unordered_map<ConnProtocol, std::unique_ptr<tcp_acceptor>> m_acceptors;
    std::atomic<bool> m_isStopped;
    ConnectionManager m_connections;

    std::list<NetworkingMessage> m_messagesIn;
    std::unordered_map<SERVER_CODES, std::unique_ptr<Icommand>> m_commandsMap;
public:
    void Start(uint thread_pool_size);
    void Stop();
//    std::shared_ptr<std::unordered_map<std::string, Player *>> GetPlayers();
//    void PushMessage(NetworkingMessage message);
//    void ProcessMessages();
//    bool CheckPlayerExistsAndConnected(const std::string & username);
//    bool CheckPlayerExists(const std::string & username);
//    bool CheckPlayerConnected(const std::string & username);
private:
    void listen(ConnProtocol protocol);
//
//    static void RejectConnection(std::shared_ptr<asio::ip::tcp::socket> sock, SERVER_CODES code);
//    void AuthenticatePlayerHandler(std::shared_ptr<asio::ip::tcp::socket> sock, nlohmann::json player_data);
};

#endif // SERVER_H_