#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "bt.h"
#include "driver/ledc.h"
#include "driver/uart.h"
#include "sensything_ads1220.h"
#include "sensything_ble.h"

spi_device_handle_t spi;
char spi_buff[4];
uint8_t SPI_RX_Buff[15];
uint8_t SPI_TX_Buff[15];
uint8_t SPI_temp_32b[4];
 	
volatile int32_t  ads1220_data_24;	
volatile int32_t ads1220_data_32;
volatile uint8_t msb, lsb, data;
float v_avg;
float channel4, channel3, channel2, channel1;

uint8_t ads1220_data_array[16] = {0};

void ads1220_spi_pre_transfer_callback(spi_transaction_t *t)
{
  ;
}

void sensything_ads1220reg_read(unsigned char WRITE_ADDRESS)
{
	gpio_set_level(ADS1220_CS_PIN, LOW);
  vTaskDelay(2 / portTICK_PERIOD_MS);

  SPI_TX_Buff[0] = RREG|(WRITE_ADDRESS<<2);
  SPI_TX_Buff[1]=0x00;
  SPI_TX_Buff[2]=0x00;
  SPI_TX_Buff[3]=0x00;

  esp_err_t ret;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));       //Zero out the transaction

  t.length=16;
  t.rxlength=16;
  t.tx_buffer=&SPI_TX_Buff;
  t.rx_buffer=&SPI_RX_Buff;

  t.user=(void*)0;
  ret=spi_device_transmit(spi, &t);
  assert(ret==ESP_OK);            //Should have had no issues.

  SPI_temp_32b[0] = SPI_RX_Buff[1];
  SPI_temp_32b[1] = SPI_RX_Buff[2];
  SPI_temp_32b[2] = SPI_RX_Buff[3];
	
	gpio_set_level(ADS1220_CS_PIN, HIGH);
	vTaskDelay(2 / portTICK_PERIOD_MS);
}


void sensything_ads1220writeregister(uint8_t address, uint8_t value)
{

  uint8_t txData[4];

  gpio_set_level(ADS1220_CS_PIN, LOW);
  vTaskDelay(2 / portTICK_PERIOD_MS);
	
  esp_err_t ret;
  spi_transaction_t ads1220;

  txData[0] = (WREG|(address<<2));
  txData[1] = value;

  memset(&ads1220, 0, sizeof(ads1220));       //Zero out the transaction

  ads1220.length=16;                 //Len is in bytes, transaction length is in bits.
  ads1220.tx_buffer=&txData;               //Data
  ret=spi_device_transmit(spi, &ads1220);  //Transmit!
  assert(ret==ESP_OK);            //Should have had no issues.
		
  gpio_set_level(ADS1220_CS_PIN, HIGH);
  vTaskDelay(2 / portTICK_PERIOD_MS);
} 

void sensything_ads1220sendcmd(uint8_t command)
{

  uint8_t txData[4];

  gpio_set_level(ADS1220_CS_PIN, LOW);
  vTaskDelay(2 / portTICK_PERIOD_MS);
	
  esp_err_t ret;
  spi_transaction_t ads1220;

  txData[0] = command;
  memset(&ads1220, 0, sizeof(ads1220));       //Zero out the transaction

  ads1220.length=8;                 //Len is in bytes, transaction length is in bits.
  ads1220.tx_buffer=&txData;               //Data
  ret=spi_device_transmit(spi, &ads1220);  //Transmit!
  assert(ret==ESP_OK);            //Should have had no issues.
		
  gpio_set_level(ADS1220_CS_PIN, HIGH);
  vTaskDelay(2 / portTICK_PERIOD_MS);
} 
 
