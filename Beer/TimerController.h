// TimerController.h

#ifndef _TIMERCONTROLLER_h
#define _TIMERCONTROLLER_h

//#if defined(ARDUINO) && ARDUINO >= 100
//#include "arduino.h"
//#else
//#include "WProgram.h"
//#endif
#define byte uint8_t

class TimerController {
protected:
    
public:
    TimerController(int address);
    void getAllDate(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year);
    void setDate(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year);
    String getDate();
    String getTime();
    String getDayOfWeek();
    void getTime(byte *second, byte *minute, byte *hour);
    void getDate(byte *day, byte *month, byte *year);
    void init(int address);
    
private:
    int _address;
    char* _days[8];
    byte _second;        // 0-59
    byte _minute;        // 0-59
    byte _hour;          // 1-23
    byte _dayOfWeek;     // 1-7 1=Mon, 7=Sun
    byte _dayOfMonth;    // 1-28/29/30/31
    byte _month;         // 1-12
    byte _year;          // 0-99
    byte decToBcd(byte val);
    byte bcdToDec(byte val);
    void getAllDate();
    String twoDigit(byte digit);
};
#endif
