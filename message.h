#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include "timestamp.h"

using namespace std;

class message
{
    public:
        message();
        virtual ~message();
        message(const message& other);
        message& operator=(const message& other);
    protected:
    private:
        string name;
        string text;
        timestamp time;
};

#endif // MESSAGE_H
