//Multimediale Netzwerke und IT Sicherheit
//Uebungsprojekt
//Andreas Stallinger / Wolfgang Vogl

#include "timestamp.h"

timestamp::timestamp()
{
    getTime();
    //ctor
}

timestamp::~timestamp()
{
    //dtor
}

timestamp::timestamp(const timestamp& other)
{
    day = other.day;
    mon = other.mon;
    year = other.year;
    hour = other.hour;
    min = other.min;
    sec = other.sec;
    //copy ctor
}

timestamp& timestamp::operator=(const timestamp& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}

void timestamp::printTime(){
    printf(" %2d %2d %4d, %2d:%2d:%2d",
    day, mon, year, hour, min, sec);
}

void timestamp::getTime()
 {
    time_t ltime;
    struct tm *Tm;

    ltime=time(NULL);
    Tm=localtime(&ltime);

    day = Tm->tm_mday;
    mon =Tm->tm_mon+1,
    year=(Tm->tm_year)+1900,
    hour=Tm->tm_hour,
    min=Tm->tm_min,
    sec=Tm->tm_sec;
 }

std::string timestamp::convertToString(){
    char s[20];
    sprintf(s,"%04d%02d%02d%02d%02d%02d",year,mon,day,hour,min,sec);
    return s;
}
