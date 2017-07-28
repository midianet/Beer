#ifndef KEYPAD_I2C_H
#define KEYPAD_I2C_H

#include "Keypad.h"
#include "Wire.h"
#define  PCF8574 1 // PCF8574 I/O expander device is 1 byte wide
#define  PCF8575 2 // PCF8575 I/O expander device is 2 bytes wide

class Keypad_I2C : public Keypad, public TwoWire {
  public:
    Keypad_I2C(char* userKeymap, byte* row, byte* col, byte numRows, byte numCols, byte address, byte width = 1) :
    Keypad(userKeymap, row, col, numRows, numCols) { i2caddr = address; i2cwidth = width;}  
    // Keypad function
    void begin(char *userKeymap);
    // Wire function
    void begin(void);
    // Wire function
    void begin(byte address);
    // Wire function
    void begin(int address);
    
    void pin_mode(byte pinNum, byte mode) {}
    void pin_write(byte pinNum, boolean level);
    int  pin_read(byte pinNum);
    // read initial value for pinState
    word pinState_set( );
    // write a whole byte or word (depending on the port expander chip) to i2c port
    void port_write( word i2cportval );
  private:
    // I2C device address
    byte i2caddr;
    // I2C port expander device width in bytes (1 for 8574, 2 for 8575)
    byte i2cwidth;
    // I2C pin_write state persistant storage
    // least significant byte is used for 8-bit port expanders
    word pinState;
};

#endif
