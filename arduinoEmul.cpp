#include "arduinoEmul.hpp"
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <gpiod.h>
#include <unistd.h>

using namespace std;

// gpiod

struct gpiod_chip  *gpiochip;
struct gpiod_line  *gpioline[PIN_MAX];


int micros()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (int)(tv.tv_sec*(uint64_t)1000000+tv.tv_usec);
}


int millis()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (int) (tv.tv_sec*(uint64_t)1000+(tv.tv_usec / 1000));
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

   case OPENDRAIN_PULLUP:
                flag=gpiod_line_request_output_flags(gpioline[pin],"ardEmul",
                                       GPIOD_LINE_REQUEST_FLAG_OPEN_DRAIN|GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP,1)==0;

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
  deviceHandle=-1;
  console=false;
}



serial::~serial()
{
    if(deviceHandle >=0)
      close(deviceHandle);
}





void serial::begin(int baudrate, char * DeviceName,bool Console)
{

   if(deviceHandle>=0)
       close(deviceHandle);
   console = Console;
   if(Console) return;

   deviceHandle=open(DeviceName,O_WRONLY);
   if(deviceHandle >=0)
    {
     if(tcgetattr(deviceHandle,&termiosStruct)<0)
      {
         close(deviceHandle);
         deviceHandle=-1;
      }
     else
     {
      termiosStruct.c_cflag &= ~PARENB;
      termiosStruct.c_cflag &= ~CSTOPB;
      termiosStruct.c_cflag &= ~CSIZE;
      termiosStruct.c_cflag |= CS8;
      termiosStruct.c_cflag &= ~CRTSCTS;
      termiosStruct.c_cflag |= CREAD | CLOCAL;
      termiosStruct.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
      termiosStruct.c_oflag &= ~OPOST;
      termiosStruct.c_cc[VMIN]  = 0;
      termiosStruct.c_cc[VTIME] = 0;
      cfsetospeed(&termiosStruct,baudrate);
      if(tcsetattr(deviceHandle,TCSANOW,&termiosStruct)<0)
            {
             close(deviceHandle);
             deviceHandle=-1;
            }
     }
  }
}

// by default is console
void serial::begin()
{
   begin(0,NULL,true);
}


void serial::print(const char thechar)
{
   if(console)
     cout << thechar;
   else
     if(deviceHandle>=0)
           write(deviceHandle,&thechar,1);
}


void serial::print(const char *  thestring)
{
   if(console)
     cout << thestring;
   else
     if(deviceHandle>=0)
           write(deviceHandle,thestring,strlen(thestring));
}

void serial::println(const char *  thestring)
{
    print(thestring);
    print('\n');
}

void serial::print(float  thefloat)
{
    char buffer[256];
     if(console)
     cout << thefloat;
   else
     if(deviceHandle>=0)
       {
         sprintf(buffer,"%f",thefloat);
         print(buffer);
       }
}

void serial::println(float  thefloat)
{
   print(thefloat);
   print('\n');
}

void serial::print(int   thenumber)
{
    char buffer[256];
     if(console)
     cout << thenumber;
   else
     if(deviceHandle>=0)
       {
         sprintf(buffer,"%d",thenumber);
         print(buffer);
       }
}


void serial::println(int  thenumber)
{
    print(thenumber);
    print('\n');
}

void serial::println()
{
    print('\n');
}

serial Serial;
