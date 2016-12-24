#include <ncursesdisp.h>
bool started_flag = false;

string current_panel = "";

vector<NamedPanel> panels;


string getline() {
    string str;
    for(NamedPanel &panel : panels){
        if(panel.name == current_panel) {
            while(true)
            {
                int i = wgetch(panel.windows[2]);
                if(((i == 127) || (i == 8)) && str.size() != 0)
                {
                    str.pop_back();
                    wdelch(panel.windows[2]);
                    wprintw(panel.windows[2],"\b");
                    wdelch(panel.windows[2]);
                    wrefresh(panel.windows[2]);
                    continue;
                }
                else if(((i == 127) || (i == 8)) && str.size() == 0)
                {
                    continue;
                }
                else if(i == KEY_RESIZE) {
                    for(NamedPanel &p : panels) {
                        //wresize(p.windows[0],)
                    }
                    continue;
                }
                if(i == '\n')
                    break;
                str += i;
                wprintw(panel.windows[2],string(1,i).c_str());
                wrefresh(panel.windows[2]);
            }
            //wprintw(cmd,"\n");
            wclear(panel.windows[2]);
            wrefresh(panel.windows[2]);
            return str;
        }
    }
}

string get_current_panel() {
    return current_panel;
}

int clear_chat_screen(string s) {
    for(NamedPanel &panel : panels) {
        if(panel.name == s) {
            wclear(panel.windows[0]);
            return 0;
        }
    }
    return -1;
}

int clear_input_screen(string s) {
    for(NamedPanel &panel : panels) {
        if(panel.name == s) {
            wclear(panel.windows[2]);
            return 0;
        }
    }
    return -1;
}

int clear_notif_bar(string s) {
    for(NamedPanel &panel : panels) {
        if(panel.name == s) {
            wclear(panel.windows[3]);
            return 0;
        }
    }
    return -1;
}

int clear_users_bar(string s) {
    for(NamedPanel &panel : panels) {
        if(panel.name == s) {
            wclear(panel.windows[1]);
            return 0;
        }
    }
    return -1;
}

int init_ncurses() {
    if(started_flag)
        return -2;
    started_flag = true;
    initscr();
    raw();
    noecho();
    start_color();
    init_pair(1,COLOR_WHITE,COLOR_BLUE);
    panel_create("main");
    panel_switch("main");
    //start_color();
    //init_pair(0, COLOR_RED, COLOR_BLUE);
    //attron(0);
    return 0;
}

int print_to_chat_screen(string s, string paneln) {
    for(NamedPanel &panel : panels) {
        if(panel.name == paneln) {
            wprintw(panel.windows[0],s.c_str());
            wrefresh(panel.windows[0]);
            return 0;
        }
    }
    return -1;
}

int print_to_input_screen(string s, string paneln) {
    for(NamedPanel &panel : panels) {
        if(panel.name == paneln) {
            wprintw(panel.windows[2],s.c_str());
            wrefresh(panel.windows[2]);
            return 0;
        }
    }
    return -1;
}

int print_to_notif_bar(string s, string paneln) {
    for(NamedPanel &panel : panels) {
        if(panel.name == paneln) {
            wprintw(panel.windows[3],s.c_str());
            wrefresh(panel.windows[3]);
            return 0;
        }
    }
    return -1;
}

int print_to_users_bar(string s, string paneln) {
    for(NamedPanel &panel : panels) {
        if(panel.name == paneln) {
            wprintw(panel.windows[1],s.c_str());
            wrefresh(panel.windows[1]);
            return 0;
        }
    }
    return -1;
}

int panel_switch(string s) {
    for(NamedPanel &panel : panels) {
        if(panel.name == s) {
            for(NamedPanel &panel : panels) {
                if(panel.name == current_panel) {
                    for(int i = 0; i < 4;i++)
                        hide_panel(panel.panel[i]);
                }
            }
            for(int i = 0; i < 4;i++) {
                wrefresh(panel.windows[i]);
                show_panel(panel.panel[i]);
            }
            current_panel = s;
            update_panels();
            return 0;
        }
    }
    return -1;
}

int panel_destroy(string s) {
    for(NamedPanel &panel : panels) {
        if(panel.name == s && panel.name != "main" && panel.name != current_panel) {
            for(int i = 0; i < 4;i++)
                del_panel(panel.panel[i]);
            for(int i = 0; i < 4;i++)
                delwin(panel.windows[i]);
        }
    }
    update_panels();
    return -1;
}

int panel_create(string s) {
    panels.push_back(NamedPanel(s));
    panels.back().windows[0] = newwin(LINES - 2,COLS - 20,1,0);
    panels.back().windows[1] = newwin(LINES - 2,20,1,COLS - 20);
    panels.back().windows[2] = newwin(1,COLS,LINES - 1,0);
    wbkgd(panels.back().windows[2],COLOR_PAIR(1));
    panels.back().windows[3] = newwin(1,COLS,0,0);
    wbkgd(panels.back().windows[3],COLOR_PAIR(1));
    for(int i = 0;i < 4;i++) {
        scrollok(panels.back().windows[i],TRUE);
        panels.back().panel[i] = new_panel(panels.back().windows[i]);
    }
    return 0;
}

DisplayFunctionWrapper get_function_wrapper() {
    DisplayFunctionWrapper w;
    w.get_user_input = &getline;
    w.init_display = &init_ncurses;
    w.create_panel = &panel_create;
    w.destroy_panel = &panel_destroy;
    w.switch_panel = &panel_switch;
    w.clear_chat_screen = &clear_chat_screen;
    w.clear_input_screen = &clear_input_screen;
    w.clear_notif_bar = &clear_notif_bar;
    w.clear_users_bar = &clear_users_bar;
    w.display_to_chat_screen = &print_to_chat_screen;
    w.display_to_input_screen = &print_to_input_screen;
    w.display_to_notif_bar = &print_to_notif_bar;
    w.display_to_users_bar = &print_to_users_bar;
    w.get_current_panel = &get_current_panel;
    return w;
}
