#ifndef PING_COMMAND_H_
#define PING_COMMAND_H_

#include "icommand.h"
#include "player.h"
#include "json.hpp"

class PingCommand : public Icommand
{
public:
    void execute(std::shared_ptr<std::unordered_map<std::string, Player *>> m_players, nlohmann::json data) override {
        if (data.contains("username")) {
            std::string username = data["username"];

            if (m_players->find(username) != m_players->end()) {
                if ((*m_players)[username]->m_connection && (*m_players)[username]->m_connection->is_open()) {
                    (*m_players)[username]->m_connection->Send(nlohmann::json{
                        {"code", SERVER_CODES::PING_RESPONSE},
                        {"message", "Pong!"}
                    });
                }
            }
        }
    };
};

#endif