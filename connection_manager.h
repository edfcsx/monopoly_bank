#ifndef CONNECTION_MANAGER_H_
#define CONNECTION_MANAGER_H_

#include <memory>
#include <mutex>
#include <iostream>
#include "networking.h"
#include "websocket_handshake.h"
#include "player.h"
#include "connection.h"
#include "uuid.h"

class ConnectionManager
{
public:
    ConnectionManager();
    ~ConnectionManager();

private:
    std::shared_ptr<std::unordered_map<std::string, Connection *>> m_connections;
    std::mutex m_connectionsMutex;

    std::shared_ptr<std::unordered_map<std::string, Player *>> m_players;
    std::mutex m_playersMutex;
public:
    void AcceptWebsocketConnection(ptr_socket sock);
    void AcceptRawConnection(ptr_socket sock);

private:
    static void RejectConnection(ptr_socket sock, SERVER_CODES status);
};

#endif // CONNECTION_MANAGER_H_
