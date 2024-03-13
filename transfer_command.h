#ifndef TRANSFER_COMMAND_H_
#define TRANSFER_COMMAND_H_

#include "icommand.h"
#include "player.h"
#include "json.hpp"
#include "iostream"
#include "server.h"
#include "mutex"

struct TransferData {
    std::string username;
    std::string dest;
    uint amount{};

    static TransferData from_json(const nlohmann::json& j) {
        TransferData transfer;

        try {
            j.at("username").get_to<std::string>(transfer.username);
            j.at("dest").get_to<std::string>(transfer.dest);
            j.at("amount").get_to<uint>(transfer.amount);

            return transfer;
        } catch (nlohmann::json::exception& e) {
            TransferData transferInvalid;
            return transferInvalid;
        }
    }

    bool valid() const {
        Server & s = Server::getInstance();
        return !username.empty() &&
        !dest.empty() &&
        amount > 0;
//        &&
//        s.CheckPlayerExists(username) &&
//        s.CheckPlayerExists(dest);
    }
};

class TransferCommand : public Icommand
{
public:
    void execute(nlohmann::json data) override {
        TransferData transfer = TransferData::from_json(data);
        Server & server = Server::getInstance();

        if (!transfer.valid()) {
//            if (server.CheckPlayerExistsAndConnected(transfer.username)) {
//                (*m_players)[transfer.username]->m_connection->Send(nlohmann::json{
//                    { "code", SERVER_CODES::BAD_REQUEST },
//                    { "message", "Invalid data!" }
//                });
//            }
//
//            // @todo: implement logging
//            std::cout << "Invalid transfer data!" << std::endl;
//
//            return;
//        }
//
//        if (!(*m_players)[transfer.username]->Withdraw(transfer.amount)) {
//            if (server.CheckPlayerConnected(transfer.username)) {
//                (*m_players)[transfer.username]->m_connection->Send(nlohmann::json{
//                    { "code", SERVER_CODES::TRANSFER_NO_FUNDS },
//                    { "message", "Insufficient funds!" }
//                });
//            }
//
//            // @todo: implement logging
//            std::cout << "Invalid transfer no funds!" << std::endl;
//
//            return;
//        }
//
//        (*m_players)[transfer.dest]->IncreaseMoney(transfer.amount);
//
//        if (server.CheckPlayerConnected(transfer.username)) {
//            (*m_players)[transfer.username]->m_connection->Send(nlohmann::json{
//                { "code", SERVER_CODES::TRANSFER_SUCCESS },
//                { "message", "Transfer successful!" }
//            });
//
//            (*m_players)[transfer.username]->SendProfile();
//        }
//
//        if (server.CheckPlayerConnected(transfer.dest)) {
//            (*m_players)[transfer.dest]->m_connection->Send(nlohmann::json{
//                { "code", SERVER_CODES::TRANSFER_RECEIVED },
//                { "from", transfer.username },
//                { "amount", transfer.amount }
//            });
//
//            (*m_players)[transfer.dest]->SendProfile();
//        }
//    };

        }
    }
};

#endif // TRANSFER_COMMAND_H_