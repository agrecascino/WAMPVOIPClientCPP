#include <reactor.h>

Reactor::Reactor(string url, string username, int reactoridtmp, std::map<string, bool> options) : logger(display,reactoridtmp){
    reactorid = reactoridtmp;
    display.create_screen("reactor" + to_string(reactorid) + "_main");
    user.name = username;
    this->uri = url;
    int err;
    device = alcCaptureOpenDevice(NULL, 48000, AL_FORMAT_MONO16, 1920);
    encoder = opus_encoder_create(48000,1,OPUS_APPLICATION_AUDIO,&err);
    decoder = opus_decoder_create(48000,1,&err);
    err = opus_encoder_ctl(encoder,OPUS_SET_BITRATE(48000));

}

Reactor::Reactor(Reactor &&other) : logger(display,reactorid) {
    this->reactorid = other.reactorid;
    //this->io = std::move(other.io);
    this->reactormail = std::move(other.reactormail);
    this->decoder = other.decoder;
    this->device = other.device;
    this->display = other.display;
    this->encoder = other.encoder;
    this->io_thread = std::move(other.io_thread);
    this->logger = other.logger;
    this->old_time = other.old_time;
    this->session = other.session;
    this->time = other.time;
    this->user = other.user;

}

void Reactor::start() {
    active = true;
    display.switch_screen("reactor" + to_string(reactorid) + "_main",true);
    display.print_to_screen("chat","Starting reactor: " + to_string(reactorid) + "\n");
    io_thread = thread(&Reactor::eventloop,this);
    io_thread.detach();
    this_thread::sleep_for(chrono::milliseconds(230));
}

void Reactor::eventloop() {
    client ws_client;
    ws_client.init_asio(&io);
    auto transport = make_shared<autobahn::wamp_websocketpp_websocket_transport<websocketpp::config::asio_client>>(ws_client,uri,false);
    transport->attach(static_pointer_cast<autobahn::wamp_transport_handler>(session));
    boost::future<void> connect_future;
    boost::future<void> start_future;
    boost::future<void> join_future;
    connect_future = transport->connect().then([&](boost::future<void> connected){
            try {
            connected.get();
} catch(const exception& e) {
            io.stop();
            return;
}
            start_future = session->start().then([&](boost::future<void> started) {
            try {
            started.get();
}catch(const exception& e) {

            io.stop();
            return;
}

            join_future = session->join("realm1").then([&](boost::future<uint64_t> joined){
            try {
                userid = joined.get();
}catch (const exception& e) {
            io.stop();
            return;
}
            bool ready = false;
            display.print_to_screen("chat","crossbar\n");
            session->subscribe("com.audiomain",[&](const autobahn::wamp_event &event){vector<vector<string>> arg; arg.push_back(event.argument<vector<string>>(0)); if(arg[0][0] == ":" && arg[0][1] == "READY" && arg[0][2] == to_string(userid)){ready = true;display.print_to_screen("chat","ready\n");}});
            session->publish("com.audiomain", std::make_tuple(std::string("NICK"),std::string(user.name)));
            while(!ready) {}
            gettimeofday(&old_time,NULL);
            session->subscribe("com.audioctl." + user.name,std::bind(&Reactor::message_handler,this,std::placeholders::_1));
            session->publish("com.audiomain", std::make_tuple(std::string("PING")));
            display.print_to_screen("chat","ping\n");
            while(true) {
            if(!reactormail.isEmpty()){
            if(internal_message_handler(reactormail.getMessage())) {
                exit(-1);

}
}
            gettimeofday(&time,NULL);
            if(((time.tv_sec * 1000) - (old_time.tv_sec * 1000)) + (((float)time.tv_usec / 1000) - ((float)old_time.tv_usec / 1000)) > 3000)
    {
            publish_message("com.audioctl." + user.name,vector<string>(1,"PING"));
            gettimeofday(&old_time,NULL);
}
            audio_dispatcher();
            this_thread::sleep_for(chrono::milliseconds(2));
}


});
});
});
io.run();

}

