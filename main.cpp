#include "includes.h"
using namespace std;
struct Message {
    Message(string ch,string usr,string msg)
    {
        channel = ch;
        name = usr;
        message = msg;
    }

    string channel;
    string name;
    string message;
};

class ChatLogger {
public:
    void addMessage(Message msg)
    {
        messages.push_back(msg);
    }

    void writeOutLinesLambda(size_t lines,function<int(Message)> func)
    {
        if(lines > messages.size())
            lines = messages.size();
        if(messages.size() == 0)
        {
            return;
        }
        size_t l = messages.size() - 1 ;
        do{
            int output_message = func(messages[l]);
            if(output_message)
                wprintw(vin,string("<" + messages[l].name + ">" + " " + messages[l].message + "\n").c_str());
            wrefresh(vin);
            l--;
        }while(l > (messages.size() - 1) - lines);

    }

    void writeOutLines(size_t lines)
    {
        writeOutLinesLambda(lines,[=](Message msg){ return true; });
    }

private:
    vector<Message> messages;
};

struct RemoteUser {
    RemoteUser(string name_temp)
    {
        name = name_temp;
        alGenSources(1,&source);
        //alGenBuffers(2,&buffer[0]);
    }
    string name;
    vector<ALuint> buffer;
    ALuint source;
    ~RemoteUser()
    {
        //alDeleteBuffers(2, &buffer[0]);
        //alDeleteSources(1, &source);
    }
};

struct User {
    string name = "not_connected";
    string channel;
    vector<RemoteUser> channelusers;
    map<string,bool> nodeliver;
    bool nodeliverall = false;
    vector<unsigned char> pubkey;
    vector<unsigned char> privkey;
};
ChatLogger logger;
User current_user;


void getline(string &str, bool display_name = true)
{
    str = "";
    if(display_name)
        wprintw(cmd,string("<" + current_user.name + "> ").c_str());
    while(true)
    {
        int i = wgetch(cmd);
        if(((i == 127) || (i == 8)) && str.size() != 0)
        {
            str.pop_back();
            wdelch(cmd);
            wprintw(cmd,"\b");
            wdelch(cmd);
            wrefresh(cmd);
            continue;
        }
        else if(((i == 127) || (i == 8)) && str.size() == 0)
        {
            continue;
        }
        if(i == '\n')
            break;
        str += i;
        wprintw(cmd,string(1,i).c_str());
        wrefresh(cmd);
    }
    //wprintw(cmd,"\n");
    wclear(cmd);
}

OpusEncoder *encoder;
OpusDecoder *decoder;
ALCdevice *device;
boost::asio::io_service io;
auto session = make_shared<autobahn::wamp_session>(io,false);
void publish_to_channel(string channame,vector<string> arguments)
{
    vector<string> base64_encrypted_arguments;
    for(string arg : arguments)
    {
        unsigned char *encrypted_out = new unsigned char[512];

        unsigned long outlen = 512;

        if(arg.size() >= 181)
        {
            wprintw(vin,"Splitting message.\n");
            vector<string> enc;
            enc.push_back(arg);
            while(enc[enc.size() - 1].size() >= 181)
            {
                enc.push_back("\xffSM");
                enc.push_back(enc[enc.size() - 2].substr(181,string::npos));
                enc[enc.size() - 3] = enc[enc.size() - 3].substr(0,181);
            }
            for(size_t i = 0;i < enc.size();i++)
            {
                if(rsa_encrypt_key_ex((unsigned char*)(void*)enc[i].c_str(),enc[i].size(),encrypted_out,&outlen,NULL,NULL,NULL,0,0,LTC_PKCS_1_V1_5,&serv_pub) != CRYPT_OK)
                {
                    wprintw(vin,"Encryption failed\n");
                    wrefresh(vin);
                }
                string encrypted_rsa(encrypted_out,encrypted_out + outlen);
                enc[i] = base_64_encode(encrypted_rsa);
                outlen = 512;
            }
            base64_encrypted_arguments.insert(base64_encrypted_arguments.end(),enc.begin(),enc.end());
            continue;
        }
        if(rsa_encrypt_key_ex((unsigned char*)(void*)arg.c_str(),arg.size(),encrypted_out,&outlen,NULL,NULL,NULL,0,0,LTC_PKCS_1_V1_5,&serv_pub) != CRYPT_OK)
        {
            wprintw(vin,"Encryption failed\n");
            wrefresh(vin);
        }
        string encrypted_rsa(encrypted_out,encrypted_out + outlen);
        base64_encrypted_arguments.push_back(base_64_encode(encrypted_rsa));
        delete[] encrypted_out;
    }
    session->publish(channame,base64_encrypted_arguments);
}

