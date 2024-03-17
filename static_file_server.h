#ifndef STATIC_FILE_SERVER_H
#define STATIC_FILE_SERVER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <memory>
#include "networking.h"

class StaticFileServer
{
public:
    StaticFileServer(tcp_socket socket);
private:
    tcp_socket m_sock;
    std::map<std::string, std::string> m_headers;
    asio::streambuf m_request_buf;
    std::string file_path;
    std::string m_response;
private:
    void on_request_received(const system::error_code & ec, std::size_t bytes_transferred);
    void on_finish(const system::error_code & ec);
    static std::vector<std::string> string_split(std::string text, std::string delimiter);
    void send_response();
    void send_not_found();
};


#endif // STATIC_FILE_SERVER_H