float sensything_ads1220readdata(void )
{  
	gpio_set_level(ADS1220_CS_PIN, LOW);
  vTaskDelay(2 / portTICK_PERIOD_MS);

  SPI_TX_Buff[0] = 0xff;
  SPI_TX_Buff[1] = 0xFF;
  SPI_TX_Buff[2] = 0xFF;
  SPI_TX_Buff[3] = 0xFF;

  esp_err_t ret;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));       //Zero out the transaction

  t.length=24;
  t.rxlength=24;
  t.tx_buffer=&SPI_TX_Buff;
  t.rx_buffer=&SPI_RX_Buff;

  t.user=(void*)0;
  ret=spi_device_transmit(spi, &t);
  assert(ret==ESP_OK);            //Should have had no issues.

  ads1220_data_24 = (int32_t) SPI_RX_Buff[0];
  ads1220_data_24 = (int32_t) ((ads1220_data_24 << 8) | SPI_RX_Buff[1]);
  ads1220_data_24 = (int32_t)((ads1220_data_24 << 8) | SPI_RX_Buff[2]);
		   
	ads1220_data_24 = ads1220_data_24 << 8 ;
	ads1220_data_32 = ads1220_data_24 >> 8;
	
	float vout = (float)((ads1220_data_32*VFSR*1000)/FSR); 	
	//printf("vout in mV : %f \n",vout);	// //In  mV

/*	
	float bar = (vout/10.3963)-2.36;
  float  psi = vout/0.7168 ; 
	float mmHg = psi*51.71;
	updateads1220_ble(bar);
  printf("differential pressure : %f \n",bar);  // //In  mV
*/
	gpio_set_level(ADS1220_CS_PIN, HIGH);
	vTaskDelay(2 / portTICK_PERIOD_MS);

  return vout;
}

void sensything_ads1220read_channel1(void){

  sensything_ads1220writeregister( CONFIG_REG0_ADDRESS , 0x81);
  
  while(gpio_get_level(ADS1220_DRDY_PIN)){
    //stay here untill drdy line goes low
  }
  
  channel1 = sensything_ads1220readdata();
  printf("channel1 : %f \n",channel1);
  
  int32_t ble_vout = (int32_t) (channel1 * 100);
  ads1220_data_array[0] = (uint8_t) ble_vout;
  ads1220_data_array[1] = (uint8_t) (ble_vout >> 8);
  ads1220_data_array[2] = (uint8_t) (ble_vout >> 16);
  ads1220_data_array[3] = (uint8_t) (ble_vout >> 24);

}

void sensything_ads1220read_channel2(void){

  sensything_ads1220writeregister( CONFIG_REG0_ADDRESS , 0x91);
  
  while(gpio_get_level(ADS1220_DRDY_PIN)){
    //stay here untill drdy line goes low
  }

  channel2 = sensything_ads1220readdata();
  printf("channel2 : %f \n",channel2);

  int32_t ble_vout = (int32_t) (channel2 * 100);
  ads1220_data_array[4] = (uint8_t) ble_vout;
  ads1220_data_array[5] = (uint8_t) (ble_vout >> 8);
  ads1220_data_array[6] = (uint8_t) (ble_vout >> 16);
  ads1220_data_array[7] = (uint8_t) (ble_vout >> 24);

}

void sensything_ads1220read_channel3(void){

  sensything_ads1220writeregister( CONFIG_REG0_ADDRESS , 0xa1);
  
  while(gpio_get_level(ADS1220_DRDY_PIN)){
    //stay here untill drdy line goes low
  }

  channel3 = sensything_ads1220readdata();
  printf("channel3 : %f \n",channel3);

  int32_t ble_vout = (int32_t) (channel3 * 100);
  ads1220_data_array[8] = (uint8_t) ble_vout;
  ads1220_data_array[9] = (uint8_t) (ble_vout >> 8);
  ads1220_data_array[10] = (uint8_t) (ble_vout >> 16);
  ads1220_data_array[11] = (uint8_t) (ble_vout >> 24);

}

void sensything_ads1220read_channel4(void){

  sensything_ads1220writeregister( CONFIG_REG0_ADDRESS , 0xb1);
  
  while(gpio_get_level(ADS1220_DRDY_PIN)){
    //stay here untill drdy line goes low
  }

  channel4 = sensything_ads1220readdata();
  printf("channel4 : %f \n",channel4);
  
  int32_t ble_vout = (int32_t) (channel4 * 100);
  ads1220_data_array[12] = (uint8_t) ble_vout;
  ads1220_data_array[13] = (uint8_t) (ble_vout >> 8);
  ads1220_data_array[14] = (uint8_t) (ble_vout >> 16);
  ads1220_data_array[15] = (uint8_t) (ble_vout >> 24);

}

