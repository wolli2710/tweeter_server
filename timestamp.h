#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <time.h>
#include <stdio.h>
#include <string>

class timestamp
{
    public:
        timestamp();
        virtual ~timestamp();
        timestamp(const timestamp& other);
        timestamp& operator=(const timestamp& other);
        void printTime();
        void getTime();
        std::string convertToString();
    protected:
    private:
        int day;
        int mon;
        int year;
        int hour;
        int min;
        int sec;
};

#endif // TIMESTAMP_H


