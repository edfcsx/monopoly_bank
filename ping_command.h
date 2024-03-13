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
        auto & server = Server::getInstance();

        if (data["is_player"]) {

        } else {
            auto connection = server.m_connections.get_connection(data["ip"]);

            if (connection && connection->is_open()) {
                connection->push_out_message(nlohmann::json{
                    { "code", server::actions::pong },
                    { "message", "Pong!" }
                });
            }
        }
    };
};

#endif