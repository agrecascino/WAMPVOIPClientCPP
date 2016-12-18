#ifndef CHATLOGGER_H
#define CHATLOGGER_H
#include <abstractor.h>

class ChatLogger {
public:
    ChatLogger(DisplayAbstractor &abstractref, int reactorid_tmp);

    void set_screen(string scr);

    void add_message(Message msg);

    void write_out_lines_lambda(size_t lines,function<int(Message)> func);

    void write_out_lines(size_t lines);

private:
    int reactorid;
    DisplayAbstractor *abstractor;
    string current_screen;
    vector<Message> messages;
};

#endif // CHATLOGGER_H
