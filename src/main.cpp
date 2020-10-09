/******************************************************************************\
 * This example illustrates how to do time-triggered loops in libcaf.         *
\******************************************************************************/

#include "main.h"


/*
 * ReaderActor, WriterActor, SerializerActor, ErrorHandlerActor
 *
 */
void writeAsync(const std::shared_ptr<Socket>& socket, std::array<char, PACKET_SIZE>* buffer) {
    boost::asio::async_write(*socket->socket,
                             boost::asio::buffer(buffer, PACKET_SIZE),
                             [socket, buffer](const boost::system::error_code& e, std::size_t bytes_transferred   ){
                                 cout << "Writing" << bytes_transferred << buffer->size() << endl;
                                 if(e){
                                     cout << "Errr captured: " << e.message() << endl;
                                     return;
                                 }
                                 readAsync( socket, buffer);

                             });
}

void readAsync( const std::shared_ptr<Socket>& socket, std::array<char, PACKET_SIZE>* buffer) {
    socket->socket->async_read_some(
               boost::asio::buffer(buffer, PACKET_SIZE),
               [socket, buffer](const boost::system::error_code& e, std::size_t bytes_transferred   ){
                   cout << "Reqding" << bytes_transferred << buffer->size() << " " << std::string(buffer->begin(), buffer->end()) << endl;
                   if(e){
                       cout << "Err reading message: " << e.message() << endl;
                       return;
                   }
                   readAsync( socket, buffer);

               });
}

void accept(tcp::acceptor* acceptor) {

    std::shared_ptr<Socket> socket = std::make_shared<Socket>() ;
    std::array<char, PACKET_SIZE>* buffer = new std::array<char, PACKET_SIZE>();

    acceptor->async_accept(*socket->socket, [=](const boost::system::error_code& e) {
        cout << "Accepting connection." << endl;
        if (e){
            cout << "Error with accepting: " << e.message() << endl;
        }
        readAsync( socket, buffer);
        accept(acceptor);
    });
}

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
    auto server = self->spawn<ServerActor>(8080);

    std::string json_string;
    // Create a json_string from sr.
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    MessageToJsonString(sensor, &json_string, options);

    // Print json_string.
    std::cout << json_string << std::endl;

    try{
        ioService.run();
    } catch (...) {
        cout << "Error" << endl;
    }
    cout << "Service stooped: " <<endl;
    sleep(10000);
    return;
}

CAF_MAIN()