#include "includes.h"
#include <reactor.h>
using namespace std;
//ALuint buffer, source;
bool ftick = false;
int tick = 0;
class ReactorManager {
    public:
    ReactorManager() {
    }
    int new_reactor(string uri, string username)
    {
        Reactor *temporary = new Reactor(uri,username,next_reactorid);
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
        new_reactor("ws://127.0.0.1:8080/ws","agrecascino3");
        while(true) {
            string line = abstractor.get_line();
            //if(line.substr(0,11) == "/switchreactor") {
            //    vector<string> data;
            //    split_string(line," ",data);
            //    active_reactor = atoi(data[1].c_str());
            //    abstractor.switch_screen("reactor" + to_string(active_reactor) + "_main");
            //    abstractor.print_to_screen("chat","Switched reactor.");
//
  //          }
    //        else if(line.substr(0,9) == "/mkreactor") {
      //          new_reactor("ws://127.0.0.1:8080/ws","agrecascino");
        //    }
            for(Reactor *reactor : r) {
                if(reactor->reactorid == active_reactor) {
                    reactor->internal_message_handler(line);
                    //Mail::Message<string> msg = Mail::Message<string>(line);
                    //reactor->reactormail.stuffMessage(msg);
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
    ltc_mp = ltm_desc;
    register_prng(&sprng_desc);
    //t.detach();
    ReactorManager reactman;
    reactman.event_loop();
    return 0;
}
