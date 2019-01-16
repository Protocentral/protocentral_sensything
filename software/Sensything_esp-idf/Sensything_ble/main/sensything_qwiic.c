#include <stdio.h>
#include "driver/i2c.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sensything_bme280.h"
#include "sensything_qwiic.h"
#include "bme280_driver.h"

static char TAG[] = "qwiic";


bool bme280_found = false;

bool read_sensor_ack(uint8_t reg, uint8_t sensor_address, i2c_port_t qwiic_id) {

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (sensor_address << 1) | I2C_MASTER_WRITE, 1 );

	i2c_master_write_byte(cmd, reg, 1);
	i2c_master_stop(cmd);

	esp_err_t err = i2c_master_cmd_begin(qwiic_id, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	if(err == ESP_OK){

		ESP_LOGI(TAG, " found sensor %d", sensor_address);
		//bme280 = true;
		return true;
	}else return false;
}

uint8_t read_peripheral_id(uint8_t reg, uint8_t sensor_address, i2c_port_t qwiic_id) {

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (sensor_address << 1) | I2C_MASTER_WRITE, 1 );

	i2c_master_write_byte(cmd, reg, 1);
	i2c_master_stop(cmd);

	esp_err_t err = i2c_master_cmd_begin(qwiic_id, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	if(err != ESP_OK){
		return 0;
	}

	cmd = 0;

	uint8_t msb;
	uint8_t lsb;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (sensor_address << 1) | I2C_MASTER_READ, 1 );
	i2c_master_read_byte(cmd, &msb, 1);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(qwiic_id, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	return msb;
}

void scan_peripherals(void* arg)
{
	/* as the number of peripheral increases, we may look for acknowldgement from the i2c sensor on write instead of reading the ID 
	also the this part can be made a bit more generic*/
	//rgb led can be used as an indication
	bool ret;

	while (true) {
		
		
		if(!bme280_found){
			ret = read_sensor_ack(0, BME280_ADDRESS, I2C_NUM_0);
			vTaskDelay(100 / portTICK_RATE_MS);
			if(ret){
				ESP_LOGI(TAG, " starting bme280 : qwiic 1");
				bme280_found = true;
				start_bme280(I2C_NUM_0);
			}


			ret = read_sensor_ack(0, BME280_ADDRESS, I2C_NUM_1);
			if(ret){
				if(bme280_found){
					ESP_LOGI(TAG, " already one bme is present on qwiic1 port, please remove the sensor from qwiic2");
				}else{
					bme280_found = true;
					ESP_LOGI(TAG, " starting bme280 : qwiic 2");
					start_bme280(I2C_NUM_1);
				}
			}
			vTaskDelay(100 / portTICK_RATE_MS);
		}
		
	}
}

void qwiic1_i2c_init(void){

	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = 21;
	conf.scl_io_num = 22;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;
	i2c_param_config(I2C_NUM_0, &conf);

	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}
void qwiic2_i2c_init(void){

	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = 26;
	conf.scl_io_num = 25;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;
	i2c_param_config(I2C_NUM_1, &conf);

	i2c_driver_install(I2C_NUM_1, I2C_MODE_MASTER, 0, 0, 0);
}

void init_qwiic(void)
{
    qwiic1_i2c_init();
    qwiic2_i2c_init();
    
    xTaskCreate(scan_peripherals, "scan", 1024 * 2, (void* ) 0, 10, NULL);

}