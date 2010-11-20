#ifndef TIME_H
#define TIME_H


class time
{
    public:
        time();
        virtual ~time();
        time(const time& other);
        time& operator=(const time& other);
    protected:
    private:
        int day;
        int mon;
        int year;
        int hour;
        int min;
        int sec;
};

#endif // TIME_H
struct time_struct{

};
