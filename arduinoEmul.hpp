#pragma once
#include <cstdint>
#include <sys/time.h>
#include <cstddef>
#include <fstream>
#include <termios.h>

#define F(A) A
#define PIN_MAX 28

const int OUTPUT=0;
const int INPUT=1;
const int INPUT_PULLUP=2;
const int INPUT_PULLDOWN=3;
const int OPENDRAIN_PULLUP=4 ;
const int LOW=0;
const int HIGH=1;
int micros();
int millis();
bool pinMode(int pin,int  mode);
void digitalWrite(int pin,int  output);
int digitalRead(int pin);


class serial{
public:
   serial();
   ~serial();
   void begin(int baudrate,char * DeviceName,bool Console=false);
   void begin();  // console by default
   void print(const char *  thestring);
   void println(const char *  thestring);
   void print(float thefloat);
   void println(float thefloat);
   void print(int thenumber);
   void println(int thenumber);
   void println();
   void print(char thechar);
private:
   int deviceHandle;
   struct termios termiosStruct;
   bool console;
};

extern struct gpiod_line  *gpioline[PIN_MAX];
extern serial Serial;
extern bool isPinExported(int pin);



/*
  pin   GPIO
  xxx	00	xxx
  xxx	01	xxx
  03	02	SDA
  05	03      SCL
  07	04      W1   ds18b20
  29	05      relaiComp
  31	06      relaisVentIntGv
  26	07      ledca     SPI_CE1
  24	08      ledFr     SPI_CE0
  21	09      ledCh     SPI_MISO
  19	10                SPI_MOSI
  23	11                SPI_CLK
  32	12       relaiVentIntPv
  33	13       relaiVentUnitExt
  08	14       TX
  10	15       RX
  36	16	relaisVentExt
  11	17      ledBoutonPlus
  12	18      ledBoutonMoins
  35	19      relaisVentUniteInt
  38	20	relaisVentInt
  40	21      ledBoutonValid
  15	22	menuPin
  16	23	validPin
  18	24	boutonPlus
  22	25	boutonMoins
  37	26	relaiV4V
  13	27      ledBoutonMenu


*/


bool init_gpiod();
void release_gpiod();