int Reactor::internal_message_handler(string s) {
    if(s[0]){
        vector<string> cmd;
        split_string(s," ",cmd);
        cmd[0] = remove_erase_if(cmd[0],"/");
        if(user.channel.size() != 0 && s[0] != '/')
        {
            vector<string> send_to_server;
            send_to_server.push_back("MESSAGE");
            send_to_server.push_back(user.screen_channel);
            send_to_server.push_back(s);
            publish_message("com.audioctl." + user.name,send_to_server);
            logger.add_message(Message(user.screen_channel,user.name,s));
            logger.write_out_lines(1);
            return 0;
        }
        //wprintw(vin,string(cmd[0] + "\n").c_str());
        if(cmd[0] == "quit")
        {
            publish_message("com.audioctl." + user.name,vector<string>(1,"QUIT"));
            endwin();
            return -1;
        }
        else if(cmd[0] == "listchannels")
        {
            publish_message("com.audioctl." + user.name,vector<string>(1,"CHANNAMES"));
            return 0;
        }
        else if(cmd[0] == "switchscreen")
        {
            int ret = display.switch_screen(cmd[1]);
            if(ret == -1) {
                display.print_to_screen("chat","You either do not own this screen, or it doesn't exist, try switching reactors.");
            }
            return 0;
        }
        else if(cmd[0] == "UNACTIVE") {
            active = !active;
            if(!active) {
                user.screen_channel = "";
            }
        }
        if(cmd.size() >= 2)
        {
            vector<string> send_to_client;
            if(cmd[0] == "mkchannel")
            {
                send_to_client.push_back("MKCHANNEL");
                send_to_client.push_back(cmd[1]);
                publish_message("com.audioctl." + user.name,send_to_client);
                return 0;
            }
            else if(cmd[0] == "joinchannel")
            {
                bool channel_exists = false;
                for(string channel : user.channel)
                    if(channel == cmd[1])
                        channel_exists = true;
                if(!channel_exists) {
                    send_to_client.push_back("JOINCHANNEL");
                    send_to_client.push_back(cmd[1]);
                    publish_message("com.audioctl." + user.name,send_to_client);
                }
                else
                {
                    display.print_to_screen("chat","You are in this channel.\n");
                }
                return 0;
            }
            else if(cmd[0] == "leavechannel")
            {
                bool channel_exists = false;
                for(string channel : user.channel)
                    if(channel == cmd[1])
                        channel_exists = true;
                if(channel_exists) {
                    send_to_client.push_back("LEAVECHANNEL");
                    send_to_client.push_back(cmd[1]);
                    user.channel.erase(std::remove(user.channel.begin(),user.channel.end(),cmd[1]),user.channel.end());
                    user.channelusers[cmd[1]].clear();
                    publish_message("com.audioctl." + user.name,send_to_client);
                    if(user.screen_channel == cmd[1])
                    {
                        display.switch_screen("reactor" + to_string(reactorid) + "_" + "main");
                        user.screen_channel = "";
                    }
                    display.destroy_screen("reactor" + to_string(reactorid) + "_" + cmd[1]);
                }
                else
                {
                    display.print_to_screen("chat","You aren't in this channel.\n");
                }
                return 0;
            }
            else if(cmd[0] == "switchchannel") {
                bool channel_exists = false;
                for(string channel : user.channel)
                    if(channel == cmd[1])
                        channel_exists = true;
                if(channel_exists) {
                    user.screen_channel = cmd[1];
                    display.switch_screen("reactor" + to_string(reactorid) + "_" + cmd[1]);
                }
                return 0;
            }
        }
    }
}
void Reactor::publish_message(string channel_name, vector<string> arguments) {
    session->publish(channel_name,arguments);
}

