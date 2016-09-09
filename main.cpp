#include <iostream>
#include "includes.h"
using namespace std;
struct User {
    string name;
    string channel;
};
User current_user;
int main(int argc, char *argv[])
{
    boost::asio::io_service io;
    client ws_client;
    ws_client.init_asio(&io);
    string uri;
    cout << "Enter your WAMP server uri." << endl;
    getline(cin,uri);
    auto transport = make_shared<autobahn::wamp_websocketpp_websocket_transport<websocketpp::config::asio_client>>(ws_client,uri,false);
    auto session = make_shared<autobahn::wamp_session>(io,false);
    transport->attach(static_pointer_cast<autobahn::wamp_transport_handler>(session));

    boost::future<void> connect_future;
    boost::future<void> start_future;
    boost::future<void> join_future;
    boost::future<void> provide_future;

    connect_future = transport->connect().then([&](boost::future<void> connected){
            try {
                connected.get();
            } catch(const exception& e) {
                cerr << e.what() << endl;
                io.stop();
                return;
            }

            cout << "Connected to crossbar server." << endl;
            start_future = session->start().then([&](boost::future<void> started) {
                try {
                    started.get();
                }catch(const exception& e) {
                    cerr << e.what() << endl;
                    io.stop();
                    return;
                }

                join_future = session->join("realm1").then([&](boost::future<uint64_t> joined){
                    try {
                        cerr << "joined realm:" << joined.get() << endl;

                    }catch (const exception& e) {
                      cerr << e.what() << endl;
                      io.stop();
                      return;
                    }
                    getline(cin,current_user.name);
                });
            });
    });
    io.run();
    return 0;
}
