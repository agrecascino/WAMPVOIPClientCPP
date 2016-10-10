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

rsa_key key;
rsa_key serv_pub;

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
#endif // INCLUDES_H
