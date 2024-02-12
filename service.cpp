#include <iostream>
#include "service.h"

using std::cout;
using std::endl;

Service::Service(std::shared_ptr<asio::ip::tcp::socket> sock) :
        m_sock(sock)
{}

Service::~Service() {}

void Service::StartHandling() {
    asio::async_read_until(*m_sock, m_request, '\n',
   [this](const system::error_code & ec, std::size_t bytes_transferred) {
       OnRequestReceived(ec, bytes_transferred);
   });
}

void Service::OnRequestReceived(const system::error_code& ec, std::size_t bytes_transferred) {
    if (ec.value() != 0) {
        std::cout << "[Server] failed to read request: " << ec.message() << std::endl;
        OnFinish();
        return;
    }

    m_response = ProcessRequest(m_request);

    // initiate asynchronous write operation and call OnResponseSent upon completion
    asio::async_write(*m_sock, asio::buffer(m_response),
[this](const system::error_code& ec, std::size_t bytes_transferred) {
      OnResponseSent(ec, bytes_transferred);
    });
}

void Service::OnResponseSent(const system::error_code& ec, std::size_t bytes_transferred) {
    if (ec.value() != 0) {
        std::cout << "[Server] failed to write response: " << ec.message() << std::endl;
    }

    OnFinish();
}

void Service::OnFinish() {
    delete this;
}

std::string Service::ProcessRequest(asio::streambuf& request) {
    std::string response{ "responding request....\n" };
    return response;
}
