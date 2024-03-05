#include "connection_manager.h"

void ConnectionManager::AcceptWebsocketConnection(ptr_socket sock)
{
    new WebSocketHandshake{sock};
}
