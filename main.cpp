/******************************************************************************\
 * This example illustrates how to do time-triggered loops in libcaf.         *
\******************************************************************************/

#include <algorithm>
#include <chrono>
#include <iostream>
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
using BoostError = atom_constant<atom("BoostError")>;
using CreateConnectionMsg = atom_constant<atom("CreateCon")>;
using InitConnection = atom_constant<atom("InitConnec")>;
using StartAcceptingCmd = atom_constant<atom("StartAcc")>;

typedef boost::shared_ptr<tcp::socket> SocketPtr;
const int max_length = 2;
struct ServerState {
    int port{0};
    std::string name{""};
    boost::asio::io_service ioService;
    tcp::acceptor* acceptor;

};
struct ConnectionState {
    SocketPtr socket;

};
using Test = typed_actor<replies_to<BoostError, int>::with<int>>;





class ErrorHandler : public Test::base {
public:
    ErrorHandler(actor_config& cfg) : Test::base(cfg) {
        // nop
    }

    behavior_type make_behavior() override {
        return make_typed();
    }

    Test::behavior_type make_typed() {
        return {
                [=](BoostError, int error) {
                    if (error == boost::asio::error::eof) {
                        cout << "Connection closed cleanly by peer." << error <<  endl; // Connection closed cleanly by peer.
                        return 1;
                    }
                    else if (error) {
                        cout << "Error:" << error << endl;
                        return 1;
                    }
                },
        };
    }
};

class ConnectionActor : public stateful_actor<ConnectionState> {
public:
    ConnectionActor(actor_config& cfg, const SocketPtr& socket)
            : stateful_actor<ConnectionState>(cfg)
    {
        state.socket = socket;
    }

protected:
    behavior make_behavior() override {
        // start thinking

        send(this, SomeAtom::value);
        // philosophers start to think after receiving {think}
        auto errorHandler = home_system().spawn<ErrorHandler>();


        return {
            [this, errorHandler](SomeAtom) {

                char data[max_length];
                boost::system::error_code error;
                size_t length = 0;
                while(length < max_length*2 || error)
                {
                    length += state.socket->read_some(boost::asio::buffer(data /*, max_length - length*/), error);
                    cout << "Read: " << length << " bytes" << std::endl;

                }

                if(error){
                    return delegate(errorHandler, BoostError::value, error.value());
                }

                cout << "Writing bytes: " << data << std::endl;

                boost::asio::write(*state.socket, boost::asio::buffer(data, length));
                send(this, SomeAtom::value);

            }
        };
    }
private:
    behavior error_;

};
class ServerActor : public stateful_actor<ServerState> {
public:
    ServerActor(actor_config& cfg, int port)
            : stateful_actor<ServerState>(cfg)
    {
        state.port = port;
    }


protected:
    behavior make_behavior() override {
        // start thinking
        state.acceptor = new tcp::acceptor(state.ioService, tcp::endpoint(tcp::v4(), state.port));

        send(this, SomeAtom::value);

        return {
            [=](SomeAtom) {
                SocketPtr sock{new tcp::socket(state.ioService)};
                state.acceptor->accept(*sock);
                home_system().spawn<ConnectionActor>(sock);
                cout << "Accepted connection" << endl;
                send(this, SomeAtom::value);

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


void caf_main(actor_system& system) {
    scoped_actor self{system};

    Sensor sensor;
    sensor.set_name("Laboratory");
    sensor.set_temperature(23.4);
    sensor.set_humidity(68);
    sensor.set_door(Sensor_SwitchLevel_OPEN);

    self->spawn<ServerActor>(8080);

    std::string json_string;
    // Create a json_string from sr.
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    MessageToJsonString(sensor, &json_string, options);

    // Print json_string.
    std::cout << json_string << std::endl;


}

CAF_MAIN()