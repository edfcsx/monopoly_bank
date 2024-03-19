#ifndef PING_COMMAND_H_
#define PING_COMMAND_H_

#include "icommand.h"
#include "player.h"
#include "json.hpp"
#include "server.h"

class PingCommand : public Icommand
{
public:
    void execute(nlohmann::json data) override {
        auto & conn_manager = Server::getInstance().m_connections;
        auto connection = conn_manager.get_connection(data["ip"]);

        if (connection && connection->is_open()) {
            connection->push_out_message(nlohmann::json{
                { "code", server::actions::pong },
                { "message", "Pong!" }
            }, data);
        }
    };
};

#endif