#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include "timestamp.h"

using namespace std;

class message
{
    public:
        message(string, string, timestamp);
        virtual ~message();
        message(const message& other);
        message& operator=(const message& other);
        string getName();
        string getText();
        string convertToString();
    protected:
    private:
        string name;
        string text;
        timestamp time;
};

#endif // MESSAGE_H
