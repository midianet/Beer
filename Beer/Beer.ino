#include <Wire.h>
#include "Keypad_I2C.h"
#include <Keypad.h>

#include <max6675.h>
#include <LiquidCrystal_I2C.h>
#include <Thread.h>
#include <ThreadController.h>
#include <Time.h>
#include "LcdController.h"
#include "TimerController.h"

const int rele   = A3;
const int buzzer = A0;

/**** MENU  **********/
enum statusMenu { MAIN_SCREEN, NUM_RAMP_SCREEN, TEMP_RAMP_SCREEN, TIME_RAMP_SCREEN, CLOCK_SCREEN, PRE_RUN_SCREEN,RUN_SCREEN, BREAK_SCREEN, FINISH_SCREEN};
statusMenu status_menu = MAIN_SCREEN;

/**** THREAD *********/
ThreadController threadControl = ThreadController();
Thread threadLCD               = Thread();
Thread threadTemp              = Thread();
Thread threadKeyboard          = Thread();
Thread threadRun               = Thread();

/****  KEYBOARD  *****/
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {{'1','2','3','A'},{'4','5','6','B'},{'7','8','9','C'},{'*','0','#','D'}};
byte rowPins[ROWS] = {0,1,2,3}; 
byte colPins[COLS] = {4,5,6,7};
int i2caddress = 0x38;
Keypad_I2C kpd = Keypad_I2C( makeKeymap(keys) , rowPins  , colPins  , ROWS        , COLS        , i2caddress, PCF8574);

/**** LCD ************/
LiquidCrystal_I2C lcd(0x27, 2, 4, 0, 4, 5, 6, 7, 3, POSITIVE);
LcdController     lcdController  = LcdController(lcd);

/**** TEMP  **********/
MAX6675 thermocouple(7,8,9);
double temp = 0.00;

/**** ICONS **********/
byte clock_icon  [8] = {B00000, B01110, B10101, B10111, B10001, B01110, B00000, B00000 };
byte term_icon   [8] = {B00100, B01010, B01010, B01110, B01110, B11111, B11111, B01110 };
byte degC_icon   [8] = {B01000, B10100, B01000, B00111, B01000, B01000, B01000, B00111 };
byte bar_icon    [8] = {B00001, B00001, B00001, B00001, B00001, B00001, B00001, B00001 };
byte burn_icon   [8] = {B00100, B00100, B01010, B01010, B10001, B10001, B10001, B01110 };
byte calend_icon [8] = {B11111, B10001, B11111, B10001, B10001, B10001, B10001, B11111 };

/**** RAMP  **********/
int     *ramp_temp;
int     *ramp_time;
int     ramp_size      = 0;
int     ramp_index     = 0;
boolean ramp_programed = false;

/**** CLOCK  *********/
const long m_day                  = 86400000;
const long m_hour                 = 3600000;
const long m_minute               = 60000;
const long m_second               = 1000;
int     set_clock_index = 0;
byte    set_clock[7][3] = {{0,59,0},{0,59,0},{1,23,0},{1,7,0},{1,31,0},{1,12,0},{0,99,0}};
TimerController timeController = TimerController(0x68);//

/**** RUN  ***********/
long    run_start   = 0;
String  run_hours   = "00";
String  run_minutes = "00";
String  run_seconds = "00";

String  input_value = "";

void clearScreen() {
    lcdController.clear();
}

void beep(unsigned char delayms, unsigned char frequency, int pino){
    tone(buzzer, frequency);
    delay(delayms);
    noTone(buzzer);
    delay(delayms);
}

void beep(unsigned char delayms, int pino) {
    tone(buzzer, 1800);
    delay(delayms);
    noTone(buzzer);
    delay(delayms);
}

boolean readRampInput(char key, int input_size){
    if (key == 'A'||  key == 'C' || key == 'D') {
        status_menu     = key == 'A' ? MAIN_SCREEN  : status_menu;
        status_menu     = key == 'C' ? CLOCK_SCREEN : status_menu;
        status_menu     = key == 'D' ? PRE_RUN_SCREEN   : status_menu;
        input_value     = "";
        ramp_index      = 0;
        beep(200, buzzer);
        clearScreen();
    }
    if(isDigit(key)){
        if(input_value.length() < input_size){
            input_value += key;
            lcdController.print("         ",0,4);
            lcdController.print(input_value,0,4);
            beep(100, buzzer);
        }
    }else if(key == '#'){
        input_value = "";
        lcdController.print("         ",0,4);
        beep(30,buzzer);
    }else if(key == '*'){
        int v = input_value.toInt();
        if(v == 0 ){
            input_value = "";
            lcdController.print("invalido!",0,4);
            beep(50,buzzer);
            beep(50,buzzer);
            beep(50,buzzer);
        }else{
            return true;
        }
    }
    return false;
}