void Reactor::message_handler(const autobahn::wamp_event &event) {
    vector<vector<string>> arguments;
    for(unsigned int i = 0;i < event.number_of_arguments();i++)
    {
        try {
            arguments.push_back(event.argument<vector<string>>(0));

        } catch(const std::exception &e) {
            //TODO: crash and burn here
            display.print_to_screen("chat","Something broke.");
            exit(-1);
        }

    }
    if(arguments[0][0] == ":"){
        //wprintw(vin,"Entered response block.\n");
        //wrefresh(vin);
        if(arguments[0][1] == "HELLO"){

            display.print_to_screen("chat","Connected to instance of audioserver.\n");
            session->provide("com.audiorpc." + user.name,std::bind(&Reactor::audio_rpc_handler,this,std::placeholders::_1));
            return;
        }

        if(arguments[0][1] == "CHANUSERNAMES"){
            for(size_t i = 3; i < arguments[0].size();i++)
            {
                user.channelusers[arguments[0][2]].push_back(RemoteUser(arguments[0][i]));
                display.print_to_screen("users",arguments[0][i] + "\n","reactor" + to_string(reactorid) + "_" + arguments[0][2]);
            }
            return;
        }

        if(arguments[0][1] == "NEWCHANUSER"){

            user.channelusers[arguments[0][2]].push_back(RemoteUser(arguments[0][3]));
            display.print_to_screen("chat","[+] " + arguments[0][3] + "\n","reactor" + to_string(reactorid) + "_" + arguments[0][2]);
            display.print_to_screen("users",arguments[0][3] + "\n","reactor" + to_string(reactorid) + "_" + arguments[0][2]);
            return;
        }
        if(arguments[0][1] == "JOINCHANNEL") {
            display.create_screen("reactor" + to_string(reactorid) + "_" + arguments[0][2]);
            display.switch_screen("reactor" + to_string(reactorid) + "_" + arguments[0][2]);
            display.print_to_screen("chat","[status] Joined channel " + arguments[0][2] + "\n","reactor" + to_string(reactorid) + "_" + arguments[0][2]);
            user.screen_channel = arguments[0][2];
            user.channel.push_back(arguments[0][2]);

        }
        if(arguments[0][1] == "PRUNECHANUSER"){
            for(size_t i = 0;i < user.channelusers[arguments[0][2]].size();i++)
                if(user.channelusers[arguments[0][2]][i].name == arguments[0][3])
                {
                    user.channelusers[arguments[0][2]].erase(user.channelusers[arguments[0][2]].begin() + i);
                }
            display.print_to_screen("chat","[-] " + arguments[0][3] + "\n","reactor" + to_string(reactorid) + "_" + arguments[0][2]);
            display.clear_screen("users","reactor" + to_string(reactorid) + "_" + arguments[0][2]);
            for(RemoteUser remoteuser : user.channelusers[arguments[0][2]])
                display.print_to_screen("users",remoteuser.name,"reactor" + to_string(reactorid) + "_" + arguments[0][2] + "\n");
            return;
        }
        if(arguments[0][1] == "MESSAGE"){
            //wprintw(vin,"Entered message block.\n");
            //wrefresh(vin);
            logger.add_message(Message(arguments[0][3],arguments[0][2],arguments[0][4]));
            logger.write_out_lines(1);
            return;
        }
        if(arguments[0][1] == "CHANNAMES"){
            display.print_to_screen("chat","[status] Listing channels on the server.\n");
            for(size_t i = 2; i <arguments[0].size();i++)
            {
                display.print_to_screen("chat","[status] Channel: " + arguments[0][i] + "\n");
            }
            return;
        }

        if(arguments[0][1] == "ERR") {
            if(arguments[0][2] == "JOIN_CHANNOTFOUND") {
                string s = display.get_current_panel();
                if(s == arguments[0][3]) {
                    display.switch_screen("reactor" + to_string(reactorid) + "_main");
                }
                display.destroy_screen("reactor" + to_string(reactorid) + "_" + arguments[0][3]);
            }
        }
    }
    else{

    }

}

void Reactor::audio_rpc_handler(autobahn::wamp_invocation i) {
    ALint state;
    string name;
    vector<unsigned char> packet;
    try {
        name = i->argument<string>(0);
        packet = i->argument<vector<vector<unsigned char>>>(1)[0];
    }catch(const std::exception &e) {
        return;
    }
    short output[2880*2];
    int frame_size = opus_decode(decoder,packet.data(),packet.size(),output,1920,0);
    RemoteUser *userptr = NULL;
    for(auto& kv : user.channelusers)
        for(RemoteUser user : kv.second)
            if(user.name == name)
                userptr = &user;
    if(userptr == NULL)
        return;
    alGetSourcei(userptr->source, AL_BUFFERS_PROCESSED, &state);
    if(state > 0 && state <= userptr->buffer.size())
    {
        alSourceUnqueueBuffers(userptr->source,state,userptr->buffer.data());
        alDeleteBuffers(state,&userptr->buffer[0]);
        userptr->buffer.erase(userptr->buffer.begin(),userptr->buffer.begin() + state);
    }
    userptr->buffer.push_back(ALuint());
    alGenBuffers(1,&userptr->buffer.back());
    alBufferData(userptr->buffer.back(),AL_FORMAT_MONO16,output,3840,48000);
    alSourceQueueBuffers(userptr->source,1,&userptr->buffer.back());
    alGetSourcei(userptr->source, AL_SOURCE_STATE, &state);
    /* Double buffers audio
        if(tick == 1)
        {
            alSourcePlay(userptr->source);
        } else*/
    if(state != AL_PLAYING /*&& tick > 1*/)
    {
        alSourcePlay(userptr->source);
    }
    //tick++;
}

void Reactor::audio_dispatcher() {
    short buffer[2880*2];
    if(!sendaudio) {
        alcCaptureStart(device);
        sendaudio = true;
    }
    ALint samples;
again:
    alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, 1, &samples);
    if(samples < 1920)
        return;
    alcCaptureSamples(device,(ALCvoid *)buffer,1920);
    unsigned char packet[2880*2];
    int nbBytes = opus_encode(encoder,(const short *)&buffer,1920,packet,2880*2);
    vector<unsigned char> packt(packet,packet + nbBytes);
    vector<vector<unsigned char>> packtpackt;
    packtpackt.push_back(packt);
    for(auto& kv : user.channelusers)
        for(RemoteUser remote_user : kv.second)
        {
            if(remote_user.name == user.name)
                continue;
            session->call("com.audiorpc." + user.name,std::make_tuple(user.name,packtpackt));
        }
}
