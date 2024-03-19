#ifndef PROFILE_COMMAND_H
#define PROFILE_COMMAND_H

#include "icommand.h"
#include "json.hpp"
#include "server.h"
#include "player.h"
#include "networking.h"

class ProfileCommand : public Icommand
{
public:
    void execute(nlohmann::json data) override {
        auto & conn_manager = Server::getInstance().m_connections;
        std::lock_guard<std::mutex> lock(conn_manager.m_players_lock);

        auto connection = conn_manager.get_connection(data["ip"]);
        auto player = conn_manager.get_player(data["ip"]);

        if (player == nullptr) {
            return;
        }

        if (connection && connection->is_open()) {
            if (player) {
                connection->push_out_message(nlohmann::json{
                    { "code", server::actions::send_profile },
                    { "balance", player->GetMoney() }
                }, data);
            }
        }
    };
};


#endif // PROFILE_COMMAND_H