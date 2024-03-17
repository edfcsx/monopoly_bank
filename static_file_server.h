#ifndef STATIC_SERVER_H
#define STATIC_SERVER_H

#include "networking.h"

class StaticServer
{
public:
    void accept_connection(tcp_socket socket);
};


#endif // STATIC_SERVER_H