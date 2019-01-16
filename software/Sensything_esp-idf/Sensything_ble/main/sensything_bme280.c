
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/task.h"

#include "sdkconfig.h"
#include "sensything_bme280.h"
#include "bme280.h"
#include "sensything_qwiic.h"
#include "sensything_ble.h"


extern bool bme280_found;
i2c_port_t qwiic_port_bme280 ;
TaskHandle_t bme280_task_handle;
uint8_t bme280_data[10] = {BME280_ADDRESS,QWIIC_OK,0 };

#define TAG_BME280 "BME280"


int8_t BME280_I2C_bus_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
	s32 iError = BME280_INIT_VALUE;

	esp_err_t espRc;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, reg_addr, true);
	i2c_master_write(cmd, reg_data, cnt, true);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(qwiic_port_bme280, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		iError = SUCCESS;
	} else {
		iError = FAIL;
	}
	i2c_cmd_link_delete(cmd);

	return (int8_t)iError;
}

int8_t BME280_I2C_bus_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
	s32 iError = BME280_INIT_VALUE;
	esp_err_t espRc;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, reg_addr, true);

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);

	if (cnt > 1) {
		i2c_master_read(cmd, reg_data, cnt-1, I2C_MASTER_ACK);
	}
	i2c_master_read_byte(cmd, reg_data+cnt-1, I2C_MASTER_NACK);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(qwiic_port_bme280, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		iError = SUCCESS;
	} else {
		iError = FAIL;
	}

	i2c_cmd_link_delete(cmd);

	return (int8_t)iError;
}

void BME280_delay_msek(u32 msek)
{
	vTaskDelay(msek/portTICK_PERIOD_MS);
}

void task_bme280_normal_mode(void *ignore)
{

	struct bme280_t bme280 = {
		.bus_write = BME280_I2C_bus_write,
		.bus_read = BME280_I2C_bus_read,
		.dev_addr = BME280_I2C_ADDRESS2,
		.delay_msec = BME280_delay_msek
	};

	s32 com_rslt;
	s32 v_uncomp_pressure_s32;
	s32 v_uncomp_temperature_s32;
	s32 v_uncomp_humidity_s32;

	com_rslt = bme280_init(&bme280);

	com_rslt += bme280_set_oversamp_pressure(BME280_OVERSAMP_16X);
	com_rslt += bme280_set_oversamp_temperature(BME280_OVERSAMP_2X);
	com_rslt += bme280_set_oversamp_humidity(BME280_OVERSAMP_1X);

	com_rslt += bme280_set_standby_durn(BME280_STANDBY_TIME_1_MS);
	com_rslt += bme280_set_filter(BME280_FILTER_COEFF_16);

	com_rslt += bme280_set_power_mode(BME280_NORMAL_MODE);
	
	if (com_rslt == SUCCESS) {
		while(true) {
		if(bme280_found){	
				vTaskDelay(40/portTICK_PERIOD_MS);

				com_rslt = bme280_read_uncomp_pressure_temperature_humidity(
				&v_uncomp_pressure_s32, &v_uncomp_temperature_s32, &v_uncomp_humidity_s32);

				if (com_rslt == SUCCESS) {
					float temperature = bme280_compensate_temperature_double(v_uncomp_temperature_s32);
					float pressure = bme280_compensate_pressure_double(v_uncomp_pressure_s32)/100; // Pa -> hPa
					float humidity = bme280_compensate_humidity_double(v_uncomp_humidity_s32);
					ESP_LOGI("bme280","temp %f :   pressure %f :   humidity  %f \n", temperature, pressure, humidity);

					int16_t temperature_ble = (uint16_t) (temperature * 100);
					int32_t pressure_ble = (uint32_t) (pressure * 100);
					int32_t humidity_ble = (uint16_t) (humidity * 100);
					//ESP_LOGI("bme280","temp %d :   pressure %d :   humidity  %d \n", temperature_ble, pressure_ble, humidity_ble);
					
					bme280_data[0] = BME280_ADDRESS;
					bme280_data[1] = QWIIC_OK;
					bme280_data[2] = temperature_ble;
					bme280_data[3] = temperature_ble >> 8;
					bme280_data[4] = pressure_ble;
					bme280_data[5] = pressure_ble >> 8;
					bme280_data[6] = pressure_ble >> 16;
					bme280_data[7] = pressure_ble >> 24;
					bme280_data[8] = humidity_ble;
					bme280_data[9] = humidity_ble >> 8;

				} else {
					ESP_LOGE(TAG_BME280, "measure error, please make sure that sensor is connected properly %d", com_rslt);
					bme280_found = false;
					
					bme280_data[1] = QWIIC_DISCONNECTED;
					send_qwiic_status_ble(bme280_data);
					
					vTaskDelete(bme280_task_handle);
					
				}
			}else{
				bme280_found = false;
				vTaskDelete(bme280_task_handle);
				bme280_data[1] = QWIIC_DISCONNECTED;
				send_qwiic_status_ble(bme280_data);
			}
		}
	} else {
		ESP_LOGE(TAG_BME280, "error,  please make sure that sensor is connected properly %d", com_rslt);
		bme280_data[1] = QWIIC_DISCONNECTED;
		send_qwiic_status_ble(bme280_data);
	}
	bme280_found = false;
	vTaskDelete(bme280_task_handle);
}