boolean readTimeInput(char key){
    if (key == 'A'|| key == 'B' || key == 'D') {
        status_menu     = key == 'A' ? MAIN_SCREEN      : status_menu;
        status_menu     = key == 'B' ? NUM_RAMP_SCREEN  : status_menu;
        status_menu     = key == 'D' ? PRE_RUN_SCREEN   : status_menu;
        input_value     = "";
        set_clock_index = 0;
        beep(200, buzzer);
        clearScreen();
    }
    int input_size = set_clock_index == 3 ? 1 : 2;
    if(isDigit(key)){
        if(input_value.length() < input_size){
           input_value += key;
           lcdController.print("         ",0,4);
           lcdController.print(input_value,0,4);
           beep(100, buzzer);
            
        }
    }else if(key == '#'){
        input_value = "";
        lcdController.print("         ",0,4);
        beep(30,buzzer);
    }else if(key == '*'){
        int v = input_value.toInt();
//        Serial.print(v);
//        Serial.print("|");
//        Serial.print(set_clock[set_clock_index][0]);
//        Serial.print("|");
//        Serial.println(set_clock[set_clock_index][1]);
        if(v < set_clock[set_clock_index][0] || v > set_clock[set_clock_index][1]){
            input_value = "";
            lcdController.print("invalido!",0,4);
            beep(50,buzzer);
            beep(50,buzzer);
            beep(50,buzzer);
        }else{
            return true;
        }
    }
    return false;
}

void readTemp() { //FUNCAO - FAZ A LEITURA DO SENSOR DE TEMPERATURA
    temp = thermocouple.readCelsius();
}

void ligarRele() {
    digitalWrite(rele, HIGH);
    //.println("ligou  " + String(temp)  + " | " + String(ramp_temp[ramp_index]));
}

void desligarRele() {
    digitalWrite(rele, LOW);
    //Serial.println("desligou  " + String(temp)  + " | " + String(ramp_temp[ramp_index]));
}

String twoDigits(int value){
    return value < 10 ? "0" + String(value) : String(value);
}

void runner(){
    if(run_start > 0 ){
        if(temp < ramp_temp[ramp_index]){
            ligarRele();
        }else{
            desligarRele();
        }
        long current  = millis() - run_start;;
        if(current / m_minute >= ramp_time[ramp_index] ){
            ramp_index++;
            run_start  = millis();
            beep(200,buzzer);
            beep(200,buzzer);
        }
        int days    =   current  / m_day;
        int hours   = ( current  % m_day) / m_hour;
        int minutes = ((current  % m_day) % m_hour) / m_minute ;
        int seconds = (((current % m_day) % m_hour) % m_minute) / m_second;
        run_hours   = twoDigits(hours);
        run_minutes = twoDigits(minutes);
        run_seconds = twoDigits(seconds);
        //Serial.println(run_hours + ":"+ run_minutes + ":"+ run_seconds);
        if(ramp_index == ramp_size){
            clearScreen();
            desligarRele();
            run_start   = 0;
            status_menu = FINISH_SCREEN;
        }

    }
}

