#include "arduinoEmul.hpp"
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <gpiod.h>
using namespace std;

// gpiod

struct gpiod_chip  *gpiochip;
struct gpiod_line  *gpioline[PIN_MAX];


uint64_t micros()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}


uint64_t millis()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000+(tv.tv_usec / 1000);
}

bool Pi5Flag=false;

bool isPinExported(int pin)
{
    int  gpiopin;
    struct stat sb;
    // est-ce un Pi5
    Pi5Flag=(stat("/sys/class/gpio/gpiochip399", &sb) == 0);
    if(Pi5Flag)
        cout << "gpiochip399 existd!" << endl;
    else
        cout << "gpiochip399 doesn't existd!" << endl;
    // est-ce deja exporte
    gpiopin = Pi5Flag ? (399+pin) : pin;
        cout << "verifie " << gpiopin << endl;
    return (stat(("/sys/class/gpio/gpio" + to_string(gpiopin)).c_str(), &sb) == 0);
}



bool  init_gpiod()
{
  gpiochip = gpiod_chip_open_by_name("gpiochip4");

  if(gpiochip == NULL)
      gpiochip = gpiod_chip_open_by_name("gpiochip0");

  if(gpiochip == NULL)
      {
           printf("unable to open GPIO\n");
           return false;
      }
   for(int loop;loop<PIN_MAX;loop++)
     gpioline[loop]=NULL;
   return true;
}



bool  pinMode(int pin,int  mode)
{
  bool flag=false;
  if(pin<0) return false;
  if(pin>=PIN_MAX) return false;

  if(gpiochip==NULL)
     init_gpiod();
  if(gpiochip==NULL)
      return false;

  // Pin initialise
  if(gpioline[pin]!=NULL)
      gpiod_line_release(gpioline[pin]);

   gpioline[pin] = gpiod_chip_get_line(gpiochip,pin);

  if(gpioline[pin] == NULL)
      return false;

  switch(mode)
  {

   case OUTPUT:
                flag=gpiod_line_request_output(gpioline[pin],"ardEmul",0)==0;
                break;
   case INPUT:
                flag=gpiod_line_request_input(gpioline[pin],"ardEmul")==0;
                break;

   case INPUT_PULLUP:
                flag=gpiod_line_request_input_flags(gpioline[pin],"ardEmul",GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP)==0;
                break;

   case INPUT_PULLDOWN:
                flag=gpiod_line_request_input_flags(gpioline[pin],"ardEmul",GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_DOWN)==0;
                break;
  }
   return flag;

  // besoin de configurer la broche
}

void release_gpiod(void)
{
  if(gpiochip!=NULL)
  {
   for(int loop=0;loop<PIN_MAX;loop++)
    if(gpioline[loop]!=NULL)
        {
          gpiod_line_release(gpioline[loop]);
          gpioline[loop]=NULL;
        }
   gpiod_chip_close(gpiochip);
   gpiochip=NULL;
  }
}



void digitalWrite(int pin,int  output)
{
   if(pin >= PIN_MAX)
      return;
    if(gpioline[pin]==NULL)
        pinMode(pin,OUTPUT);
    gpiod_line_set_value(gpioline[pin],output);
}


int digitalRead(int pin)
{
   if(pin >= PIN_MAX)
      return -1;

    if(gpioline[pin]==NULL)
        pinMode(pin,INPUT_PULLUP);
    return gpiod_line_get_value(gpioline[pin]);
}



serial::serial()
{

}

void serial::begin(int baudrate)
{

}

void serial::print(const char *  thestring)
{
//
}

void serial::println(const char *  thestring)
{
//
}

void serial::print(int  thestring)
{
//
}

void serial::println(int  thestring)
{
//
}

void serial::println()
{
//
}

serial Serial;
