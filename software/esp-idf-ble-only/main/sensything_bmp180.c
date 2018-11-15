
#include <stdio.h>
#include "driver/i2c.h"
#include "sensything_bmp180.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sensything_ble.h"
#include "sensything_qwiic.h"


static char TAG[] = "bmp180";
extern bool bmp180_found;
TaskHandle_t bmp180_task_handle;

uint8_t bmp180_data[10] = {BMP180_ADDRESS, QWIIC_DISCONNECTED};
i2c_port_t qwiic_port;

typedef enum
{
  BMP085_MODE_ULTRALOWPOWER          = 0,
  BMP085_MODE_STANDARD               = 1,
  BMP085_MODE_HIGHRES                = 2,
  BMP085_MODE_ULTRAHIGHRES           = 3
} bmp085_mode_t;

 int16_t readRegister8(uint8_t reg) {

 	if(!bmp180_found) return -999;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	esp_err_t  err = i2c_master_write_byte(cmd, (BMP180_ADDRESS << 1) | I2C_MASTER_WRITE, 1);
	i2c_master_write_byte(cmd, reg, 1);
	i2c_master_stop(cmd);
	err = i2c_master_cmd_begin(qwiic_port, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	if(err != ESP_OK){
	
		ESP_LOGI(TAG, "write fail");
		bmp180_found = false;
		
		return -999;
	}

	uint8_t msb;
	uint8_t lsb;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (BMP180_ADDRESS << 1) | I2C_MASTER_READ, 1 /* expect ack */);
	i2c_master_read_byte(cmd, &msb, 1);
	i2c_master_read_byte(cmd, &lsb, 0);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(qwiic_port, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	return msb;
}


 uint16_t readRegister16(uint8_t reg) {

	uint8_t msb = readRegister8(reg);
	uint8_t lsb = readRegister8(++reg);

	//int16_t ret = (int16_t )((msb << 8) | lsb);
	uint16_t ret = ((msb << 8) | lsb);
	return ret;
}

int readRegister24(uint8_t reg) {

	uint8_t msb = readRegister8(reg);
	uint8_t lsb = readRegister8(++reg);
	uint8_t xlsb = readRegister8(++reg);


	int ret = (int)((msb << 16) | (lsb << 8) | xlsb);
	return ret;
}

static uint32_t readUncompensatedTemp(void){
		
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (BMP180_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
	i2c_master_write_byte(cmd, BMP180_CONTROL, 1);
	i2c_master_write_byte(cmd, BMP180_READTEMPCMD, 1);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(qwiic_port, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	vTaskDelay(5/portTICK_PERIOD_MS);
	
	uint32_t ret = readRegister16(BMP180_TEMPDATA);
	return ret;
}

static uint32_t readUncompensatedPressure(uint32_t mode) {
	
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (BMP180_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
	i2c_master_write_byte(cmd, BMP180_CONTROL, 1);
	i2c_master_write_byte(cmd, BMP180_READPRESSURECMD + (mode << 6), 1);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(qwiic_port, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	switch(mode) {
	case BMP085_MODE_ULTRALOWPOWER:
		vTaskDelay(5/portTICK_PERIOD_MS);
		break;
	case BMP085_MODE_STANDARD:
		vTaskDelay(8/portTICK_PERIOD_MS);
		break;
	case BMP085_MODE_HIGHRES:
		vTaskDelay(14/portTICK_PERIOD_MS);
		break;
	case BMP085_MODE_ULTRAHIGHRES:
	default:
		vTaskDelay(26/portTICK_PERIOD_MS);
		break;
	}
	long ret;
	if (mode != BMP085_MODE_ULTRAHIGHRES) {
		ret = readRegister24(BMP180_PRESSUREDATA);
	} else {
		ret = readRegister24(BMP180_PRESSUREDATA);
	}
	return ret >> (8-mode);
}

void i2c_read_bmp180(void* arg)
{
	while (true) {

		if(bmp180_found){
		
			int16_t AC1 = readRegister16(BMP180_CAL_AC1);
			int16_t AC2 = readRegister16(BMP180_CAL_AC2);
			int16_t AC3 = readRegister16(BMP180_CAL_AC3);

			uint16_t AC4 = readRegister16(BMP180_CAL_AC4);
			uint16_t AC5 = readRegister16(BMP180_CAL_AC5);
			uint16_t AC6 = readRegister16(BMP180_CAL_AC6);
			int16_t B1 = readRegister16(BMP180_CAL_B1);
			int16_t B2 = readRegister16(BMP180_CAL_B2);
			int16_t MC = readRegister16(BMP180_CAL_MC);
			int16_t MD = readRegister16(BMP180_CAL_MD);

			int32_t bmpMode = BMP085_MODE_STANDARD;
			uint32_t UT = readUncompensatedTemp();
			int32_t UP = readUncompensatedPressure(bmpMode);		

			int32_t X1 = (UT - (int32_t)AC6) * ((int32_t)AC5) >> 15;
			int32_t X2 = ((int32_t)MC<<11) / (X1+(int32_t)MD);
			int32_t B5 =  X1 + X2;
			double temp = ((B5 + 8)>>4)/10.0;

			ESP_LOGI(TAG, "Temp = %.1foC ", temp);
			vTaskDelay(2000 / portTICK_RATE_MS);

		
			vTaskDelay(2000 / portTICK_RATE_MS);

			int32_t B6 = B5 - 4000;
			X1 = (B2 * (B6 * B6) >> 12) >> 11;
			X2 = (AC2 * B6) >> 11;
			int32_t X3 = X1 + X2;
			int32_t B3 = (((((int32_t)AC1)*4 + X3) << bmpMode) + 2) >> 2;
			X1 = (AC3 * B6) >> 13;
			X2 = (B1 * ((B6 * B6) >> 12)) >> 16;
			X3 = ((X1 + X2) + 2) >> 2;
			uint32_t B4 = ((uint32_t)AC4 * (uint32_t)(X3 + 32768)) >> 15;
			uint32_t B7 = ((((uint32_t)UP) - B3) * (50000 >> bmpMode));
			int32_t P;
			if (B7 < 0x80000000) {
				P = (B7 << 1) / B4;
			} else {
				P = (B7 / B4) << 1;
			}
			X1 = (P >> 8) * (P >> 8);
			X1 = (X1 * 3038) >> 16;
			X2 = (-7357 * P) >> 16;
			int32_t pressure = P + ((X1 + X2 + 3791) >> 4);
			//ESP_LOGI(TAG, "pressure = %d,  UP=%d", pressure, UP);

			uint32_t pressure_ble = 101325;//(uint32_t) (pressure );
			temp *=100;
			uint16_t temperatr = (uint16_t ) temp;
			ESP_LOGI(TAG, "pressure = %d  pa,  up=%d", pressure_ble, UP);
			bmp180_data[0] = BMP180_ADDRESS;
			bmp180_data[1] =  QWIIC_OK;
			bmp180_data[2] =  temperatr;
			bmp180_data[3] =  temperatr>>8;
			bmp180_data[4] =  pressure_ble;
			bmp180_data[5] =  pressure_ble >> 8;
			bmp180_data[6] =  pressure_ble >> 16;
			bmp180_data[7] =  pressure_ble >> 24;

		}
		if(!bmp180_found){ 

			bmp180_data[1] = QWIIC_DISCONNECTED;
			send_qwiic_status_ble(bmp180_data);
			vTaskDelete(bmp180_task_handle);

		}
	}
}

uint8_t * get_bmp_data(void){

	return bmp180_data;
}

void start_bmp180(i2c_port_t qwiic_id){

	qwiic_port = qwiic_id;
	bmp180_data[1] = QWIIC_CONNECTED;
	send_qwiic_status_ble(bmp180_data);

	xTaskCreate(i2c_read_bmp180, "read sensor", 2048 * 2, (void* ) 0, 10, bmp180_task_handle);
}

