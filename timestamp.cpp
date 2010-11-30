#include "timestamp.h"

timestamp::timestamp()
{
    //ctor
}

timestamp::~timestamp()
{
    //dtor
}

timestamp::timestamp(const timestamp& other)
{
    //copy ctor
}

timestamp& timestamp::operator=(const timestamp& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}

void timestamp::printTime(){
    printf(" %d %d %d, %d:%d:%d",
    day, mon, year, hour, min, sec);
}

void timestamp::getTime()
 {
    time_t ltime;
    struct tm *Tm;

    ltime=time(NULL);
    Tm=localtime(&ltime);

    day = Tm->tm_mday;
    mon =Tm->tm_mon,
    year=(Tm->tm_year)+1900,
    hour=Tm->tm_hour,
    min=Tm->tm_min,
    sec=Tm->tm_sec;
 }

std::string timestamp::convertToString(){
    return "";
}
