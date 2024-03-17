#include "static_file_server.h"

StaticFileServer::StaticFileServer(tcp_socket socket):
    m_sock(socket)
{
    asio::async_read_until(*m_sock, m_request_buf, "\r\n\r\n",
[this](const system::error_code & ec, std::size_t bytes_transferred) {
        on_request_received(ec, bytes_transferred);
    });
}

void StaticFileServer::on_finish(const system::error_code & ec)
{
    if (ec.value() != 0)
        std::cout << "[Server Client] failed to read request: " << ec.message() << std::endl;

    m_sock->close();
    delete this;
}

void StaticFileServer::on_request_received(const system::error_code & ec, std::size_t bytes_transferred)
{
    if (ec.value() != 0)
        on_finish(ec);

    std::string data;
    std::istream stream(&m_request_buf);
    std::getline(stream, data);
    data = data.substr(0, data.size() - 1); // remove \r (carriage return

    std::vector<std::string> metadata = StaticFileServer::string_split(data, " ");

    if (metadata.size() < 3 || metadata[2] != "HTTP/1.1" || metadata[0] != "GET") {
        on_finish(system::error_code(system::errc::protocol_error, system::generic_category()));
        return;
    }

    file_path = metadata[1];

    if (file_path == "/")
        file_path = "/index.html";

    std::cout << "[Server Client] requested file: " << file_path << std::endl;

    // read headers
    std::string header_name, header_value;

    while (true) {
        std::getline(stream, data);

        if (data.empty()) break;

        auto separator_pos = data.find(':');

        if (separator_pos != std::string::npos) {
            header_name = data.substr(0, separator_pos);

            if (separator_pos < data.length() - 1) {
                header_value = data.substr(separator_pos + 1);
                header_value.erase(0, header_value.find_first_not_of(' '));

                if (header_value.back() == '\r')
                    header_value.pop_back();
            }
            else
                header_value = "";

            m_headers[header_name] = header_value;
        }
    }

    send_response();
}

std::vector<std::string> StaticFileServer::string_split(std::string text, std::string delimiter)
{
    std::vector<std::string> parts;
    std::size_t pos = 0;
    std::string token;

    while ((pos = text.find(delimiter)) != std::string::npos) {
        token = text.substr(0, pos);
        parts.push_back(token);
        text.erase(0, pos + delimiter.length());
    }

    parts.push_back(text);
    return parts;
}

void StaticFileServer::send_response()
{
    std::string current_path = "C:\\repositories\\monopoly_bank";
    std::ifstream file(current_path + "/www" + file_path);

    if (!file.good()) {
        send_not_found();
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    m_response = "HTTP/1.1 200 OK\r\n"
                 "Content-Length: " + std::to_string(content.size()) + "\r\n"
                 "Connection: close\r\n";

    std::string ext = file_path.substr(file_path.find_last_of('.') + 1);

    if (ext == "js" || ext == "css") {
        m_response += "Cache-Control: max-age=31536000\r\n";
    }

    if (ext == "html") {
        m_response += "Cache-Control: no-cache, no-store, must-revalidate\r\n";
        m_response += "Pragma: no-cache\r\n";
        m_response += "Expires: 0\r\n";
        m_response += "Content-Type: text/html\r\n";
    } else if (ext == "js") {
        m_response += "Content-Type: application/javascript\r\n";
    } else if (ext == "css") {
        m_response += "Content-Type: text/css\r\n";
    } else {
        m_response += "Content-Type: text/plain\r\n";
    }

    m_response += "\r\n" + content;

    asio::async_write(*m_sock, asio::buffer(m_response),
    [this] (const system::error_code & ec, std::size_t bytes_transferred) {
        on_finish(ec);
    });
}

void StaticFileServer::send_not_found()
{
    m_response = "HTTP/1.1 404 Not Found\r\n"
                 "Content-Length: 0\r\n"
                 "Connection: close\r\n"
                 "\r\n";

    asio::async_write(*m_sock, asio::buffer(m_response),
    [this] (const system::error_code & ec, std::size_t bytes_transferred) {
        on_finish(ec);
    });
}
