#include "static_server.h"

void StaticServer::accept_connection(tcp_socket socket)
{
    std::cout << "Accepting connection from: " << socket.remote_endpoint().address().to_string() << std::endl;
    std::string message = "Hello from server";
    asio::write(socket, asio::buffer(message));
    socket.close();
}
