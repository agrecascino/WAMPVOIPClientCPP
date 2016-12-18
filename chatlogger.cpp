#include <chatlogger.h>

ChatLogger::ChatLogger(DisplayAbstractor &abstractref, int reactorid_tmp) {
    reactorid = reactorid_tmp;
    abstractor = &abstractref;
}

void ChatLogger::add_message(Message msg)
{
    messages.push_back(msg);
}

void ChatLogger::write_out_lines_lambda(size_t lines,function<int(Message)> func)
{
    if(lines > messages.size())
        lines = messages.size();
    if(messages.size() == 0)
    {
        return;
    }
    size_t l = messages.size() - 1 ;
    do{
        int output_message = func(messages[l]);
        if(output_message)
            abstractor->print_to_screen("chat","<" + messages[l].name + ">" + " " + messages[l].message + "\n","reactor" + to_string(reactorid) + "_" + messages[l].channel);
        l--;
    }while(l > (messages.size() - 1) - lines);

}

void ChatLogger::write_out_lines(size_t lines)
{
    this->write_out_lines_lambda(lines,[=](Message msg){ return true; });
}