void read_data(void *pvParameters)			// calls max30003read_data to update to aws_iot.
{	
	vTaskDelay(200 / portTICK_PERIOD_MS);

 	while(1)
  {	   
#ifdef CONFIG_ADC_CHANNEL_ONE
    sensything_ads1220read_channel1();
    vTaskDelay(100 / portTICK_PERIOD_MS);
#endif
#ifdef CONFIG_ADC_CHANNEL_TWO
    sensything_ads1220read_channel2();
    vTaskDelay(100 / portTICK_PERIOD_MS);
#endif
#ifdef CONFIG_ADC_CHANNEL_THREE
    sensything_ads1220read_channel3();
    vTaskDelay(100 / portTICK_PERIOD_MS);
#endif
#ifdef CONFIG_ADC_CHANNEL_FOUR
    sensything_ads1220read_channel4();
    vTaskDelay(100 / portTICK_PERIOD_MS);
#endif
    vTaskDelay(500 / portTICK_PERIOD_MS);   //dbug
  }
}

uint8_t * get_ads1220data(void){

  return ads1220_data_array;
}

void sensything_ads1220start(void)
{
  sensything_ads1220initchip();

  xTaskCreate(&read_data, "read_data", 4096, NULL, 4, NULL);
}

void sensything_ads1220initchip(void)
{

  esp_err_t ret;

  gpio_pad_select_gpio(ADS1220_CS_PIN);
  gpio_pad_select_gpio(ADS1220_DRDY_PIN);
  gpio_pad_select_gpio(ADS1220_MOSI);
  gpio_pad_select_gpio(ADS1220_SCLK);
  gpio_pad_select_gpio(ADS1220_MISO);
  gpio_pad_select_gpio(22);
  
  gpio_set_direction(ADS1220_DRDY_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(ADS1220_MISO, GPIO_MODE_INPUT);
  gpio_set_direction(ADS1220_CS_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(ADS1220_MOSI, GPIO_MODE_OUTPUT);
  gpio_set_direction(ADS1220_SCLK, GPIO_MODE_OUTPUT); 
  gpio_set_direction(22, GPIO_MODE_OUTPUT); 
  
  gpio_set_level(22, HIGH);
  
  spi_bus_config_t buscfg=
  {
    .miso_io_num=ADS1220_MISO,
    .mosi_io_num=ADS1220_MOSI,
    .sclk_io_num=ADS1220_SCLK,
    .quadwp_io_num=-1,
    .quadhd_io_num=-1
  };
  spi_device_interface_config_t devcfg=
  {
    .clock_speed_hz=4000000,              
    .mode=1,                                
    .queue_size=77,                          
    .pre_cb=ads1220_spi_pre_transfer_callback,  
  };
  ret=spi_bus_initialize(VSPI_HOST, &buscfg, 1);          //esp-idf v2.1 does not support dma
  assert(ret==ESP_OK);
  ret=spi_bus_add_device(VSPI_HOST, &devcfg, &spi);
  assert(ret==ESP_OK);
  vTaskDelay(100 / portTICK_PERIOD_MS);

  uint8_t Config_Reg0 = 0x01;
  uint8_t Config_Reg1 = 0x04;
  uint8_t Config_Reg2 = 0x10;
  uint8_t Config_Reg3 = 0x00;

  sensything_ads1220writeregister( CONFIG_REG0_ADDRESS , Config_Reg0);
  sensything_ads1220reg_read(CONFIG_REG0_ADDRESS);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  
  sensything_ads1220writeregister( CONFIG_REG1_ADDRESS , Config_Reg1);
  sensything_ads1220writeregister( CONFIG_REG2_ADDRESS , Config_Reg2);
  sensything_ads1220writeregister( CONFIG_REG3_ADDRESS , Config_Reg3);

  vTaskDelay(1000 / portTICK_PERIOD_MS);
/*  
  //reads back the registers to ensure its properly written
  sensything_ads1220reg_read(CONFIG_REG0_ADDRESS);
  sensything_ads1220reg_read(CONFIG_REG1_ADDRESS);
  sensything_ads1220reg_read(CONFIG_REG2_ADDRESS);
*/  
  sensything_ads1220sendcmd(START);
}