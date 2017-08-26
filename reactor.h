#ifndef REACTOR_H
#define REACTOR_H
#include <abstractor.h>
#include <includes.h>
#include <structures.h>
#include <message.h>
#include <chatlogger.h>

using namespace std;

class Reactor {
    public:
    Reactor(string uri, string username, int reactoridtmp, std::map<string,bool> options = map<string,bool>());

    Reactor(Reactor&& other);

    void audio_rpc_handler(autobahn::wamp_invocation i);

    void audio_packet_handler(string name, vector<unsigned char> packet);

    void start();

    void eventloop();

    void publish_message(string channel_name,vector<string> arguments);

    void message_handler(const autobahn::wamp_event &e);

    int internal_message_handler(string s);

    void audio_dispatcher();

    Mail::Mailbox<string> reactormail;

    int reactorid;

    private:
    sockaddr_in addr;
    int media_socket;
    unsigned long userid;
    bool active;
    bool sendaudio = false;
    ALCdevice *device;
    OpusEncoder *encoder;
    OpusDecoder *decoder;
    std::shared_ptr<autobahn::wamp_session> session = make_shared<autobahn::wamp_session>(io,false);
    User user;
    string uri;
    timeval time, old_time;
    DisplayAbstractor display;
    ChatLogger logger;
    std::thread io_thread;
};

#endif // REACTOR_H
