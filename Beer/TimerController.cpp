#include <Time.h>
#include <TimeLib.h>
#include <Wire.h>
//#include <Arduino.h>
#include "TimerController.h"
#define byte uint8_t

TimerController::TimerController(int address) {
    _address = address;
    Wire.begin();
}

byte TimerController::decToBcd(byte val) {
    return ((val / 10 * 16) + (val % 10));
}

byte TimerController::bcdToDec(byte val) {
    return ((val / 16 * 10) + (val % 16));
}
String TimerController::twoDigit(byte digit) {
    return digit < 10 ? "0" + String(digit) : String(digit);
}

void TimerController::setDate(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year) {
    Wire.beginTransmission(_address);
    Wire.write(0);
    Wire.write(decToBcd(second));
    Wire.write(decToBcd(minute));
    Wire.write(decToBcd(hour));
    Wire.write(decToBcd(dayOfWeek));
    Wire.write(decToBcd(dayOfMonth));
    Wire.write(decToBcd(month));
    Wire.write(decToBcd(year));
    Wire.endTransmission();
}

void TimerController::getAllDate(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year) {
    Wire.beginTransmission(_address);
    Wire.write(0);
    Wire.endTransmission();
    Wire.requestFrom(_address, 7);
    *second = bcdToDec(Wire.read() & 0x7f);
    *minute = bcdToDec(Wire.read());
    *hour = bcdToDec(Wire.read() & 0x3f);
    *dayOfWeek = bcdToDec(Wire.read());
    *dayOfMonth = bcdToDec(Wire.read());
    *month = bcdToDec(Wire.read());
    *year = bcdToDec(Wire.read());
}

void TimerController::getAllDate() {
    getAllDate(&_second, &_minute, &_hour, &_dayOfWeek, &_dayOfMonth, &_month, &_year);
}

String TimerController::getDate() {
    getAllDate();
    String d, m, y;
    d = twoDigit(_dayOfMonth);
    m = twoDigit(_month);
    y = "20" + twoDigit(_year);
    return  d + "/" + m + "/" + y;
}

String TimerController::getTime() {
    getAllDate();
    String h, m, s;
    h = twoDigit(_hour);
    m = twoDigit(_minute);
    s = twoDigit(_second);
    return  h + ":" + m + ":" + s;
}

String TimerController::getDayOfWeek() {
    getAllDate();
    char* days[] = { "NA", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab", "Dom" };
    return String(days[_dayOfWeek]);
}

void TimerController::getTime(byte *second, byte *minute, byte *hour) {
    getAllDate();
    *second = _second;
    *minute = _minute;
    *hour = _hour;
}

void TimerController::getDate(byte *day, byte *month, byte *year) {
    getAllDate();
    *day = _dayOfMonth;
    *month = _month;
    *year = _year;
}
