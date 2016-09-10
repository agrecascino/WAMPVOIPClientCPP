#include "includes.h"
using namespace std;
struct User {
    string name;
    string channel;
    vector<unsigned char> pubkey;
    vector<unsigned char> privkey;
};
User current_user;
int base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize)
{
   const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
   const uint8_t *data = (const uint8_t *)data_buf;
   size_t resultIndex = 0;
   size_t x;
   uint32_t n = 0;
   int padCount = dataLength % 3;
   uint8_t n0, n1, n2, n3;

   /* increment over the length of the string, three characters at a time */
   for (x = 0; x < dataLength; x += 3) 
   {
      /* these three 8-bit (ASCII) characters become one 24-bit number */
      n = ((uint32_t)data[x]) << 16; //parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0
      
      if((x+1) < dataLength)
         n += ((uint32_t)data[x+1]) << 8;//parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0
      
      if((x+2) < dataLength)
         n += data[x+2];

      /* this 24-bit number gets separated into four 6-bit numbers */
      n0 = (uint8_t)(n >> 18) & 63;
      n1 = (uint8_t)(n >> 12) & 63;
      n2 = (uint8_t)(n >> 6) & 63;
      n3 = (uint8_t)n & 63;
            
      /*
       * if we have one byte available, then its encoding is spread
       * out over two characters
       */
      if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
      result[resultIndex++] = base64chars[n0];
      if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
      result[resultIndex++] = base64chars[n1];

      /*
       * if we have only two bytes available, then their encoding is
       * spread out over three chars
       */
      if((x+1) < dataLength)
      {
         if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
         result[resultIndex++] = base64chars[n2];
      }

      /*
       * if we have all three bytes available, then their encoding is spread
       * out over four characters
       */
      if((x+2) < dataLength)
      {
         if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
         result[resultIndex++] = base64chars[n3];
      }
   }  

   /*
    * create and add padding that is required if we did not have a multiple of 3
    * number of characters available
    */
   if (padCount > 0) 
   { 
      for (; padCount < 3; padCount++) 
      { 
         if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
         result[resultIndex++] = '=';
      } 
   }
   if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
   result[resultIndex] = 0;
   return 0;   /* indicate success */
}

void process_command(const autobahn::wamp_event& event)
{

}

int main(void)
{
    rsa_key key;
    ltc_mp = ltm_desc;
    register_prng(&sprng_desc);
    if (rsa_make_key(NULL, find_prng("sprng"), 1536/8, 65537, &key) != CRYPT_OK) {
        return -1;
    }
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
                    unsigned char* public_key = new unsigned char[1024];
                    memset(public_key,0,1024);
                    unsigned long length;
                    rsa_export(public_key,&length,PK_PUBLIC,&key);
                    current_user.pubkey = vector<unsigned char>(public_key,public_key + length);
                    auto args = std::make_tuple(std::string("NICK"),std::string(current_user.name),current_user.pubkey);
                    session->subscribe("com.audioctl." + current_user.name,&process_command);
                    session->publish("com.audioctl.main", args);


                });
            });
    });
    io.run();
    return 0;
}
