#include "includes.h"
using namespace std;
struct User {
    string name;
    string channel;
    vector<unsigned char> pubkey;
    vector<unsigned char> privkey;
};
User current_user;
rsa_key key;

void process_command(const autobahn::wamp_event& event)
{
    vector<string> arguments;
    while(true)
    cout << "we here now" << endl;
    for(unsigned int i = 0;i < event.number_of_arguments();i++)
    {
        try{
        cout << event.argument<string>(i) << endl;
        arguments.push_back(event.argument<string>(i));
        }catch(const std::exception &e)
        {
            cout << e.what() << endl;
        }
    }
}

int main(void)
{
    ltc_mp = ltm_desc;
    register_prng(&sprng_desc);
    if (rsa_make_key(NULL, find_prng("sprng"), 1536/8, 65537, &key) != CRYPT_OK) {
        return -1;
    }
    unsigned char* public_key = (unsigned char*)malloc(32768);
    memset(public_key,0,1024);
    unsigned long length;
    rsa_export(public_key,&length,PK_PUBLIC,&key);
    current_user.pubkey = vector<unsigned char>(public_key,public_key + length);
    string public_key2;
    for(unsigned int i =0;i < length;i++)
    {
        public_key2.push_back(current_user.pubkey[i]);
    }
    string base64key = base_64_encode(public_key2);
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
                    cout << "Enter your username:" << endl;
                    getline(cin,current_user.name);
                    cout << "com.audioctl." + current_user.name << endl;
                    session->subscribe("com.audioctl." + current_user.name,&process_command);
                    session->publish("com.audioctl.main", std::make_tuple(std::string("NICK"),std::string(current_user.name),base64key));
                });
            });
    });
    io.run();
    return 0;
}
