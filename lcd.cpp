#include "lcd.hpp"


void hd44780::fatalError(int result)
{


}



hd44780_I2Cexp::hd44780_I2Cexp()
 {
  //   do nothing for now
 }

void hd44780_I2Cexp::createChar(int nchar,unsigned char * array)
{
 // need to fill

}

void hd44780_I2Cexp::setCursor(int pos, int ligne)
{
  // 
}

void hd44780_I2Cexp::print(const char * bufferEtiquette)
{
  ///
}

void hd44780_I2Cexp::print(char C)
{
  ///
}

void hd44780_I2Cexp::write(char C)
{
  ///
}

int  hd44780_I2Cexp::begin(int nbColonnes,int  nbLignes)
{
///
   return 1;
}

void hd44780_I2Cexp::noBacklight()
{
///
}

void hd44780_I2Cexp::backlight()
{

///
}

void hd44780_I2Cexp::clear()
{

///
}
