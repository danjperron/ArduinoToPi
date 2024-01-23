#include "sonde.hpp"

Sonde::Sonde(const int pinNumber, const char* nom, const uint32_t periodeMaJ)
{
  this->pinNumber = pinNumber;
  this->nom = nom;
  _periodeMaJ = periodeMaJ;
  _chrono = -periodeMaJ;
  _temperature = -127.0;
}

void Sonde::begin() {

//  _capteur.setResolution(12);
//  _capteur.setWaitForConversion(false);
//  _capteur.begin();
//  _capteur.requestTemperatures();
}

float Sonde::temperature() {
/*  if (millis() - _chrono >= _periodeMaJ) {
    if (_capteur.isConversionComplete()) {
      _temperature = _capteur.getTempCByIndex(0);
      _capteur.requestTemperatures();
      _chrono = millis(); // Mettre à jour le chrono
      _temperature = floor(_temperature + ((_temperature - floor(_temperature) < 0.50) ? 0.50 : 1.0));
    }
  }
*/
  return _temperature;
}
