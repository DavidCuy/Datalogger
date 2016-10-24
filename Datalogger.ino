/*
 * Se necesita instalar las librerías OneWire y LowPower
 * se puede descargar desde http://playground.arduino.cc/Learning/OneWire
 * y https://github.com/rocketscream/Low-Power
 * se descomprime la carpeta en la carpeta de librerías de Arduino, por lo regular
 * es la ruta C:\Program Files (x86)\Arduino\libraries
 * David Cuy
 *
 * PINES de conexión del arduino
 * A4 - SDA del RTC
 * A5 - SCL del RTC
 * 9  - Slave Select (SS) de la microSD
 * 11 - MOSI de la microSD
 * 12 - MISO de la microSD
 * 13 - SCK de la microSD
 * 8  - Pin de entrada de los sensores de temperatura DS18B20
 */

#include <SPI.h>        /// Librería para uso de protocolo SPI, usada para leer la microSD
#include <SD.h>         /// Librería para manejar tarjetas SD - MicroSD.
#include <OneWire.h>    /// Librería del protocolo OneWire que usa el sensor DS18B20
#include "DS18B20.h"    /// Librería del sensor de temperatura DS18B20, debe estar en la misma carpeta que el archino Datalogger.ino
#include "Wire.h"       /// Librería I2C para el Reloj de tiempo real (RTC) DS1307
#include "LowPower.h"   /// Libreria para entrar en Modo Sleep

#define RTC_ADDRESS 0x68   /// Dirección I2C del RTC

/// Estructura para manejar de manera más cómoda el RTC
struct Date{
  byte second;
  byte minute;
  byte hour;
  byte dayOfWeek;
  byte dayOfMonth;
  byte month;
  byte year;
};

OneWire ow(8);        /// Recurso del protocolo OneWire
DS18B20 Sensor(&ow);  /// Se inicializa la entrada del Sensor DS18B20, se la dirección de memoria del recurso OneWire
Date today;

uint8_t NumberOfDevices = 0;            /// Número de sensores de temperatura utilizados
uint8_t Devices[8][2];                  /// Dirección de cada uno de los sensores de temperatura (2 sensores) cada uno de 8 bytes
const int chipSelect = 9;               /// Chip select utilizado para la activacion de la MicroSD card
byte minutoComp = 36;
byte horaComp = 10;
byte date[] = {0, minutoComp, horaComp, 3, 18, 10, 16}; /// Segundo, minutos, horas, día de la semana (Sun=1), día del mes, mes, año, 0x10 set 1HZ square wave

byte decToBcd(byte val);                /// Convierte valor decimal a BCD
byte bcdToDec(byte val);                /// Convierte valor BCD a decimal
void setRTCtime(byte* date);         /// Inicia el reloj en el RTC
void readRTCtime(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year);  /// Lee el valor actual del RTC
String displayTime();                   /// Devuelve una cadena con formato de fecha

int ledTestigo = 3;
int ledError = 2;
bool error = false;

int count = 0;
//---------------------VOID SETUP()--------------------------------------
void setup() {
  Wire.begin();
  //Serial.begin(9600);
  pinMode(ledTestigo, OUTPUT);
  pinMode(ledError,OUTPUT);

  //setRTCtime(date);

  while(!SD.begin(chipSelect)){
    //Serial.println("Card failed, or not present");
    digitalWrite(ledError,HIGH);
    delay(2000);
    digitalWrite(ledError,LOW);
    delay(1000);
    count++;
  }

  delay(1000);
  //Serial.println(displayTime());
  readRTCtime(&today);
  horaComp = today.hour;
  minutoComp = today.minute;

  NumberOfDevices = Sensor.GetDevicesCount(); // La librería tiene capacidad de devolver los números de dispositivos conectados al bus
  //Serial.print("Number of sensors connected: ");
  //Serial.println(NumberOfDevices);

  Sensor.Flush();
  delay(2000);
}//---------------------FIN VOID SETUP()--------------------------------------
//----------------------------------------------------------------------------

