#ifndef STRUCTURES_H
#define STRUCTURES_H
#include <includes.h>
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
    vector<string> channel;
    map<string,vector<RemoteUser>> channelusers;
    string screen_channel;
};

struct DisplayFunctionWrapper {
    int (*clear_chat_screen)(string s);
    int (*clear_input_screen)(string s);
    int (*clear_notif_bar)(string s);
    int (*clear_users_bar)(string s);
    int (*create_panel)(string s);
    int (*switch_panel)(string s);
    int (*destroy_panel)(string s);
    int (*display_to_chat_screen)(string s, string panel);
    int (*display_to_input_screen)(string s, string panel);
    int (*display_to_notif_bar)(string s, string panel);
    int (*display_to_users_bar)(string s, string panel);
    string (*get_user_input)();
    string (*get_current_panel)();
    int (*init_display)();
};

struct DebuggerFunctionWrapper {
    int (*display_to_screen)(string s);
    int (*init_telnet_debbuger)();
    string (*get_user_input)();
};

struct NamedPanel {
    NamedPanel(string s) {
        name = s;
    }

    string name;
    WINDOW *windows[4];
    PANEL *panel[4];
};

#endif // STRUCTURES_H
