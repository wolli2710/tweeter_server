#include "message.h"

message::message(string username, string message, timestamp currenttime)
    :name(username),
     text(message),
     time(currenttime)
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

string message::getName(){
    return name;
}

string message::getText(){
    return text;
}
