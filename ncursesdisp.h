 #ifndef NCURSESDISP_H
#define NCURSESDISP_H
#include <includes.h>
#include <structures.h>
using namespace std;

extern bool started_flag;

int init_ncurses();

int panel_switch(string s);

int panel_destroy(string s);

int panel_create(string s);

vector<string> panel_list();

int clear_chat_screen(string s);

int clear_input_screen(string s);

int clear_notif_bar(string s);

int clear_users_bar(string s);

int print_to_chat_screen(string s, string paneln);

int print_to_input_screen(string s, string paneln);

int print_to_notif_bar(string s, string paneln);

int print_to_users_bar(string s, string paneln);

string getline(bool display_name = true);

string get_current_panel();

DisplayFunctionWrapper get_function_wrapper();

extern string current_panel;

extern vector<NamedPanel> panels;

#endif // NCURSESDISP_H