void infinite_ping_loop()
{
    while(true){
        publish_to_channel("com.audioctl." + current_user.name,vector<string>(1,"PING"));
        this_thread::sleep_for(chrono::seconds(2));
    }
}
thread *t;
thread *t2;
//ALuint buffer, source;
void err(int error_code){
    int error;
    int stack_depth = 0;

    while((error = alGetError()) != AL_NO_ERROR)
    {
            stack_depth++;
            switch(error)
            {
                case AL_INVALID_NAME:
                    wprintw(vin,"Invalid name detected in openAL.\n");
                    break;
                case AL_INVALID_ENUM:
                    wprintw(vin,"Invalid enum detected in openAL.\n");
                    break;
                case AL_INVALID_VALUE:
                    wprintw(vin,"Invalid value detected in openAL\n");
                    break;
                case AL_OUT_OF_MEMORY:
                    wprintw(vin,"Out of memory detected in openAL.\n");
                    break;
                case AL_INVALID_OPERATION:
                    wprintw(vin,"Invalid operation detected in openAL.\n");
                    break;
                default:
                    wprintw(vin,string("Something broke, hex error code: " + string(itoa(error,16)) + string("\n")).c_str());
                    break;
            }
    }
    if(stack_depth > 0)
    {
        wprintw(vin,string("Arbitrary error code: " + string(to_string(error_code)) + string("\n")).c_str());
        wrefresh(vin);
        exit(-1);
    }
}
ALuint s;
vector<ALuint> b;
void audio_encode()
{
    size_t val = 0;
    alGenSources(1,&s);
    short buffer[2880*2];
    alcCaptureStart(device);
    while(true)
    {
        ALint samples;
        again:
        alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, 1, &samples);
        if(samples < 480)
            goto again;


        //ALint sample;
        alcCaptureSamples(device,(ALCvoid *)buffer,480);
        unsigned char packet[2880];
        if(current_user.nodeliverall)
            continue;
        int nbBytes = opus_encode(encoder,(const short *)&buffer,480,packet,2880);
        vector<unsigned char> packt(packet,packet + nbBytes);

        //if(val > oggdec.size())
        //{
        //    val = 0;
        //}
        //ALint state;
        //alGetSourcei(s, AL_BUFFERS_PROCESSED, &state);
        //if(state > 0 && state <= b.size())
        //{
        //    alSourceUnqueueBuffers(s,state,b.data());
        //    alDeleteBuffers(state,&b[0]);
        //    b.erase(b.begin(),b.begin() + state);

        //}
        //b.push_back(ALuint());
        //alGenBuffers(1,&b.back());
        //alBufferData(b.back(),AL_FORMAT_MONO16,&buffer[0],480*2,48000);
        //alSourceQueueBuffers(s,1,&b.back());
        //alSourcePlay(s);
        //alGetSourcei(s,AL_SOURCE_STATE,&state);
        //if(state != AL_PLAYING)
        //    alSourcePlay(s);
        //while(state == AL_PLAYING)
        //{
        //    alGetSourcei(s,AL_SOURCE_STATE,&state);
        //}
        //alDeleteBuffers(1,&b);

        //val += 2880*2;
        vector<vector<unsigned char>> packtpackt;
        packtpackt.push_back(packt);
        for(RemoteUser user : current_user.channelusers)
        {
            if(user.name == current_user.name)
                continue;
            if(current_user.nodeliver[user.name])
                continue;
            session->call("com.audiorpc." + user.name,std::make_tuple(current_user.name,packtpackt));
        }
        //session->publish("com.audiodata." + current_user.name,packtpackt);
        //alcCaptureStop(device);
        //this_thread::sleep_for(chrono::milliseconds(59));
    }
}
bool ftick = false;
int tick = 0;
void audio_play(const autobahn::wamp_invocation& event)
{
    ALint state;
    //mtx.lock();
    string name;
    vector<unsigned char> packet;
    try {
        name = event->argument<string>(0);
        packet = event->argument<vector<vector<unsigned char>>>(1)[0];

    }catch(const std::exception &e) {
        //return;
        exit(-1);
    }
    short output[2880*2];
    int frame_size = opus_decode(decoder,packet.data(),packet.size(),output,480,0);
    //wprintw(vin, frame_size);
    RemoteUser *userptr = NULL;
    for(RemoteUser &user : current_user.channelusers)
        if(user.name == name)
            userptr = &user;
    if(userptr == NULL)
        return;
    //err(tick);
    //
    //mtx.lock();
    alGetSourcei(userptr->source, AL_BUFFERS_PROCESSED, &state);
    //err(tick);
    if(state > 0 && state <= userptr->buffer.size())
    {

        //alSourceStop(userptr->source);
        //alSourcei(userptr->source,AL_BUFFERS_PROCESSED,0);
        alSourceUnqueueBuffers(userptr->source,state,userptr->buffer.data());
        alDeleteBuffers(state,&userptr->buffer[0]);
        userptr->buffer.erase(userptr->buffer.begin(),userptr->buffer.begin() + state);

    }
    //mtx.unlock();
    userptr->buffer.push_back(ALuint());
    alGenBuffers(1,&userptr->buffer.back());
    
    //if(!ftick)
    //    ftick = true;
    //else
    //    alSourceUnqueueBuffers(userptr->source, 1, &userptr->buffer[1]);
    alBufferData(userptr->buffer.back(),AL_FORMAT_MONO16,output,960,48000);
    alSourceQueueBuffers(userptr->source,1,&userptr->buffer.back());
    alGetSourcei(userptr->source, AL_SOURCE_STATE, &state);
    if(tick == 1)
    {
        alSourcePlay(userptr->source);
    }
    if(state != AL_PLAYING && tick > 1)
    {
        //wprintw(vin,"Restarting source.\n");
        //wrefresh(vin);
        alSourcePlay(userptr->source);
    }
    //
    tick++;
}

