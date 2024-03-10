#ifndef CONNECTION_MANAGER_H_
#define CONNECTION_MANAGER_H_

#include <memory>
#include <mutex>
#include <iostream>
#include "networking.h"
#include "websocket_handshake.h"
#include "player.h"
#include "connection.h"

class ConnectionManager
{
public:
    ConnectionManager();
    ~ConnectionManager();

private:
    std::unordered_map<std::string, std::unique_ptr<Connection>> m_connections;
    std::mutex m_connections_lock;

    std::shared_ptr<std::unordered_map<std::string, Player *>> m_players;
    std::mutex m_playersMutex;
public:
    void accept_connection(tcp_socket socket, ConnProtocol p);
private:
    static void RejectConnection(ptr_socket sock, SERVER_CODES status);
};

#endif // CONNECTION_MANAGER_H_
