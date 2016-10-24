#include "DS18B20.h"
#include "Arduino.h"

/**
 * Constructor
 */
DS18B20::DS18B20(OneWire* _sensor)
{
  _Sensor = _sensor;
  devices = 0;

  begin();
}

/**
 * Busca los sensores válidos en el bus y aumenta el
 * contador de número de dispositivos por cada nuevo
 */
void DS18B20::begin()
{
  uint8_t deviceAddress[8];

  _Sensor->reset_search();
  devices = 0;

  while(_Sensor->search(deviceAddress))
  {
    if(validAddress(deviceAddress))
    {
      devices++;
    }
  }
}

/**
 * Retorna el número de dispositivos encontrados
 */
uint8_t DS18B20::GetDevicesCount()
{
  return devices;
}

/**
 * Verifica que la dirección del sensor sea válida
 */
bool DS18B20::validAddress(uint8_t* address)
{
  return (_Sensor->crc8(address, 7) == address[7]);
}

/**
 * Obtiene la dirección del sensor
 */
bool DS18B20::GetDeviceAddress(uint8_t* address, uint8_t index)
{
  uint8_t counter = 0;

  _Sensor->reset_search();

  while(counter <= index && _Sensor->search(address))
  {
    if(counter == index && validAddress(address))
    {
      return true;
    }
    counter++;
  }

  _Sensor->reset_search();
  return false;
}

/**
 * Obtiene la familia del sensor
 */
uint8_t DS18B20::GetFamily(uint8_t* address)
{
  uint8_t type_s = NOVALIDSENSOR;
  
  switch (address[0])
  {
    case DS18S20MODEL:
      type_s = DS18S20SENSOR;
      break;
    case DS18B20MODEL:
      type_s = DS18B20SENSOR;
      break;
    case DS1822MODEL:
      type_s = DS1822SENSOR;
      break;
    case DS1825MODEL:
      type_s = DS1825SENSOR;
      break;
  }

  return type_s;
}

/**
 * Espera a que 1 sensor termine de leer temperatura, la
 * tarea es asíncrona
 */
bool DS18B20::waitForConvertion()
{
  unsigned long now = millis();
  while(millis() - WAIT < now);
}

/**
 * Obtiene el valor crudo (en bytes) de temperatura
 */
int16_t DS18B20::getRawTemp(uint8_t* address)
{
  byte present = 0;
  byte data[12];

  _Sensor->search(address);
  _Sensor->reset();
  _Sensor->select(address);
  _Sensor->write(STARTCONV, 1);

  waitForConvertion();

  present = _Sensor->reset();
  _Sensor->select(address);
  _Sensor->write(READSCRATCH);

  for(int i = 0; i < 9; i++)
  {
    data[i] = _Sensor->read();
  }

  _Sensor->crc8(data, 8);

  int16_t raw = (data[1] << 8) | data[0];

  if(GetFamily(address) == DS18S20SENSOR)
  {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  }
  else
  {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }

  return raw;
}

/**
 * Convierte el valor en bytes de temperatura (en grados 
 * centígrados) a flotante
 */
float DS18B20::GetTempC(uint8_t* address)
{
  return (float)getRawTemp(address) / 16.0;
}

/**
 * Vacía el bufer de lectura/escritura del sensor
 */
void DS18B20::Flush()
{
  _Sensor->reset_search();
  delay(250);
}

