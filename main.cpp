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

// This file is partially included in the manual, do not modify
// without updating the references in the *.tex files!
// Manual references: lines 58-75 (MessagePassing.tex)

using std::cout;
using std::endl;
using std::pair;
using namespace boost::asio::ip::tcp;
using namespace caf;

using EventActor = event_based_actor;
using SomeAtom = atom_constant<atom("Some")>;
using CreateConnectionMsg = atom_constant<atom("CreateConnectionMsg")>;
using InitConnection = atom_constant<atom("InitConnection")>;
using StartAcceptingCmd = atom_constant<atom("StartAcceptingCmd")>;

struct ServerState {
    int clients{0};
    std::string name{""};
    asio::io_service ioService;
    asio::ip::tcp::acceptor* acceptor;

    ~ServerState() {
        delete acceptor;
    }
};

struct ConnectionState {
    boost::asio::ip::tcp::socket socket;
    enum { max_length = 1024 };
    char data_[max_length];
};
class session
{
public:
    session(boost::asio::io_service& io_service)
            : socket_(io_service)
    {
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    tcp::socket socket_;

};

class server
{
public:
    server(boost::asio::io_service& io_service, short port)
            : io_service_(io_service),
              acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        start_accept();
    }

private:
    void start_accept()
    {
        session* new_session = new session(io_service_);
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&server::handle_accept, this, new_session,
                                           boost::asio::placeholders::error));
    }

    void handle_accept(session* new_session,
                       const boost::system::error_code& error)
    {
        if (!error)
        {
            new_session->start();
        }
        else
        {
            delete new_session;
        }

        start_accept();
    }

    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};
class ConnectionActor: public stateful_actor<ConnectionState>
{
    ConnectionActor(actor_config& cfg, Io)
            : stateful_actor<ServerState>(cfg) {
        initializing_.assign([=](InitConnection) {

            become(running_);
        });


    }
protected:
    behavior make_behavior() override {
        // start thinking
        state.socket_.async_read_some(boost::asio::buffer(state.data_, state.max_length),
                                      boost::bind(&ConnectionActor::handle_read, this,
                                                  boost::asio::placeholders::error,
                                                  boost::asio::placeholders::bytes_transferred));
        send(this);
        // philosophers start to think after receiving {think}
        return {
                [=]() {
                    send(this, SomeAtom::value);
                    aout(this) << name_ << " starts to think\n";
                    become(denied_);

                },
        };
    }

private:
    void handle_read(const boost::system::error_code& error,
                     size_t bytes_transferred)
    {
        if (!error)
        {
            boost::asio::async_write(state.socket_,
                                     boost::asio::buffer(state.data_, bytes_transferred),
                                     boost::bind(&ConnectionActor::handle_write, this,
                                                 boost::asio::placeholders::error));
        }
        else
        {
            delete this;
        }
    }

    void handle_write(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_.async_read_some(boost::asio::buffer(state.data_, max_length),
                                    boost::bind(&ConnectionActor::handle_read, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            delete this;
        }
    }

    behavior running_;   // could not get first chopsticks
    behavior initializing_;   // could not get first chopsticks

};

class ServerActor : public stateful_actor<ServerState> {
public:
    ServerActor(actor_config& cfg, int port)
            : stateful_actor<ServerState>(cfg)
    {
        state.acceptor = new acceptor(state.ioService, endpoint(v4(), port));

        // a philosopher that receives {eat} stops thinking and becomes hungry
        // philosopher was *not* able to obtain the first chopstick
        accepting_.assign([=](CreateConnectionMsg) {

            if (state.clients > 10) {
                quit();
            }
            state.clients++;
            cout << "accepting_ new connection" << endl;
//            delayed_send(this, std::chrono::seconds(3) , CreateConnectionMsg::value);

            become(eating_);
        });
        // philosopher obtained both chopstick and eats (for five seconds)
        eating_.assign([=](SomeAtom) {
            delayed_send(this, std::chrono::seconds(3), SomeAtom::value);

            cout << "thinking" << endl;
            become(denied_);
        });
    }

    void setName(const std::string n) { state.name = n; }
    void increment() { state.clients++; }

protected:
    behavior make_behavior() override {
        // start thinking
        send(this, SomeAtom::value);
        // philosophers start to think after receiving {think}
        return {
                [=](SomeAtom) {
                    send(this, SomeAtom::value);
                    aout(this) << name_ << " starts to think\n";
                    become(denied_);

                },
        };
    }

private:
    std::string name_;  // the name of this philosopher
    behavior denied_;   // could not get first chopsticks
    behavior eating_;   // wait for some time, then go thinking again
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