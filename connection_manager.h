#ifndef CONNECTION_MANAGER_H_
#define CONNECTION_MANAGER_H_

#include <memory>
#include "networking.h"
#include "websocket_handshake.h"

class ConnectionManager
{
public:
    static ConnectionManager& GetInstance() {
        static ConnectionManager instance;
        return instance;
    }

private:
    ConnectionManager() = default;
    ~ConnectionManager() = default;

    // Delete copy and move constructors and assign operators
    // to prevent any copies of your singleton
    ConnectionManager(ConnectionManager const&);
    void operator=(ConnectionManager const&);
public:
    void AcceptWebsocketConnection(ptr_socket sock);
};

#endif // CONNECTION_MANAGER_H_