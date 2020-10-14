//
// Created by Adrian Jutrowski on 12/10/2020.
//

#ifndef SERVER_CONNECTIONACTOR_H
#define SERVER_CONNECTIONACTOR_H

#include <algorithm>
#include <chrono>
#include <iostream>
#include <array>
#include <thread>
#include <caf/all.hpp>
#include <google/protobuf/util/json_util.h>
#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>

using namespace caf;

struct ConnectionState {
    std::shared_ptr<Socket> socket;
    Buffer buffer;

    bool receivedFirst{false};

};

class ConnectionActor : public stateful_actor<ConnectionState> {
public:
    ConnectionActor(actor_config& cfg, std::shared_ptr<Socket> socket)
            : stateful_actor<ConnectionState>(cfg)
    {
        state.socket = socket;
        errorHandler = home_system().spawn<ErrorHandler>();

    }

    ~ConnectionActor(){
        cout << "ConnectionActor deleted" <<endl;
    }
private:
    caf::actor errorHandler;

    behavior nonAccepting;
    behavior ready;


protected:
    behavior make_behavior() override {

        nonAccepting.assign([](ReadData) {});
        ready.assign( [this](ReadData){
            cout << "Trying to read data ..." <<endl;
            become(nonAccepting);
            state.socket->socket->async_read_some(
                    boost::asio::buffer(state.buffer, PACKET_SIZE),
                    [=](const boost::system::error_code& e, std::size_t bytes_transferred ){
                        onRead(e, bytes_transferred);
                    }
            );
        });

        return ready;
    }

    void onRead(const boost::system::error_code& e, std::size_t bytes_transferred )
    {
        cout << "Reqding" << bytes_transferred << state.buffer.size() << " " << std::string(state.buffer.begin(), state.buffer.end()) << endl;
        if (e) {
            //return delegate(errorHandler, BoostError::value, e.message(), e.value());
        }
        become(ready);
        send(this, ReadData::value);
    }
private:
    behavior error_;

};

#endif //SERVER_CONNECTIONACTOR_H
