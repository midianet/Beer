#include "Keypad_I2C.h"

// Let the user define a keymap - assume the same row/column count as defined in constructor
void Keypad_I2C::begin(char *userKeymap) {
  Keypad::begin(userKeymap);
  TwoWire::begin();
  pinState = pinState_set( );
}

// Initialize I2C
void Keypad_I2C::begin(void) {
  TwoWire::begin();
  //pinState = 0xff;
  pinState = pinState_set( );
}

// Initialize I2C
void Keypad_I2C::begin(byte address) {
  i2caddr = address;
  TwoWire::begin(address);
  //pinState = 0xff;
  pinState = pinState_set( );
}

// Initialize I2C
void Keypad_I2C::begin(int address) {
  i2caddr = address;
  TwoWire::begin(address);
  //pinState = 0xff;
  pinState = pinState_set( );
}


void Keypad_I2C::pin_write(byte pinNum, boolean level) {
  word mask = 1<<pinNum;
  if( level == HIGH ) {
    pinState |= mask;
  } else {
    pinState &= ~mask;
  }
  port_write( pinState );
} // I2CxWrite( )


int Keypad_I2C::pin_read(byte pinNum) {
  word mask = 0x1<<pinNum;
  TwoWire::requestFrom((int)i2caddr, (int)i2cwidth);
  word pinVal = TwoWire::read( );
  if (i2cwidth > 1) {
    pinVal |= TwoWire::read( ) << 8;
  } 
  pinVal &= mask;
  if( pinVal == mask ) {
    return 1;
  } else {
    return 0;
  }
}

void Keypad_I2C::port_write( word i2cportval ) {
  TwoWire::beginTransmission((int)i2caddr);
  TwoWire::write( i2cportval & 0x00FF);
  if (i2cwidth > 1) {
    TwoWire::write( i2cportval >> 8 );
  }
  TwoWire::endTransmission();
  pinState = i2cportval;
} // port_write( )

word Keypad_I2C::pinState_set( ) {
  TwoWire::requestFrom( (int)i2caddr, (int)i2cwidth );
  pinState = TwoWire::read( );
  if (i2cwidth > 1) {
    pinState |= TwoWire::read( ) << 8;
  }
  return pinState;
} // set_pinState( )
