#include "connection_manager.h"

ConnectionManager::ConnectionManager() :
    m_connections(std::unordered_map<std::string, std::unique_ptr<Connection>>())
{}

ConnectionManager::~ConnectionManager()
{
    m_connections.clear();
}

void ConnectionManager::RejectConnection(ptr_socket sock, SERVER_CODES status)
{
    std::cout << "[Server] rejecting connection from " <<
        sock->remote_endpoint().address().to_string() <<
        " code: " << status << '\n';

    nlohmann::json j;
    j["code"] = std::to_string(static_cast<int>(status));
    auto response = new std::string{j.dump()};

    asio::async_write(*sock, asio::buffer(*response), [response, sock](const system::error_code & ec, std::size_t bytes_transfered) {
        if (ec.value() != 0)
            std::cout << "[Server] failed to send reject response: " << ec.message() << std::endl;

        sock->close();
        delete response;
    });
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
        auto * handshake = new WebSocketHandshake(socket, this, [](ptr_socket sock, ConnectionManager * self) {
            std::lock_guard<std::mutex> lock(self->m_connections_lock);

            self->m_connections.insert({
                sock->remote_endpoint().address().to_string(),
                std::make_unique<Connection>(sock, ConnProtocol::WEBSOCKET)
            });
        });
    }
}
