#ifndef DS18B20_h
#define DS18B20_h

#include <OneWire.h>

#define WAIT 750

#define STARTCONV       0x44  // Informa al dispostivo que se haran lecturas de temperatura
#define READSCRATCH     0xBE  // Lee la EEPROM
#define WRITESCRATCH    0x4E  // Escribe la EEPROM
#define RECALLSCRATCH   0xB8  // Recarga desde el último conocido
#define READPOWERSUPPLY 0xB4  // Determina si el dispositivo necesita una carga parásita
#define ALARMSEARCH     0xEC  // Bus de consulta para los dispositivos que manejan alarma

#define DS18S20SENSOR 0x01    // Sensor DS18S20
#define DS18B20SENSOR 0x02    // Sensor DS18B20**
#define DS1822SENSOR  0x03    // Sensor DS1822
#define DS1825SENSOR  0x04    // Sensor DS1825
#define NOVALIDSENSOR  0x04   // Sensor no válido

#define DS18S20MODEL 0x10     // Modelo DS18S20
#define DS18B20MODEL 0x28     // Modelo DS18B20
#define DS1822MODEL  0x22     // Modelo DS1822
#define DS1825MODEL  0x3B     // Modelo DS1825

class DS18B20
{
  public:
    DS18B20(OneWire*);            // Constructor
    uint8_t GetDevicesCount();    // Obtiene el número de dispositivos en el Bus
    bool GetDeviceAddress(uint8_t* address, uint8_t index); // Obtiene la dirección del dispositivo
    uint8_t GetFamily(uint8_t* address);  // Obtiene la familia del sensor
    float GetTempC(uint8_t*);     // Obtiene la temperatura en *C
    void Flush();                 // Vacía el búfer
    
  private:
    uint8_t devices;             // Dispositivos
    OneWire* _Sensor;            // Recurso de OneWire
    void begin(void);            // Inicializa el sensor
    bool validAddress(uint8_t* address);  // Verifica que la dirección sea válida
    bool waitForConvertion();    // Espera a que el sensor devuelva el valor de temperatura ** Asíncrono
    int16_t getRawTemp(uint8_t* address); // Obtiene el valor crudo (en bytes) de la temperatura
};
#endif