void controller() {
    char keypressed = kpd.getKey();
    if(keypressed != NO_KEY){
        //Serial.println(keypressed);      
        switch (status_menu) {
            case MAIN_SCREEN :
                if (keypressed == 'B'|| keypressed == 'C' || keypressed == 'D') {
                    status_menu     = keypressed == 'B' ? NUM_RAMP_SCREEN  : status_menu;
                    status_menu     = keypressed == 'C' ? CLOCK_SCREEN     : status_menu;
                    status_menu     = keypressed == 'D' ? PRE_RUN_SCREEN   : status_menu;
                    beep(200, buzzer);
                    clearScreen();
                }
                break;
            case NUM_RAMP_SCREEN :
                if(readRampInput(keypressed,2)){
                    int v = input_value.toInt();
                    input_value    = "";
                    ramp_index     = 0;
                    ramp_size      = v;
                    ramp_temp      = new int[v];
                    ramp_time      = new int[v];
                    ramp_programed = false;
                    status_menu    = TIME_RAMP_SCREEN;
                    beep(200, buzzer);
                    clearScreen();
                }
                break;
            case TIME_RAMP_SCREEN :
                if(readRampInput(keypressed,3)){
                    int v = input_value.toInt();
                    input_value = "";
                    ramp_time[ramp_index] = v;
                    lcdController.print("                    ",0,4);
                    beep(100, buzzer);
                    ramp_index++;
                    if(ramp_index >= ramp_size){
                        status_menu = TEMP_RAMP_SCREEN;
                        ramp_index  = 0;
                        beep(200, buzzer);
                        clearScreen();
                    }
                }
                break;
            case TEMP_RAMP_SCREEN :
                if(readRampInput(keypressed,3)){
                    int v = input_value.toInt();
                    input_value = "";
                    ramp_temp[ramp_index] = v;
                    lcdController.print("                    ",0,4);
                    beep(100, buzzer);
                    ramp_index++;
                    if(ramp_index >= ramp_size){
                        ramp_programed = true;
                        status_menu    = MAIN_SCREEN;
                        beep(200, buzzer);
                        clearScreen();
                    }
                }
                break;
            case CLOCK_SCREEN :
                if(readTimeInput(keypressed)){
                    int v = input_value.toInt();
                    set_clock[set_clock_index][2] = v;
                    set_clock_index++;
                    input_value = "";
                    clearScreen();
                    beep(200, buzzer);
                    if(set_clock_index >= 7){
                        timeController.setDate(set_clock[0][2], set_clock[1][2], set_clock[2][2], set_clock[3][2], set_clock[4][2], set_clock[5][2], set_clock[6][2]);
                        status_menu     = MAIN_SCREEN;
                        set_clock_index = 0;
                        beep(200, buzzer);
                        beep(200, buzzer);
                        clearScreen();
                    }
                }
                break;
            case PRE_RUN_SCREEN :
                if(run_start == 0){
                    if (keypressed == 'A'|| keypressed == 'B' || keypressed == 'C') {
                        status_menu = keypressed == 'A' ? MAIN_SCREEN      : status_menu;
                        status_menu = keypressed == 'B' ? NUM_RAMP_SCREEN  : status_menu;
                        status_menu = keypressed == 'C' ? CLOCK_SCREEN     : status_menu;
                        input_value = "";
                        clearScreen();
                        beep(200, buzzer);
                    }
                    if(keypressed == 'D' && ramp_programed){
                        status_menu = RUN_SCREEN;
                        ramp_index  = 0;
                        run_start   = millis();
                        ligarRele();
                        beep(200, buzzer);
                        beep(200, buzzer);
                        beep(200, buzzer);
                        clearScreen();
                    }
                }else{
                    if(keypressed == 'D'){
                        run_start  = 0;
                        status_menu = MAIN_SCREEN;
                        desligarRele();
                        beep(200, buzzer);
                        clearScreen();
                    }
                }
                break;
            case RUN_SCREEN :
                if(keypressed == 'D'){
                    status_menu = BREAK_SCREEN;
                    beep(200, buzzer);
                    beep(100, buzzer);
                    clearScreen();
                }
                break;
            case BREAK_SCREEN :
                if(keypressed  == 'A'){
                    status_menu = MAIN_SCREEN;
                    ramp_index  = 0;
                    run_start   = 0;
                    desligarRele();
                    beep(200, buzzer);
                    beep(200, buzzer);
                    beep(200, buzzer);
                    clearScreen();
                }else if(keypressed == 'D'){
                    status_menu = RUN_SCREEN;
                    clearScreen();
                    beep(200, buzzer);
                }
            case FINISH_SCREEN :
                if (keypressed == 'A') {
                    status_menu = MAIN_SCREEN;
                    clearScreen();
                    beep(200, buzzer);
                }
                break;
        }
    }
}