//---------------------VOID LOOP()--------------------------------------------
void loop() {
  do{
    readRTCtime(&today);
    delay(1000);
    digitalWrite(ledTestigo,LOW);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    digitalWrite(ledTestigo,HIGH);
    delay(1000);

    /*if(count >= 6)
    {
      //Serial.println("Error");
      //Serial.println(displayTime());
      //Serial.print("SetPoint: ");
      //Serial.println(minutoComp);
      digitalWrite(ledError,HIGH);
      count == 0;
    }
    else
      count++;*/

  //}while(today.minute != minutoComp);
  }while(today.hour != horaComp);
  //Serial.println(displayTime());

  Lectura_Guardado();
  count = 0;

  /*if  (minutoComp < 59) {  minutoComp++;   }
  else { minutoComp = 0; }*/

  if(horaComp <23) { horaComp++;}
  else {horaComp = 0; }

} //---------------------FIN VOID LOOP()--------------------------------------
//----------------------------------------------------------------------------

void Lectura_Guardado(){
  digitalWrite(ledError,HIGH);
  // Imprime el valor de temperatura obtenida por cada uno de los sensores ------------
  //Serial.println(displayTime());
  String dataString = "";
  dataString += displayTime();
  dataString += ",";

  for(int i = 0; i < NumberOfDevices; i++)
  {
    float temp = Sensor.GetTempC(Devices[i]);

    /*Serial.print("Device["); Serial.print(i); Serial.print("]:");
    for(int j = 0; j < 8; j++)
    {
      Serial.write(' ');
      Serial.print(Devices[i][j], HEX);
    }
    //Serial.println();
    Serial.print("   ---   Temperatura: ");
    Serial.print(temp); Serial.println(" °C");
    */

    dataString += String(temp);     // Se almacena los datos en un string para escribir posteriormente en la SD
    if (i < (NumberOfDevices - 1)) {
      dataString += ",";
    }
  }//Fin FOR --- Almaceno los valores de temp en la variable "dataString" con el formato
  //------------------------------------------------------------------
  Sensor.Flush();

  File daFile = SD.open("Datos.csv",FILE_WRITE);

  if(daFile)         // Si existe el archivo, agrega la línea de los valores de temperatura
  {
    daFile.println(dataString);
    daFile.close();
  }
  else {
    for(int i = 0; i < 20; i++)
    {
      digitalWrite(ledError,HIGH);
      delay(500);
      digitalWrite(ledError,LOW);
      delay(500);
    }
  }
    digitalWrite(ledError,LOW);
  }//----FIN void Lectura_Guardado()-----


//---------------------FUNCIONES--------------------------------
//--------------------------------------------------------------
/*El RTC maneja formato BCD, por tanto se necesita la funcion para poder escribir una fecha */
byte decToBcd(byte val) { return( (val/10*16) + (val%10) ); }

//-----------------------------------
/*El RTC maneja formato BCD, por tanto se necesita la función para poder leer fechas */
byte bcdToDec(byte val){ return( (val/16*10) + (val%16) ); }

//-----------------------------------
/*Escribe o cambia el valor del RTC, leer la hoja de datos */
void setRTCtime(byte*date) {
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  for(int i = 0; i < 7; i++)
  {
    Wire.write(decToBcd(date[i]));
  }
  //Wire.write(0x01); //Wire.write(0x10);
  Wire.endTransmission();
}

//-----------------------------------
void readRTCtime(Date* today)
{
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(RTC_ADDRESS, 7);

  today->second = bcdToDec(Wire.read() & 0x7f);
  today->minute = bcdToDec(Wire.read());
  today->hour = bcdToDec(Wire.read() & 0x3f);
  today->dayOfWeek = bcdToDec(Wire.read());
  today->dayOfMonth = bcdToDec(Wire.read());
  today->month = bcdToDec(Wire.read());
  today->year = bcdToDec(Wire.read());
}

//-----------------------------------
String displayTime()
{
  String strTime = "";
  Date today;
  readRTCtime(&today);

  strTime += today.dayOfMonth;
  strTime += "-";
  strTime += today.month;
  strTime += "-";
  strTime += today.year;
  strTime += ",";
  strTime += today.hour;
  strTime += ":";
  strTime += today.minute;
  strTime += ":";
  strTime += today.second;
  strTime.trim();

  return strTime;
}//-------fin displayTime----------------------------
