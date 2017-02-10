#ifndef INCLUDES_H
#define INCLUDES_H
/* Define needed headers and global helper functions in here */

#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#include <future>
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
#include <stack>
#include <map>
#include <cstdlib>
#include <tuple>
#include <autobahn/autobahn.hpp>
#include <autobahn/wamp_subscription.hpp>
#include <thread>
#include <mutex>
#include <time.h>
#include <signal.h>
#include <boost/any.hpp>
//#include <opus/opusfile.h>
#include <vorbis/vorbisfile.h>
#include <panel.h>
#include <ncurses.h>
#include <functional>
typedef websocketpp::client<websocketpp::config::asio_client> client;
extern boost::asio::io_service io;
#ifdef LTC_LTC_PKCS_1_V1_5
#define LTC_PKCS_1_V1_5 LTC_LTC_PKCS_1_V1_5
#endif

char* itoa(int val, int base);

void split_string(std::string const &k, std::string const &delim, std::vector<std::string> &output);

std::string remove_erase_if(std::string c, std::string delim);

inline bool is_base64(unsigned char c);

std::string base_64_decode(std::string const& encoded_string);

#endif // INCLUDES_H
