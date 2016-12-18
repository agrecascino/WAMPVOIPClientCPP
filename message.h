#ifndef MESSAGE_H
#define MESSAGE_H
#include <includes.h>

namespace Mail {

template <class T>
class Message {
    public:

    Message(T d) {
        setData(d);
    }

    void setData(T d)
    {
        //message = promise<T>();
        message.set_value(d);
    }

    std::future<T> getData()
    {
        return message.get_future();
    }

    private:
    std::promise<T> message;
};

template <class T>
class Mailbox {
    public:
    void stuffMessage(Message<T> &f)
    {
        mail.push(f.getData());
    }

    bool isEmpty()
    {
        if(this->messages() == 0)
            return true;
        std::future<T> &ref = mail.top();
        if(ref.valid())
        {
            std::future_status status = ref.wait_for(chrono::microseconds(1));
            if(status == future_status::ready)
                return false;
        }
        return true;
    }

    size_t messages()
    {
        return mail.size();
    }

    T getMessage()
    {
        T data = mail.top().get();
        mail.pop();
        return data;
    }

    private:
    std::stack<future<T>> mail;
};

}
#endif // MESSAGE_H
