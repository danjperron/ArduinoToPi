#pragma once
#include "arduinoEmul.hpp"

class hd44780{
 public:
   static   void fatalError(int result);

};


class hd44780_I2Cexp {
    public:
         hd44780_I2Cexp();
         void createChar(int nchar,unsigned char  * array);
         void setCursor(int pos, int ligne);
         void print(const char * bufferEtiquette);
         void print(char C);
         int  begin(int nbColonnes, int nbLignes);
         void noBacklight();
         void backlight();
         void clear();
         void write(char C);
 };
