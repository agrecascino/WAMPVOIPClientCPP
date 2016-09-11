#ifndef INCLUDES_H
#define INCLUDES_H
#include <boost/asio.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <autobahn/wamp_websocketpp_websocket_transport.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <inttypes.h>
#include <string.h>
#include <tuple>
#include <autobahn/autobahn.hpp>
#include <tomcrypt.h>
#include <tfm.h>
#include <thread>
#include <boost/any.hpp>
typedef websocketpp::client<websocketpp::config::asio_client> client;

#endif // INCLUDES_H
