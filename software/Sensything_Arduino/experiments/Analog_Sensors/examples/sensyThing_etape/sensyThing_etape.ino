
#include <SPI.h> 
#include "Protocentral_ADS1220.h"

//--ads1220--------------------------------------------------
#define PGA 1                 // Programmable Gain = 1
#define VREF 2.048            // Internal reference of 2.048V
#define VFSR VREF/PGA             
#define FSR (((long int)1<<23)-1)  

volatile byte MSB;
volatile byte data;
volatile byte LSB;
//volatile char SPI_RX_Buff[3];
volatile byte *SPI_RX_Buff_Ptr;

Protocentral_ADS1220 ADS1220;

void setup()
{
 pinMode(ADS1220_CS_PIN, OUTPUT);
 pinMode(ADS1220_DRDY_PIN, INPUT);
  
 ADS1220.begin();
  
}

void loop()
{
  long int bit32;
  long int bit24;
  byte *config_reg;
  
  //if((digitalRead(ADS1220_DRDY_PIN)) == LOW)             //        Wait for DRDY to transition low
  { 
    SPI_RX_Buff_Ptr = ADS1220.Read_Data();
  }

  if(ADS1220.NewDataAvailable = true)
  {
  ADS1220.NewDataAvailable = false;

  MSB = SPI_RX_Buff_Ptr[0];    
  data = SPI_RX_Buff_Ptr[1];
  LSB = SPI_RX_Buff_Ptr[2];

  bit24 = MSB;
  bit24 = (bit24 << 8) | data;
  bit24 = (bit24 << 8) | LSB;                                 // Converting 3 bytes to a 24 bit int
    
  bit24= ( bit24 << 8 );
  bit32 = ( bit24 >> 8 );                      // Converting 24 bit two's complement to 32 bit two's complement
   
  float Vout = (float)((bit32*VFSR*1000)/FSR);     //In  mV
  Serial.print("Vout: "); 
  Serial.println(Vout);
  float reading = (1023 / Vout)  - 1;
  reading = 560 / reading;
  Serial.print("Sensor resistance "); 
  Serial.println(reading);

  }
}

