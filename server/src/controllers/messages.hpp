#pragma once

#include "db/database.hpp"
#include "ws/ws.hpp"
#include "http/controller.hpp"
#include "shared.hpp"

class MessagesController : public Controller {
    public:
        MessagesController(Database& db, WebSocket& ws)
            : db_{db}, ws_{ws} {};

        AsyncResponse GET(RequestContext& request) override;
        AsyncResponse POST(RequestContext& request) override;

    private:
        void message_created(
            RequestContext& ctx,
            std::string message_id,
            std::string sender_iid,
            std::string receiver_iid
        );

        Database& db_;
        WebSocket& ws_;
};
