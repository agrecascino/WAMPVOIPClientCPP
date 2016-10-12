#include "includes.h"
using namespace std;
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
    string name;
    string channel;
    vector<RemoteUser> channelusers;
    vector<unsigned char> pubkey;
    vector<unsigned char> privkey;
};
User current_user;
OpusEncoder *encoder;
OpusDecoder *decoder;
PaStream *stream;
boost::asio::io_service io;
auto session = make_shared<autobahn::wamp_session>(io,false);
void publish_to_channel(string channame,vector<string> arguments)
{
    vector<string> base64_encrypted_arguments;
    for(string arg : arguments)
    {
        unsigned char *encrypted_out = new unsigned char[512];

        unsigned long outlen = 512;

        if(rsa_encrypt_key_ex((unsigned char*)(void*)arg.c_str(),arg.size(),encrypted_out,&outlen,NULL,NULL,NULL,0,0,LTC_PKCS_1_V1_5,&serv_pub) != CRYPT_OK)
        {
            cout << "Encryption failed" << endl;
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
        this_thread::sleep_for(chrono::seconds(4));
    }
}
thread *t;
thread *t2;
void audio_encode()
{
     while(true)
     {
     short data[512];
     Pa_ReadStream(stream,&data,512);
     unsigned char packet[3*1276];
     int nbBytes = opus_encode(encoder,data,512,packet,3*1276);
     vector<unsigned char> packt(packet,packet + 512);
     session->publish("com.audiodata." + current_user.name,packt);
     }
}

void audio_play(const autobahn::wamp_event& event)
{
    vector<unsigned char> packet;
    try{
        packet = event.argument<vector<unsigned char>>(0);

    }catch(const std::exception &e)
    {
        cout << e.what() << endl;
    }
    short output[3*1276];
    int frame_size = opus_decode(decoder,packet.data(),packet.size(),output,3*1276,0);
    Pa_WriteStream(stream,&output,frame_size);
}
void process_command(const autobahn::wamp_event& event)
{

    vector<vector<string>> arguments;
       for(unsigned int i = 0;i < event.number_of_arguments();i++)
       {
           try{
               arguments.push_back(event.argument<vector<string>>(0));

           }catch(const std::exception &e)
           {
               cout << e.what() << endl;
           }

       }
           if(arguments[0][0] == "~"){

               if(arguments[0][1] == "PUBKEY"){
                   string base64publickey = arguments[0][2];
                   unsigned char *nonb64key = new unsigned char[4096];
                   unsigned long k64len = 4096;
                   base64_decode((unsigned char*)(void*)base64publickey.c_str(),base64publickey.size(),nonb64key,&k64len);
                   rsa_import(nonb64key,k64len,&serv_pub);
                   cout << base64publickey << endl;
                   t2 = new thread(audio_encode);
                   t = new thread(infinite_ping_loop);
                   t->detach();
               }
           }
               else{
                   for(int i = 0;i < arguments[0].size();i++){
                       string argument = base_64_decode(arguments[0][i]);
                       unsigned long outlen = 512;
                       unsigned char *output = new unsigned char[512];
                       int val = 0;

                       rsa_decrypt_key_ex((unsigned char*)(void*)argument.c_str(),argument.size(),output,&outlen,NULL,NULL,0,LTC_PKCS_1_V1_5,&val,&key);
                       arguments[0][i] = string(output,output + outlen);
                   }
               }
               if(arguments[0][0] == ":"){
                    if(arguments[0][1] == "CHANUSERNAMES"){
                        for(int i = 2; i <arguments[0].size();i++)
                        {
                            boost::future<void> subscribe_future = session->subscribe("com.audiodata." + arguments[0][i],&audio_play).then([&] (boost::future<autobahn::wamp_subscription> subscribed)
                            {
                                try {
                                    current_user.channelusers.push_back(RemoteUser(arguments[0][i],subscribed.get()));
                                    session->subscribe("com.audiodata." + arguments[0][i],&audio_play);
                                    cout << "Channel user: " << arguments[0][i] << endl;
                                }
                                catch (const std::exception& e) {
                                    std::cerr << e.what() << std::endl;
                                    io.stop();
                                    return;
                            } });
                        }
                    }
                    if(arguments[0][1] == "MESSAGE"){
                        cout << "<" << arguments[0][2] << ">" << " " << arguments[0][4] << endl;
                    }

                    if(arguments[0][1] == "CHANNAMES"){
                        for(int i = 2; i <arguments[0].size();i++)
                        {
                            cout << "Channel: " << arguments[0][i] << endl;
                        }
                    }
               }
               else{

               }


}

int main(void)
{
    ltc_mp = ltm_desc;
    register_prng(&sprng_desc);
    int err;
    Pa_Initialize();
    Pa_OpenDefaultStream(&stream,2,2,paInt16,48000,4,NULL,NULL);
    Pa_StartStream(stream);
    encoder = opus_encoder_create(48000,2,OPUS_APPLICATION_AUDIO,&err);
    decoder = opus_decoder_create(48000,2,&err);
    err = opus_encoder_ctl(encoder,OPUS_SET_BITRATE(48000));

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
    cout << "Enter your WAMP server uri." << endl;
    getline(cin,uri);
    auto transport = make_shared<autobahn::wamp_websocketpp_websocket_transport<websocketpp::config::asio_client>>(ws_client,uri,false);

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
            session->subscribe("com.audioctl." + current_user.name,&process_command);
            session->publish("com.audioctl.main", std::make_tuple(std::string("NICK"),std::string(current_user.name),base64key));
            while(true)
    {
            string command;
            getline(cin,command);
            if(command.size() == 0)
            continue;
            if(command[0] == '/')
    {
        vector<string> cmd;
        split_string(command," ",cmd);
        cmd[0] = remove_erase_if(cmd[0],"/");
        if(cmd[0] == "quit")
        {
            publish_to_channel("com.audioctl." + current_user.name,vector<string>(1,"QUIT"));
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
                    cout << "You need to leave your channel first." << endl;
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
                    cout << "You need to be in a channel." << endl;
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
