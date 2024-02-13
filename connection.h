#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "networking.h"

class Connection
{
public:
    Connection(std::shared_ptr<asio::ip::tcp::socket> sock);
    ~Connection();

    void Close();
    [[nodiscard]] bool IsOpen() const;

private:
    std::shared_ptr<asio::ip::tcp::socket> m_sock;
    asio::streambuf m_request;
    std::string m_response;

    bool m_isOpen;

    void IOListener();
};

#endif // CONNECTION_H_