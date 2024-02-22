#ifndef TRANSFER_COMMAND_H_
#define TRANSFER_COMMAND_H_

#include "icommand.h"
#include "player.h"
#include "json.hpp"
#include "iostream"

class TransferCommand : public Icommand
{
public:
    void execute(std::shared_ptr<std::unordered_map<std::string, Player *>> m_players, nlohmann::json data) override {

        if (data.contains("username") && data.contains("dest") && data.contains("amount")) {
            std::string username = data["username"];
            std::string dest = data["dest"];
            uint amount = data["amount"];

            if (m_players->find(username) != m_players->end() && m_players->find(dest) != m_players->end()) {
                if ((*m_players)[username]->Withdraw(amount)) {
                    (*m_players)[dest]->IncreaseMoney(amount);

                    if ((*m_players)[username]->m_connection && (*m_players)[username]->m_connection->IsOpen()) {
                        (*m_players)[username]->m_connection->Send(nlohmann::json{
                            {"code", SERVER_CODES::TRANSFER_SUCCESS },
                            {"message", "Transfer successful!"}
                        });

                        (*m_players)[username]->SendProfile();
                    }

                    if ((*m_players)[dest]->m_connection && (*m_players)[dest]->m_connection->IsOpen()) {
                        (*m_players)[dest]->m_connection->Send(nlohmann::json{
                            {"code", SERVER_CODES::TRANSFER_RECEIVED},
                            {"from", username},
                            {"amount", amount}
                        });

                        (*m_players)[dest]->SendProfile();
                    }
                } else {
                    if ((*m_players)[username]->m_connection && (*m_players)[username]->m_connection->IsOpen()) {
                        (*m_players)[username]->m_connection->Send(nlohmann::json{
                            {"code", SERVER_CODES::TRANSFER_NO_FUNDS},
                            {"message", "Insufficient funds!"}
                        });
                    }
                }
            } else {
                if (m_players->find(data["username"]) != m_players->end()) {
                    if ((*m_players)[data["username"]]->m_connection && (*m_players)[data["username"]]->m_connection->IsOpen()) {
                        (*m_players)[data["username"]]->m_connection->Send(nlohmann::json{
                            {"code", SERVER_CODES::TRANSFER_INVALID},
                            {"message", "Invalid transfer data!"}
                        });
                    }
                }
            }
        } else {
            if (data.contains("username") && m_players->find(data["username"]) != m_players->end()) {
                if ((*m_players)[data["username"]]->m_connection && (*m_players)[data["username"]]->m_connection->IsOpen()) {
                    (*m_players)[data["username"]]->m_connection->Send(nlohmann::json{
                        {"code", SERVER_CODES::TRANSFER_INVALID},
                        {"message", "Invalid transfer data!"}
                    });
                }
            } else {
                std::cout << "Invalid transfer data!" << std::endl;
            }
        }
    };
};


#endif // TRANSFER_COMMAND_H_