void process_command(const autobahn::wamp_event& event)
{
    //tickctr++;
    //wprintw(vin,itoa(tickctr,10));
    //wprintw(vin,"\n");
    //wrefresh(vin);
    vector<vector<string>> arguments;
    for(unsigned int i = 0;i < event.number_of_arguments();i++)
    {
        try{
            arguments.push_back(event.argument<vector<string>>(0));

        }catch(const std::exception &e)
        {
            //TODO: crash and burn here
            wprintw(vin,"Error converting to string.\nYou really shouldn't see this. Exiting immediately!!!!!!!!1\n");
            wrefresh(vin);
            exit(-1);
        }

    }
    if(arguments[0][0] == "~"){

        if(arguments[0][1] == "PUBKEY"){
            string base64publickey = arguments[0][2];
            unsigned char *nonb64key = new unsigned char[4096];
            unsigned long k64len = 4096;
            base64_decode((unsigned char*)(void*)base64publickey.c_str(),base64publickey.size(),nonb64key,&k64len);
            rsa_import(nonb64key,k64len,&serv_pub);
            //wprintw(vin, string(base64publickey  + "\n").c_str());
            wprintw(vin,"Connected to instance of audioserver.\n");
            wrefresh(vin);
            t2 = new thread(audio_encode);
            t = new thread(infinite_ping_loop);
            t->detach();
            t2->detach();
            session->provide("com.audiorpc." + current_user.name,audio_play);
            delete[] nonb64key;
        }
    }
    else{
        for(size_t i = 0;i < arguments[0].size();i++){
            string argument = base_64_decode(arguments[0][i]);
            unsigned long outlen = 512;
            unsigned char *output = new unsigned char[512];
            int val = 0;
            //wprintw(vin,"Decrypting...\n");
            //wrefresh(vin);
            rsa_decrypt_key_ex((unsigned char*)(void*)argument.c_str(),argument.size(),output,&outlen,NULL,NULL,0,LTC_PKCS_1_V1_5,&val,&key);
            arguments[0][i] = string(output,output + outlen);
            //wprintw(vin,"Decrypted.\n");
            //wrefresh(vin);
        }
    }
    for(size_t i = 0;i < arguments[0].size();i++)
    {
        //wprintw(vin,string(arguments[0][i] + "\n").c_str());
        //wrefresh(vin);
        if(arguments[0][i] == "\xffSM")
        {
            if((((int)i - 1) == -1) || (((int)i + 1) >= arguments[0].size()))
            {
                wprintw(vin,"Message corrupt.\n");
                wrefresh(vin);
                return;
            }
            arguments[0][i - 1] = arguments[0][i - 1] + arguments[0][i + 1];
            arguments[0].erase(arguments[0].begin() + (i + 1));
            arguments[0].erase(arguments[0].begin() + i);
        }

    }
    if(arguments[0][0] == ":"){
        //wprintw(vin,"Entered response block.\n");
        //wrefresh(vin);
        if(arguments[0][1] == "CHANUSERNAMES"){
            for(size_t i = 3; i < arguments[0].size();i++)
            {
                current_user.channelusers.push_back(RemoteUser(arguments[0][i]));
                wprintw(vin,string("Channel user: " + arguments[0][i] + "\n").c_str());
                wrefresh(vin);
            }
            return;
        }

        if(arguments[0][1] == "NEWCHANUSER"){


            current_user.channelusers.push_back(RemoteUser(arguments[0][3]));
            wprintw(vin,string("[+] " + arguments[0][3] + "\n").c_str());
            wrefresh(vin);
            return;
        }
        if(arguments[0][1] == "PRUNECHANUSER"){
            for(size_t i = 0;i < current_user.channelusers.size();i++)
                if(current_user.channelusers[i].name == arguments[0][3])
                {
                    wprintw(vin,string("[-] " + arguments[0][3] + "\n").c_str());
                    wrefresh(vin);
                    current_user.channelusers.erase(current_user.channelusers.begin() + i);
                }
            return;
        }
        if(arguments[0][1] == "MESSAGE"){
            //wprintw(vin,"Entered message block.\n");
            //wrefresh(vin);
            logger.addMessage(Message(arguments[0][3],arguments[0][2],arguments[0][4]));
            logger.writeOutLines(1);
            return;

        }
        if(arguments[0][1] == "NODELIVER")
        {
            wprintw(vin,string(arguments[0][2] + " has muted themselves from you.\n").c_str());
            wrefresh(vin);
            return;
        }
        if(arguments[0][1] == "CHANNAMES"){
            for(size_t i = 2; i <arguments[0].size();i++)
            {
                wprintw(vin,string("Channel: " + arguments[0][i] + "\n").c_str());
                wrefresh(vin);
            }
            return;
        }
    }
    else{

    }


}

