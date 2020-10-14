//
// Created by Adrian Jutrowski on 12/10/2020.
//

#ifndef SERVER_SERVERACTOR_H
#define SERVER_SERVERACTOR_H

#include <algorithm>
#include <chrono>
#include <iostream>
#include <array>
#include <thread>
#include <caf/all.hpp>
#include <google/protobuf/util/json_util.h>
#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include "AtomMessages.h"

using namespace caf;

struct ServerState {
    int port{0};
    std::string name{""};
    tcp::acceptor* acceptor;

};

class ServerActor : public event_based_actor {
public:

    INetworkConfiguration config;

    ServerActor(actor_config& cfg, int port)
            : event_based_actor(cfg)
    {
        acceptor = new tcp::acceptor(ioService, tcp::endpoint(tcp::v4(), 8080));
    }

    std::vector<caf::actor> connections;
    //ErrorHandler errorHandler;

    ~ServerActor() {
        cout << "Deleting actor" << endl;
    }
protected:

    behavior make_behavior() override {

        send(this, AcceptConnection::value);

        return {
            [this](AcceptConnection) {

                cout << " Accepting next connection" << endl;

                std::shared_ptr<Socket> socket = std::make_shared<Socket>();
                auto connection = home_system().spawn<ConnectionActor>(socket);

                state.acceptor->async_accept(*socket->socket, [=](const boost::system::error_code& e) {
                    onAccept(connection, e);
                });
            },
        };
    }

    void onAccept(const caf::actor& connection, const boost::system::error_code& e) {
        if (e){
            //  delegate(errorHandler, BoostError::value, e.message(), e.value());
            return;
        }
        //
        request(connection, std::chrono::milliseconds(30), ReadData::value).then([=](){
            send(this, AcceptConnection::value);
        });
    }
};


#endif //SERVER_SERVERACTOR_H
