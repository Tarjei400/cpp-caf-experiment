
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
