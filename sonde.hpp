#pragma once
#include <cstddef>
#include <cstdint>
#include "arduinoEmul.hpp"


class Sonde {
  public:
    Sonde(const int pinNumber,const char* nom, const uint32_t periodeMaJ = 250);
    void begin();
    float temperature();
    int pinNumber;
    const char* nom;

  private:
    float _temperature;
    uint32_t _chrono;
    uint32_t _periodeMaJ;
};