int main(void)
{
    srand(time(NULL));
    alutInit(0,NULL);
    vfile = fopen("x14_haista_poks.ogg","rb");
    ov_open_callbacks(vfile,&vf,NULL,0,OV_CALLBACKS_DEFAULT);
    char arr[4096];
    int bytes = 0;
    do {

      // Read up to a buffer's worth of decoded sound data

      bytes = ov_read(&vf, arr, 4096, 0, 2, 1, NULL);

      // Append to end of buffer

      oggdec.insert(oggdec.end(), arr, arr + bytes);

    } while (bytes > 0);
    //opusfile = op_open_file("fabetik.opus",NULL);
    int err;
    device = alcCaptureOpenDevice(NULL, 48000, AL_FORMAT_MONO16, 480);
    ltc_mp = ltm_desc;
    encoder = opus_encoder_create(48000,1,OPUS_APPLICATION_AUDIO,&err);
    decoder = opus_decoder_create(48000,1,&err);
    err = opus_encoder_ctl(encoder,OPUS_SET_BITRATE(48000));
    initscr();
    raw();
    noecho();
    vin = newwin(LINES - 1,COLS,0,0);
    cmd = newwin(1,COLS,LINES - 1,0);
    //dbg = newwin(1,COLS,0,0);
    scrollok(vin, TRUE);
    //start_color();
    //init_pair(0, COLOR_RED, COLOR_BLUE);
    //attron(0);
    wrefresh(vin);
    register_prng(&sprng_desc);
    if (rsa_make_key(NULL, find_prng("sprng"), 1536/8, 65537, &key) != CRYPT_OK) {
        return -1;
    }
    fclose(stderr);
    unsigned char* public_key = new unsigned char[512];
    memset(public_key,0,512);
    unsigned long length = 512;
    rsa_export(public_key,&length,PK_PUBLIC,&key);
    current_user.pubkey = vector<unsigned char>(public_key,public_key + length);
    string public_key2;
    for(unsigned int i =0;i < length;i++)
    {
        public_key2.push_back(current_user.pubkey[i]);
    }
    string base64key = base_64_encode(public_key2);
    delete[] public_key;
    client ws_client;
    ws_client.init_asio(&io);
    string uri;
    bool testmode= false;
    wprintw(vin,string("Enter your WAMP server uri.\n").c_str());
    wrefresh(vin);
    getline(uri,false);
    if(uri == "test")
    {
        uri = "ws://127.0.0.1:8080/ws";
        testmode = true;
    }
    auto transport = make_shared<autobahn::wamp_websocketpp_websocket_transport<websocketpp::config::asio_client>>(ws_client,uri,false);

    transport->attach(static_pointer_cast<autobahn::wamp_transport_handler>(session));

    boost::future<void> connect_future;
    boost::future<void> start_future;
    boost::future<void> join_future;

    connect_future = transport->connect().then([&](boost::future<void> connected){
            try {
            connected.get();
            wprintw(vin,"Connected to crossbar server.\n");
            wrefresh(vin);
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

}catch (const exception& e) {
            io.stop();
            return;
}
            wprintw(vin,"Enter your username:\n");
            wrefresh(vin);
            if(testmode)
                goto coolkids;
            getline(current_user.name,false);
            goto dosomething;
            coolkids:
            current_user.name = "test" + string(itoa(rand() % 20000,10));
            dosomething:
            session->subscribe("com.audioctl." + current_user.name,&process_command);
            session->publish("com.audioctl.main", std::make_tuple(std::string("NICK"),std::string(current_user.name),base64key));
            wclear(vin);
            while(true)
    {
            string command;
            getline(command);
            if(command.size() == 0)
            continue;
            if(command[0] == '/')
    {
        vector<string> cmd;
        split_string(command," ",cmd);
        cmd[0] = remove_erase_if(cmd[0],"/");
        //wprintw(vin,string(cmd[0] + "\n").c_str());
        if(cmd[0] == "quit")
        {
            publish_to_channel("com.audioctl." + current_user.name,vector<string>(1,"QUIT"));
            endwin();
            exit(-1);
        }
        else if(cmd[0] == "listchannels")
        {
            publish_to_channel("com.audioctl." + current_user.name,vector<string>(1,"CHANNAMES"));
            continue;
        }
        if(cmd.size() >= 2)
        {
            vector<string> send_to_client;
            if(cmd[0] == "mkchannel")
            {
                send_to_client.push_back("MKCHANNEL");
                send_to_client.push_back(cmd[1]);
                publish_to_channel("com.audioctl." + current_user.name,send_to_client);
                continue;
            }
            else if(cmd[0] == "joinchannel")
            {
                if(current_user.channel == "")
                {
                    send_to_client.push_back("JOINCHANNEL");
                    send_to_client.push_back(cmd[1]);
                    current_user.channel = cmd[1];
                    publish_to_channel("com.audioctl." + current_user.name,send_to_client);
                }
                else
                {
                    wprintw(vin,"You need to leave your channel first.\n");
                    wrefresh(vin);
                }
                continue;
            }
            else if(cmd[0] == "leavechannel")
            {
                if(current_user.channel != "")
                {
                    send_to_client.push_back("LEAVECHANNEL");
                    send_to_client.push_back(cmd[1]);
                    current_user.channel = "";
                    current_user.channelusers.clear();
                    publish_to_channel("com.audioctl." + current_user.name,send_to_client);
                }
                else
                {
                    wprintw(vin,"You need to be in a channel.\n");
                    wrefresh(vin);
                }
                continue;
            }
            else if(cmd[0] == "nodeliver")
            {
                wprintw(vin,string("Stopped sending audio to user: " + cmd[1] + "\n").c_str());
                wrefresh(vin);
                current_user.nodeliver[cmd[1]] = !current_user.nodeliver[cmd[1]];
                vector<string> s;
                s.push_back("NODELIVER");
                s.push_back(cmd[1]);
                publish_to_channel("com.audioctl." + current_user.name,s);
            }
            else if(cmd[0] == "nodeliverall")
            {
                wprintw(vin,"Muted self.\n");
                wrefresh(vin);
                current_user.nodeliverall = !current_user.nodeliverall;
                vector<string> s;
                s.push_back("NODELIVERALL");
                publish_to_channel("com.audioctl." + current_user.name,s);
            }
        }
    }
    else
    {
        if(current_user.channel != "")
        {
            vector<string> send_to_server;
            send_to_server.push_back("MESSAGE");
            send_to_server.push_back(current_user.channel);
            send_to_server.push_back(command);
            publish_to_channel("com.audioctl." + current_user.name,send_to_server);
            logger.addMessage(Message(current_user.channel,current_user.name,command));
            logger.writeOutLines(1);
            continue;
        }
    }
}
});
});
});
io.run();
return 0;
}
