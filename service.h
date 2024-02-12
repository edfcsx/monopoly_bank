#ifndef _MONOPOLY_SERVICE_H_
#define _MONOPOLY_SERVICE_H_

#include <memory>
#include "asio_common.h"

using namespace boost;

class Service
{
public:
    Service(std::shared_ptr<asio::ip::tcp::socket> sock);
    ~Service();

    void StartHandling();

private:
    std::shared_ptr<asio::ip::tcp::socket> m_sock;
    asio::streambuf m_request;
    std::string m_response;

private:
    void OnRequestReceived(const system::error_code & ec, std::size_t bytes_transferred);
    void OnResponseSent(const system::error_code & ec, std::size_t bytes_transferred);
    void OnFinish();
    std::string ProcessRequest(asio::streambuf & request);
};

#endif // _MONOPOLY_SERVICE_H_
