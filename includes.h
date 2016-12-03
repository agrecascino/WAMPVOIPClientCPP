#ifndef INCLUDES_H
#define INCLUDES_H
#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION

#include <boost/thread/future.hpp>
#include <boost/asio.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <autobahn/wamp_websocketpp_websocket_transport.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <inttypes.h>
#include <math.h>
#include <functional>
#include <string.h>
#include <opus/opus.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <map>
#include <cstdlib>
#include <tuple>
#include <autobahn/autobahn.hpp>
#include <autobahn/wamp_subscription.hpp>
#include <tomcrypt.h>
#include <thread>
#include <mutex>
#include <time.h>
#include <signal.h>
#include <boost/any.hpp>
//#include <opus/opusfile.h>
#include <vorbis/vorbisfile.h>
#include <ncurses.h>
typedef websocketpp::client<websocketpp::config::asio_client> client;
#ifdef LTC_LTC_PKCS_1_V1_5
#define LTC_PKCS_1_V1_5 LTC_LTC_PKCS_1_V1_5
#endif
rsa_key key;
rsa_key serv_pub;
WINDOW *vin;
WINDOW *cmd;
WINDOW *dbg;
OggVorbis_File vf;
FILE *vfile;
std::vector<char> oggdec;
std::mutex mtx;
#define MIXER_AUDIO_16BITS_STEREO 1
#define MIXER_AUDIO_8BITS_STEREO  2
#define MIXER_AUDIO_16BITS_MONO   3
#define MIXER_AUDIO_8BITS_MONO    4
char* itoa(int val, int base){

    static char buf[32] = {0};

    int i = 30;

    for(; val && i ; --i, val /= base)

        buf[i] = "0123456789abcdef"[val % base];

    return &buf[i+1];

}


void split_string(std::string const &k, std::string const &delim, std::vector<std::string> &output)
{
    // Due to the use of strpbrk here, this will stop copying at a null char. This is in fact desirable.
    char const *last_ptr = k.c_str(), *next_ptr;
    while ((next_ptr = strpbrk(last_ptr, delim.c_str())) != nullptr)
    {
        output.emplace_back(last_ptr, next_ptr - last_ptr);
        last_ptr = next_ptr + 1;
    }
    output.emplace_back(last_ptr);
}

std::string remove_erase_if(std::string c, std::string delim)
{
    std::string output;
    for(unsigned int i = 0; i < c.size();i++)
    {
        if(strchr(delim.c_str(), c[i]) == nullptr)
        {
            output += c[i];
        }
    }
    return output;
}

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base_64_decode(std::string const& encoded_string) {
  static const std::string base64_chars =
                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                 "abcdefghijklmnopqrstuvwxyz"
                 "0123456789+/";
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}
#endif // INCLUDES_H
