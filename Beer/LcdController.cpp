#include "LcdController.h"

void  LcdController::write(uint8_t numIcone, int lin, int col, boolean clear) {
    if (clear) _lcd.clear();
    _lcd.setCursor(col, lin);
    _lcd.write(numIcone);
    
}

void LcdController::createChar(uint8_t num, uint8_t* icone) {
    _lcd.createChar(num, icone);
}


void LcdController::print(String text, int col, int lin, boolean clear) {
    if (clear) _lcd.clear();
    _lcd.setCursor(col, lin);
    _lcd.print(text);
}

void LcdController::print(double valor, int col, int lin, boolean clear) {
    if (clear) _lcd.clear();
    _lcd.setCursor(col, lin);
    _lcd.print(valor);
}

void LcdController::clear() {
    _lcd.clear();
}

void LcdController::init(int col, int lin, String title, String load) {
    _lcd.begin(col, lin);
    _lcd.backlight();
    _lcd.clear();
    _lcd.setCursor(0, 0);
    _lcd.print(title);
    //	delay(1000);
    //	_lcd.setCursor(0, 1);
    //	_lcd.print(load);
    delay(500);
    //	for (int i = 10; i < 18; i++) {
    //		delay(150);
    //		_lcd.setCursor(i, 1);
    //		_lcd.print(".");
    //	}
    //	_lcd.setCursor(10, 1);
    //	_lcd.print("          ");
    //	delay(500);
    //	for (int i = 10; i < 18; i++) {
    //		delay(150);
    //		_lcd.setCursor(i, 1);
    //		_lcd.print(".");
    //	}
    //	delay(500);
    //	_lcd.setCursor(10, 1);
    //	_lcd.print("          ");
    //	for (int i = 10; i < 18; i++) {
    //		delay(150);
    //		_lcd.setCursor(i, 1);
    //		_lcd.print(".");
    //	}
    //	delay(350);
    //	_lcd.setCursor(0, 0);
    _lcd.clear();
}
