#ifndef ABSTRACTOR_H
#define ABSTRACTOR_H
#define NCURSES_INTERFACE
#include <structures.h>
#ifdef NCURSES_INTERFACE
#include <ncursesdisp.h>
#endif
class DisplayAbstractor {
    public:
    DisplayAbstractor();

    string get_line();

    int print_to_screen(string screen, string message, string panel = "");

    int clear_screen(string screen, string panel= "");

    int destroy_screen(string name);

    int switch_screen(string name, bool force = false);

    string get_current_panel();

    string create_screen(string name);
    
    private:
    DisplayFunctionWrapper wrapper;
    vector<string> owned_panels;
};

#endif // ABSTRACTOR_H