void task_bme280_forced_mode(void *ignore) {
	struct bme280_t bme280 = {
		.bus_write = BME280_I2C_bus_write,
		.bus_read = BME280_I2C_bus_read,
		.dev_addr = BME280_I2C_ADDRESS2,
		.delay_msec = BME280_delay_msek
	};

	s32 com_rslt;
	s32 v_uncomp_pressure_s32;
	s32 v_uncomp_temperature_s32;
	s32 v_uncomp_humidity_s32;

	com_rslt = bme280_init(&bme280);

	com_rslt += bme280_set_oversamp_pressure(BME280_OVERSAMP_1X);
	com_rslt += bme280_set_oversamp_temperature(BME280_OVERSAMP_1X);
	com_rslt += bme280_set_oversamp_humidity(BME280_OVERSAMP_1X);

	com_rslt += bme280_set_filter(BME280_FILTER_COEFF_OFF);

	if (com_rslt == SUCCESS) {
		while(true) {
			
			if(bme280_found){
					com_rslt = bme280_get_forced_uncomp_pressure_temperature_humidity(&v_uncomp_pressure_s32, &v_uncomp_temperature_s32, &v_uncomp_humidity_s32);

					if (com_rslt == SUCCESS) {
						
							float temperature = bme280_compensate_temperature_double(v_uncomp_temperature_s32);
							float pressure = bme280_compensate_pressure_double(v_uncomp_pressure_s32)/100; // Pa -> hPa
							float humidity = bme280_compensate_humidity_double(v_uncomp_humidity_s32);
							ESP_LOGI("bme280","temp %f  pressure %f  humidity  %f \n", temperature, pressure, humidity);
					} else {
						ESP_LOGE(TAG_BME280, "measure error. code: %d", com_rslt);
					}
			}else{
				bme280_found = false;
				vTaskDelete(bme280_task_handle);
			}
		}
	} else {
		ESP_LOGE(TAG_BME280, "init or setting error. code: %d", com_rslt);
	}
	bme280_found = false;
	vTaskDelete(bme280_task_handle);
}

uint8_t * get_bme280_data(void){

	return bme280_data;
}

void start_bme280(i2c_port_t qwiic_id){

	qwiic_port_bme280 = qwiic_id;

	bme280_data[1] = QWIIC_CONNECTED;
	send_qwiic_status_ble(bme280_data);
	xTaskCreate(&task_bme280_normal_mode, "bme280_normal_mode", 2048 * 2, (void* ) 0, 10, bme280_task_handle);
}
