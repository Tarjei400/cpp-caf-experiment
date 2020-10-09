//
// Created by Adrian Jutrowski on 06/10/2020.
//

#ifndef TEST_MAIN_H
#define TEST_MAIN_H

#include <algorithm>
#include <chrono>
#include <iostream>
#include <array>
#include <thread>
#include <sensor.pb.h>
#include <caf/all.hpp>
#include <google/protobuf/util/json_util.h>
#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>


// This file is partially included in the manual, do not modify
// without updating the references in the *.tex files!
// Manual references: lines 58-75 (MessagePassing.tex)

using std::cout;
using std::endl;
using std::pair;
using boost::asio::ip::tcp;
using namespace caf;

using EventActor = event_based_actor;
using SomeAtom = atom_constant<atom("Some")>;
using AcceptConnection = atom_constant<atom("Accept")>;
using ReadData = atom_constant<atom("Read")>;
using WriteData = atom_constant<atom("Write")>;
using BoostError = atom_constant<atom("BoostError")>;
using ParseMessage = atom_constant<atom("ParseMese")>;
using CreateConnectionMsg = atom_constant<atom("CreateCon")>;
using InitConnection = atom_constant<atom("InitConnec")>;
using StartAcceptingCmd = atom_constant<atom("StartAcc")>;

const int PACKET_SIZE = 20;

boost::asio::io_service ioService;
boost::asio::io_service::work work(ioService);

using Buffer = std::array<char, PACKET_SIZE>;

struct Socket {
    boost::asio::deadline_timer readTimer{ioService};
    boost::asio::deadline_timer writeTimer{ioService};
    boost::shared_ptr<tcp::socket> socket{new tcp::socket(ioService)};


};

struct ServerState {
    int port{0};
    std::string name{""};
    tcp::acceptor* acceptor;

};
struct ConnectionState {
    std::shared_ptr<Socket> socket;
    Buffer buffer;

    bool receivedFirst{false};

};
using Test = typed_actor<replies_to<BoostError, int>::with<int>>;
using ParserType = typed_actor<replies_to<ParseMessage, Buffer>::with<int>>;

class ParserActor : public ParserType::base {
public:
    ParserActor(actor_config& cfg) : ParserType::base(cfg) {
        // nop
    }

    behavior_type make_behavior() override {
        return make_typed();
    }

    ParserType::behavior_type make_typed() {
        return {
                [=](ParseMessage, Buffer data) {
                    cout << "Parsed message: " << std::string(std::begin(data), std::end(data)) << endl;
                    return 1;
                },
        };
    }
};

class ErrorHandler : public event_based_actor {
public:
    ErrorHandler(actor_config& cfg) : event_based_actor(cfg) {
        // nop
    }

    behavior_type make_behavior() override {
        return {
                [=](BoostError, std::string msg, int code) {
                    if(code > 0) {
                        cout << "Code: " << code << " " << msg <<  endl; // Connection closed cleanly by peer.
                    }
                }
        };
    }

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

protected:
    behavior make_behavior() override {


        return {
            [this](ReadData){
                cout << "Trying to read data ..." <<endl;
                state.socket->socket->async_read_some(
            boost::asio::buffer(state.buffer, PACKET_SIZE),
            [ this](const boost::system::error_code& e, std::size_t bytes_transferred   ){
                        cout << "Reqding" << bytes_transferred << state.buffer.size() << " " << std::string(state.buffer.begin(), state.buffer.end()) << endl;
                        if (e) {
                            return delegate(errorHandler, BoostError::value, e.message(), e.value());
                        }
                        send(this, ReadData::value);
                });
            },
        };
    }
private:
    behavior error_;

};

void readAsync( const std::shared_ptr<Socket>& socket, std::array<char, PACKET_SIZE>* buffer);


class ServerActor : public stateful_actor<ServerState> {
public:
    ServerActor(actor_config& cfg, int port)
            : stateful_actor<ServerState>(cfg)
    {
        state.port = port;
        state.acceptor = new tcp::acceptor(ioService, tcp::endpoint(tcp::v4(), 8080));

        errorHandler = home_system().spawn<ErrorHandler>();

    }

    std::vector<caf::actor> connections;
    caf::actor errorHandler;

    ~ServerActor() {
        cout << "Deleting actor" << endl;
    }
protected:

    behavior make_behavior() override {

        send(this, AcceptConnection::value);

        return {
            [this](AcceptConnection) {

                cout << " Accepting next connection" << endl;

                std::shared_ptr<Socket> socket = std::make_shared<Socket>() ;
                auto connection = home_system().spawn<ConnectionActor>(socket);
                connections.emplace_back(connection);

                state.acceptor->async_accept(*socket->socket, [=](const boost::system::error_code& e) {
                    if (e){
                        return delegate(errorHandler, BoostError::value, e.message(), e.value());
                    }
                    request(connection, std::chrono::milliseconds(30), ReadData::value).then([=](){
                        send(this, AcceptConnection::value);
                    });

                });
            },
        };
    }
};
//
//struct config : actor_system_config {
//    config() {
//        add_message_type<SomeAtom>("SomeAtom");
//    }
//};


#endif //TEST_MAIN_H
