#ifndef _LCDCONTROLLER_h
#define _LCDCONTROLLER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <LiquidCrystal_I2C.h>

class LcdController {
public:
    LcdController(LiquidCrystal_I2C& lcd) : _lcd(lcd) {};
    void print(String text, int lin, int col, boolean clear = false);
    void print(double valor, int lin, int col, boolean clear = false);
    void init(int col, int lin, String title, String load);
    void createChar(uint8_t num, uint8_t* icone);
    void write(uint8_t numIcone, int lin, int col, boolean clear = false);
    void clear();
private:
    LiquidCrystal_I2C& _lcd;
};
#endif
