#include "server.h"
#include "ping_command.h"
#include "transfer_command.h"
#include "authenticate_command.h"

Server::Server() :
    m_ios(asio::io_service {}),
    m_isStopped(false),
    m_commands(std::unordered_map<server::actions, std::unique_ptr<Icommand>>()),
    m_acceptors(std::unordered_map<server::protocol, std::unique_ptr<tcp_acceptor>>())
{
    m_acceptors[server::protocol::raw] = std::make_unique<tcp_acceptor>(m_ios,tcp_endpoint(asio::ip::address_v4::any(), 3333));
    m_acceptors[server::protocol::websocket] = std::make_unique<tcp_acceptor>(m_ios, tcp_endpoint(asio::ip::address_v4::any(), 4444));
    m_acceptors[server::protocol::http] = std::make_unique<tcp_acceptor>(m_ios, tcp_endpoint(asio::ip::address_v4::any(), 80));

    m_work = std::make_unique<asio::io_service::work>(m_ios);

    m_commands[server::actions::ping] = std::make_unique<PingCommand>();
    m_commands[server::actions::authenticate] = std::make_unique<AuthenticateCommand>();
}

Server::~Server()
{
    Stop();
}

void Server::Start(uint thread_pool_size)
{
    if (thread_pool_size <= 0)
        throw std::runtime_error("No thread pool available for server");

    for (auto& acc : m_acceptors) {
        auto & [protocol, acceptor] = acc;
        acceptor->listen();
        listen(protocol);
    }

    std::cout << "[Server] starting server with " << thread_pool_size << " threads.\n";
    std::cout << "[Server] server is already to accept connections\n";

    // create and start specified number of threads
    for (uint i = 0; i < thread_pool_size; ++i)
    {
        std::unique_ptr<std::thread> th(new std::thread([this]() { m_ios.run(); }));
        m_thread_pool.push_back(std::move(th));
    }

    std::cout << "[Server] server started: " <<
        "raw: 3333" << " Websocket: 4444" << std::endl;
}

void Server::Stop()
{
    std::cout << "[Server] stopping accept connections" << std::endl;

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
    std::cout << "[Server] server stopped" << std::endl;
}

void Server::listen(server::protocol protocol)
{
    auto & acceptor = m_acceptors[protocol];
    auto socket = std::make_shared<asio::ip::tcp::socket>(m_ios);

    acceptor->async_accept(*socket, [this, socket, protocol](const system::error_code & ec) {
        if (ec)
            std::cout << "[Server] failed to accept connection: " << ec.message() << std::endl;
        else {
            if (protocol == server::protocol::http)
                new StaticFileServer(socket);
            else
                m_connections.accept_connection(socket, protocol);
        }


        // Init next async accept operation if acceptor has not been stopped yet
        if (!m_isStopped.load())
            listen(protocol);
        else
            Stop();
    });
}

void Server::process_io_messages()
{
    std::unique_lock<std::mutex> lock(m_connections.m_connections_lock);

    for (auto & [ip, conn] : m_connections.m_connections) {
        if (conn->is_open()) {
            conn->send_out_messages();

            for (auto & msg : conn->get_in_messages()) {
                if (m_commands.find(msg["code"]) != m_commands.end()) {
                    m_commands[msg["code"]]->execute(msg);
                } else {
                    std::cout << "[Server] unknown message code: " << msg["code"] << std::endl;
                }
            }

            conn->clear_in_messages();
        }
    }
}
