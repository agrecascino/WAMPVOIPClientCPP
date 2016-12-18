#include <abstractor.h>
DisplayAbstractor::DisplayAbstractor() {
    wrapper = get_function_wrapper();
#ifdef NCURSES_INTERFACE
    if(wrapper.init_display() != -2)
        owned_panels.push_back("main");
#endif
}

string DisplayAbstractor::get_line() {
    return wrapper.get_user_input();
}

int DisplayAbstractor::print_to_screen(string screen, string message, string panel) {
    if(panel == "") {
        panel = current_panel;
    }
    if(screen == "chat") {
        wrapper.display_to_chat_screen(message,panel);
    } else if(screen == "input") {
        wrapper.display_to_input_screen(message,panel);
    } else if(screen == "users") {
        wrapper.display_to_users_bar(message,panel);
    } else if(screen == "notif") {
        wrapper.display_to_notif_bar(message,panel);
    }
    return 0;
}

int DisplayAbstractor::clear_screen(string screen, string panel) {
    if(panel == "") {
        panel = current_panel;
    }
    if(screen == "chat") {
        wrapper.clear_chat_screen(panel);
    } else if(screen == "input") {
        wrapper.clear_input_screen(panel);
    } else if(screen == "users") {
        wrapper.clear_users_bar(panel);
    } else if(screen == "notif") {
        wrapper.clear_notif_bar(panel);
    }
    return 0;
}


string DisplayAbstractor::create_screen(string name) {
#ifdef NCURSES_INTERFACE
    int rv = wrapper.create_panel(name);
    if(!rv) {
        owned_panels.push_back(name);
        return name;
    }
#endif
    return "";
}

int DisplayAbstractor::destroy_screen(string name) {
#ifdef NCURSES_INTERFACE
    owned_panels.erase(std::remove(owned_panels.begin(),owned_panels.end(),name),owned_panels.end());
    return wrapper.destroy_panel(name);
#endif
}

int DisplayAbstractor::switch_screen(string name, bool force) {
    bool is_owned = false;
    for(string s : owned_panels) {
        if(s == name) {
            is_owned = true;
        }
    }
    if(force)
        is_owned = true;
    #ifdef NCURSES_INTERFACE
    if(is_owned) {
    int ret = wrapper.switch_panel(name);
    if(!ret)
        return -1;
    current_panel = name;
    return ret;
    }
    else {
        return -1;
    }
#endif
}

string DisplayAbstractor::get_current_panel() {
#ifdef NCURSES_INTERFACE
    return wrapper.get_current_panel();
#endif
}
