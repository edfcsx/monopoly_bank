#include "connection_manager.h"
#include <iostream>

ConnectionManager::ConnectionManager() :
    m_connections(std::make_shared<std::unordered_map<std::string, Connection *>>())
{}

ConnectionManager::~ConnectionManager()
{
    for (auto & connection : *m_connections) {
        auto & [id, conn] = connection;
        delete conn;
    }

    m_connections->clear();

    for (auto & playerList : *m_players) {
        auto & [username, player] = playerList;
        delete player;
    }

    m_players->clear();
}

void ConnectionManager::AcceptWebsocketConnection(ptr_socket sock)
{
    new WebSocketHandshake{sock, this, [](ptr_socket ret_sock, ConnectionManager * manager) {
        std::cout << "connection from websocket\n";

        auto id = UUID::v4();
        auto * connection = new Connection(ret_sock);
        connection->SetConnectionId(id);

        std::lock_guard lock(manager->m_connectionsMutex);
        manager->m_connections->insert({id, connection });
    }};
}

void ConnectionManager::AcceptRawConnection(ptr_socket sock)
{
    auto id = UUID::v4();
    auto * connection = new Connection(sock);
    connection->SetConnectionId(id);
    std::lock_guard lock(m_connectionsMutex);
    m_connections->insert({id, connection });
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
