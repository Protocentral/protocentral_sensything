#ifndef SENSYTHING_ADS1220_H
#define SENSYTHING_ADS1220_H

volatile unsigned int HR ,RR ;

xQueueHandle rgbLedSem;
QueueHandle_t xQueue_tcp;

#define HIGH 1
#define LOW 0

#define ADS1220_CS_PIN 4		
#define ADS1220_DRDY_PIN 34

#define ADS1220_MOSI 23
#define ADS1220_MISO 19
#define ADS1220_SCLK 18

#define CONFIG_REG0_ADDRESS 0x00
#define CONFIG_REG1_ADDRESS 0x01
#define CONFIG_REG2_ADDRESS 0x02
#define CONFIG_REG3_ADDRESS 0x03
#define WREG  0x40
#define RREG 0x20
#define ADS1220_CMD_RDATA    	0x10
#define START 0x08 

#define PGA 1
#define VREF 2.048 
#define VFSR VREF/PGA             
#define FSR (((long int)1<<23)-1) 
//#define FSR 16777215 

void sensything_ads1220initchip(void);
void sensything_ads1220writeregister(uint8_t address, uint8_t value);
void sensything_ads1220reg_read(unsigned char WRITE_ADDRESS);

void sensything_ads1220sendcmd(uint8_t command);
float sensything_ads1220readdata(void );
void sensything_ads1220start(void);
uint8_t * get_ads1220data(void);

#endif
