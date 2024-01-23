#pragma once
#include <cstdint>
#include <sys/time.h>
#include <cstddef>

#define F(A) A
#define PIN_MAX 28

const int OUTPUT=0;
const int INPUT=1;
const int INPUT_PULLUP=2;
const int INPUT_PULLDOWN=3;

const int LOW=0;
const int HIGH=1;
uint64_t micros();
uint64_t millis();
bool pinMode(int pin,int  mode);
void digitalWrite(int pin,int  output);
int digitalRead(int pin);


class serial{
public:
   serial();
   void begin(int baudrate=115200);
   void print(const char *  thestring);
   void println(const char *  thestring);
   void print(int thenumber);
   void println(int thenumber);
   void println();

};

extern bool Pi5Flag;
extern serial Serial;
extern bool isPinExported(int pin);



/*
  pin   GPIO
  xxx	00	xxx
  xxx	01	xxx
  03	02	relaisVentIntPv
  05	03      SDA
  07	04      SCL
  29	05      relaiComp
  31	06      relaisVentIntGv
  26	07      ledca     SPI_CE1
  24	08      ledFr     SPI_CE0
  21	09      ledCh     SPI_MISO
  19	10                SPI_MOSI
  23	11                SPI_CLK
  32	12
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


