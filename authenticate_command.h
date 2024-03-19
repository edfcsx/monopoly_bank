#ifndef AUTHENTICATE_COMMAND_H_
#define AUTHENTICATE_COMMAND_H_

#include <string>
#include "player.h"
#include "icommand.h"
#include "server.h"

class AuthenticateCommand : public Icommand
{
public:
    void execute(nlohmann::json data) override {
        if (data.contains("username") && data.contains("password")) {
            auto & conn_manager = Server::getInstance().m_connections;

            std::string user = data["ip"];
            std::string username = data["username"];
            std::string password = data["password"];

            auto connection = conn_manager.get_connection(user);
            std::lock_guard<std::mutex> lock(conn_manager.m_players_lock);
            auto player = conn_manager.get_player(user);

            if (player == nullptr) {
                conn_manager.m_players.insert({
                    user,
                    std::make_shared<Player>(username, password)
                });

                connection->push_out_message(nlohmann::json{
                    { "code", server::actions::authenticate_success },
                    { "message", "Authenticated!" }
                }, data);

                connection->set_playing(true);
            } else {
                if (player->GetPassword() == password) {
                    connection->push_out_message(nlohmann::json{
                        { "code", server::actions::authenticate_success },
                        { "message", "Authenticated!" }
                    }, data);

                    connection->set_playing(true);
                } else {
                    connection->push_out_message(nlohmann::json{
                        { "code", server::actions::authenticate_failed },
                        { "message", "Invalid password!" }
                    }, data);
                }
            }
        }
    };
};

#endif // AUTHENTICATE_COMMAND_H_