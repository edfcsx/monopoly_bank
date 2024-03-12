#include "connection_manager.h"

ConnectionManager::ConnectionManager() :
    m_connections(std::unordered_map<std::string, std::unique_ptr<Connection>>())
{}

ConnectionManager::~ConnectionManager()
{
    m_connections.clear();
}

void ConnectionManager::accept_connection(tcp_socket socket, ConnProtocol p)
{
    if (p == ConnProtocol::RAW) {
        std::lock_guard<std::mutex> lock(m_connections_lock);

        m_connections.insert({
            socket->remote_endpoint().address().to_string(),
            std::make_unique<Connection>(socket, p)
        });
    } else if (p == ConnProtocol::WEBSOCKET) {
        auto * handshake = new WebSocketHandshake(socket, [this](ptr_socket sock) {
            std::lock_guard<std::mutex> lock(m_connections_lock);

            m_connections.insert({
                sock->remote_endpoint().address().to_string(),
                std::make_unique<Connection>(sock, ConnProtocol::WEBSOCKET)
            });
        });
    }
}
