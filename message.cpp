//Multimediale Netzwerke und IT Sicherheit
//Uebungsprojekt
//Andreas Stallinger / Wolfgang Vogl

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
    name = other.name;
    text = other.text;
    time = other.time;
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

string message::convertToString(){
    return time.convertToString()+name+text;
}
