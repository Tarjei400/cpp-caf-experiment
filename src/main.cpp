/******************************************************************************\
 * This example illustrates how to do time-triggered loops in libcaf.         *
\******************************************************************************/
#include <boost/di.hpp>
#include "main.h"

namespace di = boost::di;


auto Module = []( auto&& ...deps){

    return di::make_injector(
            std::move(deps)...
    );
};


void caf_main(actor_system& system) {

    Sensor sensor;
    sensor.set_name("Laboratory");
    sensor.set_temperature(23.4);
    sensor.set_humidity(68);
    sensor.set_door(Sensor_SwitchLevel_OPEN);
    cout << "Spawning Server" <<endl;

    scoped_actor self{system};
   // auto acceptor = new tcp::acceptor(ioService, tcp::endpoint(tcp::v4(), 8080));

//    accept(acceptor);
    //auto server = self->spawn<ServerActor>(8080);

    std::string json_string;
    // Create a json_string from sr.
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    MessageToJsonString(sensor, &json_string, options);

    // Print json_string.
    std::cout << json_string << std::endl;

//    try{
//        ioService.run();
//    } catch (...) {
//        cout << "Error" << endl;
//    }
//    cout << "Service stooped: " <<endl;
//    sleep(10000);

    struct IConnection;

    struct IServer {
       virtual std::shared_ptr<IConnection> getConnection() = 0;

    };
    struct IParser {
        virtual std::string parsed() = 0;
    };

    struct IConnection {
       virtual std::shared_ptr<IParser> getParser() = 0;
    };



    struct ServerImpl : public IServer {
        std::shared_ptr<IConnection> conn{nullptr};

        std::shared_ptr<IConnection> getConnection() override { return conn; };
        ServerImpl(std::shared_ptr<IConnection> connection): conn(connection){ }
    };

    struct ConnectionImpl : public IConnection {
        std::shared_ptr<IParser> parser{nullptr};
        std::shared_ptr<IParser> getParser() override { return parser; };

        ConnectionImpl(const std::shared_ptr<IParser>& p): parser(std::move(p)){ }

    };

    struct ParserImpl1 : public IParser {
        std::string conf{"def"};

        ParserImpl1(std::string v): conf(v) {}
        virtual std::string parsed() {
            return "First impl" + conf;
        };

    };
    struct ParserImpl2 : public IParser {
        std::string conf{"def"};

        ParserImpl2(std::string v): conf(v) {}
        virtual std::string parsed() {
            return "Second impl" + conf;
        };

    };

    auto MainModule = Module(
        di::bind<IServer>().to<ServerImpl>(),
        di::bind<IConnection>().to<ConnectionImpl>()
    );

    auto aInjector = Module(
        di::bind<std::string>().to("42"),
        di::bind<IParser>().to<ParserImpl1>()
    );

    auto bInjector = Module(
        di::bind<std::string>().to("BAR"),
        di::bind<IParser>().to<ParserImpl2>()
    );

    auto ModuleAInjector = Module(
        MainModule,
        aInjector
    );

    auto ModulebInjector = Module(
        MainModule,
        bInjector,
        di::bind<std::string>().to("overriden value :D")[di::override]
    );


    auto c = ModuleAInjector.create<std::shared_ptr<IServer>>();
    auto d = ModulebInjector.create<std::shared_ptr<IServer>>();

    cout << "Injected: " << c->getConnection()->getParser()->parsed() << endl;
    cout << "Injected: " << d->getConnection()->getParser()->parsed() << endl;

    return;
}

CAF_MAIN()