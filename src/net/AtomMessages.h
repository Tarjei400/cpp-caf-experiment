//
// Created by Adrian Jutrowski on 12/10/2020.
//

#ifndef SERVER_ATOMMESSAGES_H
#define SERVER_ATOMMESSAGES_H
#include <caf/all.hpp>


const int PACKET_SIZE = 20;
using Buffer = std::array<char, PACKET_SIZE>;


boost::asio::io_service ioService;
boost::asio::io_service::work work(ioService);


struct Socket {
    boost::asio::deadline_timer readTimer{ioService};
    boost::asio::deadline_timer writeTimer{ioService};
    boost::shared_ptr<tcp::socket> socket{new tcp::socket(ioService)};


};


using Test = typed_actor<replies_to<BoostError, int>::with<int>>;



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

#endif //SERVER_ATOMMESSAGES_H
