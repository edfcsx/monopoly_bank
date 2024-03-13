#include "connection_manager.h"

ConnectionManager::ConnectionManager() :
    m_connections(std::unordered_map<std::string, std::shared_ptr<Connection>>())
{}

ConnectionManager::~ConnectionManager()
{
    m_connections.clear();
}

void ConnectionManager::accept_connection(tcp_socket socket, ConnProtocol p)
{
    std::string ip = socket->remote_endpoint().address().to_string();
    std::unique_lock<std::mutex> check_lock(m_connections_lock);

    if (m_connections.find(ip) != m_connections.end()) {
        if (m_connections[ip]->is_open()) {
            std::cout << "[Server] connection already exists for ip: " << ip << std::endl;
            m_connections[ip]->close_connection();
        }

        m_connections.erase(ip);
    }

    check_lock.unlock();

    if (p == ConnProtocol::RAW) {
        std::lock_guard<std::mutex> lock(m_connections_lock);

        m_connections.insert({
            socket->remote_endpoint().address().to_string(),
            std::make_shared<Connection>(socket, p)
        });
    } else if (p == ConnProtocol::WEBSOCKET) {
        auto * handshake = new WebSocketHandshake(socket, this, [](ptr_socket s, ConnectionManager * self) {
            std::lock_guard<std::mutex> lock(self->m_connections_lock);

           self->m_connections.insert({
                s->remote_endpoint().address().to_string(),
                std::make_shared<Connection>(s, ConnProtocol::WEBSOCKET)
            });
        });
    }
}

std::shared_ptr<Connection> ConnectionManager::get_connection(const std::string & ip)
{
    if (m_connections.find(ip) != m_connections.end())
        return m_connections.at(ip);
    else
        return nullptr;
}