void view() { //FUNCAO - ATUALIZA A TELA LCD
    switch (status_menu) {
        case MAIN_SCREEN :
            lcdController.print("CookBeer" , 1, 0, false);
            lcdController.write((uint8_t)5 , 0, 0, false); //burn
            
            lcdController.write((uint8_t)6 , 1, 0, false); //calend
            lcdController.print(timeController.getDate(),1,1);
            lcdController.print("B-Progr." ,12, 1);
            
            lcdController.write((uint8_t)3 , 2, 0, false); //clock
            lcdController.print(timeController.getTime(),1,2);
            lcdController.print("C-Calen." ,12, 2);
            
            lcdController.write((uint8_t)1 , 3, 0, false); //term
            lcdController.print(temp       , 1, 3);
            lcdController.write((uint8_t)2 , 3, temp < 100 ?  6 : 7, false); //Cels
            lcdController.print("D-Cozin." ,12, 3);
            break;
        case NUM_RAMP_SCREEN :
            lcdController.print("Num. Rampas [1-99]",0,0);
            lcdController.print("* Continuar",0,1);
            lcdController.print("# Limpar"   ,0,2);
            break;
        case TIME_RAMP_SCREEN:
            lcdController.print( "Durac. RAMP" +  String(ramp_index + 1) + " [1-999]",0,0);
            lcdController.print("* Continuar",0,1);
            lcdController.print("# Limpar"   ,0,2);
            break;
        case TEMP_RAMP_SCREEN:
            lcdController.print( "Tempe. RAMP" +  String(ramp_index + 1) + " [1-999]" ,0,0);
            lcdController.print("* Continuar",0,1);
            lcdController.print("# Limpar"   ,0,2);
            break;
        case CLOCK_SCREEN :
            if(set_clock_index == 0){
                lcdController.print("Infor Segundos[0-59]",0,0);
            }else if(set_clock_index == 1){
                lcdController.print("Inform Minutos[0-59]",0,0);
            }else if(set_clock_index == 2){
                lcdController.print("Informe Horas[1-23]",0,0);
            }else if(set_clock_index == 3){
                lcdController.print("Info Dia Semana[1-7]",0,0);
                lcdController.print("(1=Seg)",12,1);
                lcdController.print("(7=Dom)",12,2);
            }else if(set_clock_index == 4){
                lcdController.print("Inf Dia do Mes[1-31]",0,0);
            }else if(set_clock_index == 5){
                lcdController.print("Informe o Mes[1-12]",0,0);
            }else if(set_clock_index == 6){
                lcdController.print("Informe o Ano[0-99]",0,0);
            }
            lcdController.print("* Continuar",0,1);
            lcdController.print("# Limpar"   ,0,2);
            
            break;
        case PRE_RUN_SCREEN :
            if(ramp_programed){
                lcdController.print("Cozinhar?"  ,0,0);
                lcdController.print("D Confirmar",0,1);
            }else{
                lcdController.print("Precisa Programar...",0,0);
                lcdController.print("B Program",0,1);
            }
            break;
        case RUN_SCREEN :
            lcdController.print("RAMPA ",6 ,0);
            lcdController.print(String(ramp_index + 1) ,11,0);
            lcdController.write((uint8_t)3, 1, 0, false);
            lcdController.print(timeController.getTime(), 1, 1, false);
            lcdController.write((uint8_t)1, 2, 0, false);
            lcdController.write((uint8_t)2, 2, temp < 100 ? 6 : 7, false);
            lcdController.print(temp, 1, 2);
            lcdController.write((uint8_t)5, 3, 0, false);
            lcdController.print(String(ramp_temp[ramp_index]) + ".00",12,2);
            lcdController.write((uint8_t)2, 2, ramp_temp[ramp_index] < 100 ? 17 : 18, false);
            lcdController.print(run_hours + ":" + run_minutes + ":" + run_seconds ,1,3);
            lcdController.print(twoDigits(ramp_time[ramp_index] / 60) + ":" + twoDigits(ramp_time[ramp_index] % 60) + ":00" ,12,3);
            break;
        case BREAK_SCREEN :
            lcdController.print("Parar programa? ",0 ,0);
            lcdController.print("A para Parar",0 ,1);
            lcdController.print("D para Continuar",0 ,2);
            break;
        case FINISH_SCREEN :
            lcdController.print("******************",1 ,0);
            lcdController.print("*   TERMINADO    *",1 ,1);
            lcdController.print("*  COM SUCESSO!  *",1 ,2);
            lcdController.print("******************",1 ,3);
            beep(200,buzzer);
            break;
    }
}

void setup() {
    //Serial.begin(9600);
    pinMode(buzzer, OUTPUT);
    pinMode(rele,   OUTPUT);
    digitalWrite(rele,LOW);
    beep(200,1800, buzzer);
    beep(100,1100, buzzer);
    lcdController.init(20,4,"CookBeer V1.0", "Carregando");
    lcdController.createChar(1,term_icon);
    lcdController.createChar(2,degC_icon);
    lcdController.createChar(3,clock_icon);
    lcdController.createChar(4,bar_icon);
    lcdController.createChar(5,burn_icon);
    lcdController.createChar(6,calend_icon);
    threadKeyboard.onRun(controller);
    threadKeyboard.setInterval(0);
    threadTemp.onRun(readTemp);
    threadTemp.setInterval(600);
    threadLCD.onRun(view);
    threadLCD.setInterval(500);
    threadRun.onRun(runner);
    threadRun.setInterval(500);
    threadControl.add(&threadKeyboard);
    threadControl.add(&threadTemp);
    threadControl.add(&threadLCD);
    threadControl.add(&threadRun);
    beep(200,buzzer);
    kpd.begin();
    kpd.setDebounceTime(10); //keyboard delay
    //Serial.println("Inicio");    
}

void loop(){
    threadControl.run();
}
