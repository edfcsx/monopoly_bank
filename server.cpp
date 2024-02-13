#include <iostream>
#include <memory>
#include "server.h"
#include "json.hpp"

using std::cout;
using std::endl;

Server::Server() :
    m_ios(asio::io_service {}),
    m_isStopped(false),
    m_acceptor(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), 3333)),
    m_connections_limit(0)
{
    m_work = std::make_unique<asio::io_service::work>(m_ios);
}

Server::~Server()
{
    Stop();
}

void Server::Start(uint port_num, uint thread_pool_size)
{
    if (thread_pool_size <= 0)
        throw std::runtime_error("No thread pool available for server");

    m_acceptor.listen();
    InitAcceptConnections();

    cout << "[Server] starting server with " << thread_pool_size << " threads." << endl;
    cout << "[Server] server is already to accept connections" << endl;

    // create and start specified number of threads
    for (uint i = 0; i < thread_pool_size; ++i)
    {
        std::unique_ptr<std::thread> th(new std::thread([this]() { m_ios.run(); }));
        m_thread_pool.push_back(std::move(th));
    }

    cout << "[Server] server started on port: " << port_num << endl;
}

void Server::Stop()
{
    cout << "[Server] stopping accept connections" << endl;
    // stop accepting connections
    m_isStopped.store(true);
    m_acceptor.close();

    // close connections
    for (auto& conn : m_connections)
        conn->Close();

    m_connections.clear();

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

void Server::InitAcceptConnections()
{
    auto sock = std::make_shared<asio::ip::tcp::socket>(m_ios);

    m_acceptor.async_accept(*sock, [this, sock](const system::error_code & ec) {
        if (!ec) {
            if (m_connections.size() >= m_connections_limit)
                Server::RejectConnection(sock, SERVER_CODES::LIMIT_REACHED);
            else
                RequestAuthorization(sock);
        }
        else
            cout << "[Server] failed to accept connection: " << ec.message() << endl;

        // Init next async accept operation if acceptor has not been stopped yet
        if (!m_isStopped.load())
            InitAcceptConnections();
        else
            Stop();
    });
}

void Server::DumpClosedConnections()
{
    m_connections.remove_if([](Connection * conn) {
        if (!conn->IsOpen())
        {
            delete conn;
            return true;
        }
        return false;
    });
}

void Server::RejectConnection(std::shared_ptr<asio::ip::tcp::socket> sock, SERVER_CODES code)
{
    cout << "[Server] rejecting connection from " <<
        sock->remote_endpoint().address().to_string() <<
        " code: " << code;

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

void Server::RequestAuthorization(std::shared_ptr<asio::ip::tcp::socket> sock)
{
    auto * request = new asio::streambuf{};

    asio::async_read_until(*sock, (*request), '\n',
       [this, request, sock](const system::error_code & ec, std::size_t bytes_transferred) {
        if (ec.value() != 0) {
            cout << "[Server] failed to read authorization request: " << ec.message() << endl;
            sock->close();
            return;
        }

        std::string request_data;
        std::istream is(&(*request));
        std::getline(is, request_data);

        try {
            nlohmann::json j = nlohmann::json::parse(request_data);

            if (j["code"] == SERVER_CODES::AUTHENTICATE) {
                std::string username = j["username"];
                std::string password = j["password"];

                if (m_players.find(username) != m_players.end()) {
                    Player * player = m_players[username];

                    if (player->m_password == password)
                        m_connections.emplace_back(new Connection(sock, player));
                    else
                        Server::RejectConnection(sock, SERVER_CODES::AUTHENTICATE_FAILED);
                } else {
                    auto * p = new Player(username, password);
                    m_players.insert({ username, p });
                    m_connections.emplace_back(new Connection(sock, p));
                }
            } else {
                Server::RejectConnection(sock, SERVER_CODES::NEED_AUTHENTICATE);
            }
        }
        catch (const nlohmann::json::parse_error & e) {
            Server::RejectConnection(sock, SERVER_CODES::NEED_AUTHENTICATE);
        }
    });
}
