#include "message.h"

message::message()
{
    //ctor
}

message::~message()
{
    //dtor
}

message::message(const message& other)
{
    //copy ctor
}

message& message::operator=(const message& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}
