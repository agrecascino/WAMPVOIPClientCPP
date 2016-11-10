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
struct DataWithSampleTicks
{
    DataWithSampleTicks(int tick,int mintick,vector<short> audio)
    {
        sampletick = tick;
        minutetick = mintick;
        data = audio;
    }

    int sampletick;
    int minutetick;
    vector<short> data;
};
/*
class Mixer {
public:
    Mixer()
    {
        alutInit(0,NULL);
        alGenSources(1,&source);
        signal(SIGALRM,incrementTick);
        ualarm(0,666);
    }
    template <typename T>
    int commitNewDataToStream(vector<T> audiodata, int audiotype)
    {
        if(audiotype == MIXER_AUDIO_16BITS_STEREO)
        {
           streams.push_back(DataWithSampleTicks(sampleticks,minuteticks,vector<short>(audiodata)));
        }
        else
        {
            return -1;
        }
    }
    static void incrementTick(int signal)
    {
        Mixer::sampleticks += 32;
        if(Mixer::sampleticks > 48000)
        {
            Mixer::minuteticks++;
            Mixer::sampleticks = 0;
        }
    }

    int emmitBufferContentsToStream(){
        vector<short> mixed_audio_data;
        mixed_audio_data.reserve(sampleticks + (minuteticks*48000));
        for(DataWithSampleTicks audio : streams)
        {
            vector<short> i = audio.data;
            for(int k = audio.sampletick;k < i.size();k++)
            {
                if(k < mixed_audio_data.size())
                {
                    mixed_audio_data[k] = fmax((int)(mixed_audio_data[k]/2) + (i[k]/2),32768);
                }
                else
                {
                    mixed_audio_data.push_back(i[k]);
                }
            }
        }
        streams.clear();
    }

private:
    static std::atomic<int> minuteticks;
    static std::atomic<int> sampleticks;
    vector<DataWithSampleTicks> streams;
    ALuint buffer, source;
};
Mixer mixer;
*/
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
    RemoteUser(string name_temp,autobahn::wamp_subscription subscription_temp)
    {
        name = name_temp;
        subscription = subscription_temp;

    }

    string name;
    autobahn::wamp_subscription subscription;
};

struct User {
    string name = "not_connected";
    string channel;
    vector<RemoteUser> channelusers;
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
        else if((i == 127) || (i == 8) && str.size() == 0)
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
void audio_encode()
{
    int val;
    while(true)
    {
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &val);
        if(val <= 0)
            continue;
        alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, 1, &val);
        if(val > 1920)
            continue;
        short data[1920];
        alcCaptureStart(device);
        unsigned char packet[4*1276];
        int nbBytes = opus_encode(encoder,data,1920,packet,4*1276);
        vector<unsigned char> packt(packet,packet + nbBytes);

        vector<vector<unsigned char>> packtpackt;
        packtpackt.push_back(packt);
        session->publish("com.audiodata." + current_user.name,packtpackt);
        alcCaptureStop(device);
    }
}

void audio_play(const autobahn::wamp_event& event)
{
    vector<unsigned char> packet;
    try{
        packet = event.argument<vector<unsigned char>>(0);

    }catch(const std::exception &e)
    {
    }
    short output[4*1276];
    //wprintw(vin,"Attempting to play audio...");
    int frame_size = opus_decode(decoder,packet.data(),packet.size(),output,4*1276,0);
    //wprintw(vin, frame_size);
    ALuint buffer, source;
    alGenSources(1,&source);
    alGenBuffers(1,&buffer);

    alBufferData(buffer,AL_FORMAT_STEREO16,output,frame_size,48000);
    alSourceQueueBuffers(source,1,&buffer);
    alSourcePlay(source);

}
volatile int tickctr = 0;
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
                autobahn::wamp_subscription subscription;
                session->subscribe("com.audiodata." + arguments[0][i],&audio_play).then([&] (boost::future<autobahn::wamp_subscription> subscribed)
                {
                    try {
                        subscription = subscribed.get();
                    }
                    catch (const std::exception& e) {
                        io.stop();
                        return;
                    } });
                current_user.channelusers.push_back(RemoteUser(arguments[0][i],subscription));
                session->subscribe("com.audiodata." + arguments[0][i],&audio_play);
                wprintw(vin,string("Channel user: " + arguments[0][i] + "\n").c_str());
                wrefresh(vin);
            }
            return;
        }

        if(arguments[0][1] == "NEWCHANUSER"){

            autobahn::wamp_subscription subscription;
            session->subscribe("com.audiodata." + arguments[0][3],&audio_play).then([&] (boost::future<autobahn::wamp_subscription> subscribed)
            {
                try {
                    subscription = subscribed.get();
                }
                catch (const std::exception& e) {
                    io.stop();
                    return;
                } });
            current_user.channelusers.push_back(RemoteUser(arguments[0][3],subscription));
            session->subscribe("com.audiodata." + arguments[0][3],&audio_play);
            wprintw(vin,string("New channel user: " + arguments[0][3] + "\n").c_str());
            wrefresh(vin);
            return;
        }
        if(arguments[0][1] == "PRUNECHANUSER"){
            for(size_t i = 0;i < current_user.channelusers.size();i++)
                if(current_user.channelusers[i].name == arguments[0][3])
                {
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
    alutInit(0,NULL);
    int err;
    device = alcCaptureOpenDevice(NULL, 48000, AL_FORMAT_STEREO16, 1920);
    ltc_mp = ltm_desc;
    encoder = opus_encoder_create(48000,2,OPUS_APPLICATION_AUDIO,&err);
    decoder = opus_decoder_create(48000,2,&err);
    err = opus_encoder_ctl(encoder,OPUS_SET_BITRATE(48000));
    initscr();
    raw();
    noecho();
    vin = newwin(LINES - 1,COLS,0,0);
    cmd = newwin(1,COLS,LINES - 1,0);
    scrollok(vin, TRUE);
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
    wprintw(vin,string("Enter your WAMP server uri.\n").c_str());
    wrefresh(vin);
    getline(uri,false);
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
            getline(current_user.name,false);
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
                    for(RemoteUser user : current_user.channelusers)
                    {
                        session->unsubscribe(user.subscription);
                    }
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
