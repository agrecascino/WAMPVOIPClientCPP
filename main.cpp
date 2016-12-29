#include "includes.h"
#include <reactor.h>
using namespace std;
//ALuint buffer, source;
bool ftick = false;
int tick = 0;
class ReactorManager {
    public:
    ReactorManager() {
        abstractor.print_to_screen("chat","[status] Welcome to WAMPVOIP!\n[status] Use /mkreactor to join a server.\n");
    }
    int new_reactor(string uri, string username)
    {
        Reactor* temporary = new Reactor(uri,username,next_reactorid);
        r.push_back(temporary);
        r.back()->start();
        if(next_reactorid == 0) {
            active_reactor = 0;
        }
        next_reactorid++;
        //io.run();
        return 0;
    }
    int event_loop()
    {
        while(true) {
            string line = abstractor.get_line();
            if(line.substr(0,14) == "/switchreactor") {
                vector<string> data;
                split_string(line," ",data);
                if(data.size() == 2) {
                    for(Reactor *reactor : r) {
                        if(reactor->reactorid == active_reactor) {
                            Mail::Message<string> msg = Mail::Message<string>("/UNACTIVE");
                            reactor->reactormail.stuffMessage(msg);
                        }
                    }
                    active_reactor = atoi(data[1].c_str());
                    abstractor.switch_screen("reactor" + to_string(active_reactor) + "_main",true);
                    abstractor.print_to_screen("chat","Switched to reactor: " + to_string(active_reactor) + "\n");
                    for(Reactor *reactor : r) {
                        if(reactor->reactorid == active_reactor) {
                            Mail::Message<string> msg = Mail::Message<string>("/UNACTIVE");
                            reactor->reactormail.stuffMessage(msg);
                        }
                    }
                } else {
                    abstractor.print_to_screen("chat","Invalid syntax.\n");
                }
                continue;
            }
            else if(line.substr(0,10) == "/mkreactor") {
                vector<string> data;
                split_string(line," ",data);
                if(data.size() == 3) {
                    abstractor.print_to_screen("chat","Creating reactor: " + to_string(next_reactorid) + "\n");
                    new_reactor(data[1],data[2]);
                } else {
                    abstractor.print_to_screen("chat","Invalid syntax.\n");
                }
                continue;
            }
            for(Reactor *reactor : r) {
                if(reactor->reactorid == active_reactor) {
                    //reactor->internal_message_handler(line);
                    Mail::Message<string> msg = Mail::Message<string>(line);
                    reactor->reactormail.stuffMessage(msg);
                }
            }
        }
    }
    private:
    boost::asio::io_service::work work = boost::asio::io_service::work(io);
    DisplayAbstractor abstractor;
    int next_reactorid = 0;
    vector<Reactor*> r;
    int active_reactor = -1;

};
//std::thread t([&](){io.run();});
int main(void)
{
    srand(time(NULL));
    alutInit(0,NULL);
    //t.detach();
    ReactorManager reactman;
    reactman.event_loop();
    return 0;
}
