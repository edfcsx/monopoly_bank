#include <iostream>
#include <memory>
#include "server.h"
#include "json.hpp"
#include "ping_command.h"
#include "transfer_command.h"

using std::cout;
using std::endl;

Server::Server() :
    m_ios(asio::io_service {}),
    m_isStopped(false),
    m_connections_limit(0),
    m_players(std::make_shared<std::unordered_map<std::string, Player *>>()),
    m_commandsMap(std::unordered_map<SERVER_CODES, std::unique_ptr<Icommand>>()),
    m_acceptors(std::unordered_map<ConnectionProtocol, std::unique_ptr<asio::ip::tcp::acceptor>>())
{
    m_acceptors[ConnectionProtocol::RAW] = std::make_unique<asio::ip::tcp::acceptor>(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), 3333));
    m_acceptors[ConnectionProtocol::WEBSOCKET] = std::make_unique<asio::ip::tcp::acceptor>(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), 4444));

    m_work = std::make_unique<asio::io_service::work>(m_ios);
    m_commandsMap[SERVER_CODES::PING] = std::make_unique<PingCommand>();
    m_commandsMap[SERVER_CODES::TRANSFER] = std::make_unique<TransferCommand>();
}

Server::~Server()
{
    Stop();
}

void Server::Start(uint port_num, uint thread_pool_size)
{
    if (thread_pool_size <= 0)
        throw std::runtime_error("No thread pool available for server");

    for (auto& acc : m_acceptors) {
        auto & [protocol, acceptor] = acc;
        acceptor->listen();
        Listen(protocol);
    }

    std::cout << "[Server] starting server with " << thread_pool_size << " threads.\n";
    std::cout << "[Server] server is already to accept connections\n";

    // create and start specified number of threads
    for (uint i = 0; i < thread_pool_size; ++i)
    {
        std::unique_ptr<std::thread> th(new std::thread([this]() { m_ios.run(); }));
        m_thread_pool.push_back(std::move(th));
    }

    cout << "[Server] server started: " << "raw: " << port_num << " Websocket: 4444" << endl;
}

void Server::Stop()
{
    cout << "[Server] stopping accept connections" << endl;
    // stop accepting connections
    m_isStopped.store(true);

    for (auto& acceptor : m_acceptors)
        acceptor.second->close();

    // stop io_service
    m_ios.stop();

    // stop threads
    for (auto& th : m_thread_pool)
    {
        if (th->joinable())
            th->join();
    }

    m_thread_pool.clear();
    cout << "[Server] server stopped" << endl;
}

void Server::SetConnectionsLimit(uint limit)
{
    m_connections_limit = limit;
}

void Server::Listen(ConnectionProtocol protocol)
{
    auto & acceptor = m_acceptors[protocol];
    auto socket = std::make_shared<asio::ip::tcp::socket>(m_ios);

    acceptor->async_accept(*socket, [this, socket, protocol](const system::error_code & ec) {
        if (ec)
            cout << "[Server] failed to accept connection: " << ec.message() << endl;
        else {
            if (m_players->size() >= m_connections_limit)
                Server::RejectConnection(socket, SERVER_CODES::LIMIT_REACHED);
            else
                protocol == ConnectionProtocol::RAW ?
                    m_connectionManager.AcceptRawConnection(socket) :
                    m_connectionManager.AcceptWebsocketConnection(socket);
        }

        // Init next async accept operation if acceptor has not been stopped yet
        if (!m_isStopped.load())
            Listen(protocol);
        else
            Stop();
    });
}

void Server::RejectConnection(std::shared_ptr<asio::ip::tcp::socket> sock, SERVER_CODES code)
{
    cout << "[Server] rejecting connection from " <<
        sock->remote_endpoint().address().to_string() <<
        " code: " << code << '\n';

    nlohmann::json j;
    j["code"] = std::to_string(static_cast<int>(code));
    auto response = new string{j.dump()};

    asio::async_write(*sock, asio::buffer(*response), [response, sock](const system::error_code & ec, std::size_t bytes_transferred) {
        if (ec.value() != 0)
            cout << "[Server] failed to send reject response: " << ec.message() << endl;

        sock->close();
        delete response;
    });
}

void Server::AuthenticatePlayerHandler(std::shared_ptr<asio::ip::tcp::socket> sock, nlohmann::json player_data) {
    std::cout << "received: " << player_data.dump() << "\n";
    int code = std::atoi(player_data["code"].get<std::string>().c_str());

    if (code == SERVER_CODES::AUTHENTICATE) {
        std::string username = player_data["username"];
        std::string password = player_data["password"];

        if (m_players->find(username) != m_players->end()) {
            if ((*m_players)[username]->GetPassword() == password){
                (*m_players)[username]->AttachConnection(sock);
            }
            else
                Server::RejectConnection(sock, SERVER_CODES::AUTHENTICATE_FAILED);
        } else {
            auto * p = new Player(username, password, sock);
            m_players->insert({ username, p });
        }
    } else {
        Server::RejectConnection(sock, SERVER_CODES::NEED_AUTHENTICATE);
    }
}

std::shared_ptr<std::unordered_map<std::string, Player *>> Server::GetPlayers() {
    return m_players;
}

void Server::PushMessage(NetworkingMessage message) {
    std::lock_guard<std::mutex> lock(m_messageInMutex);
    m_messagesIn.push_back(message);

    for (auto & msg : m_messagesIn) {
        cout << "[Server] message: " << msg.data.dump() << endl;
    }
}

void Server::ProcessMessages() {
    std::lock_guard<std::mutex> lock(m_messageInMutex);
    std::lock_guard<std::mutex> players_lock(m_playersMutex);

    for (auto & msg : m_messagesIn) {
        if (m_commandsMap.find(msg.code) != m_commandsMap.end()) {
            m_commandsMap[msg.code]->execute(m_players, msg.data);
        } else {
            cout << "[Server] unknown message code: " << static_cast<int>(msg.code) << endl;
        }
    }

    m_messagesIn.clear();
}

bool Server::CheckPlayerExistsAndConnected(const std::string & username) {
    if (m_players->find(username) != m_players->end()) {
        return (*m_players)[username]->m_connection && (*m_players)[username]->m_connection->IsOpen();
    }

    return false;
}

bool Server::CheckPlayerExists(const std::string & username) {
    return m_players->find(username) != m_players->end();
}

bool Server::CheckPlayerConnected(const std::string & username) {
    return (*m_players)[username]->m_connection && (*m_players)[username]->m_connection->IsOpen();
}
