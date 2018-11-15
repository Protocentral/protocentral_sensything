
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/task.h"

#include "sdkconfig.h"
#include "sensything_ccs811.h"
#include "sensything_qwiic.h"
#include "sensything_ble.h"

#define TAG "CCS811"
#define CCS811_HARDWARE_ID_REG 0x20
#define CCS811_HARDWARE_ID 0x81

extern bool ccs811_found;
i2c_port_t qwiic_port_ccs811 ;
TaskHandle_t ccs811_task_handle;
uint8_t ccs811_data[10] = {CCS811_ADDRESS, QWIIC_CONNECTED};

uint8_t read_id(uint8_t reg, uint8_t sensor_address, i2c_port_t qwiic_id) {

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (sensor_address << 1) | I2C_MASTER_WRITE, 1 );
	i2c_master_write_byte(cmd, reg, 1);
	i2c_master_stop(cmd);

	esp_err_t err = i2c_master_cmd_begin(qwiic_id, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	cmd = 0;
	if(err == ESP_OK){

		ESP_LOGI(TAG, " write success");
		//bme280 = true;
	}else{ ESP_LOGI(TAG, " write fail");
		ccs811_found = false;
		ccs811_data[1] = QWIIC_DISCONNECTED;
		send_qwiic_status_ble(ccs811_data);
	}

	uint8_t msb;
	uint8_t lsb;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (sensor_address << 1) | I2C_MASTER_READ, 1 );
	i2c_master_read(cmd, &msb, 1, 1);
	//i2c_master_read_byte(cmd, lsb, 0);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(qwiic_id, cmd, 2000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	ESP_LOGI("CCS811", " read id %d", msb);
	ccs811_data[0] = msb;
	ccs811_data[1] = QWIIC_OK;
	ccs811_data[2] = 01;
	ccs811_data[3] = 21;
	ccs811_data[4] = 00;
	ccs811_data[5] = 33;

	return msb;
}

void ccs811_read_task(void *ignore)
{

	while(true){
		read_id(CCS811_HARDWARE_ID_REG, CCS811_ADDRESS, qwiic_port_ccs811);
		vTaskDelay(3000/portTICK_PERIOD_MS);
	}
}

uint8_t * get_ccs811_data(void){

	return ccs811_data;
}

void start_ccs811(i2c_port_t qwiic_id){
	

	qwiic_port_ccs811 = qwiic_id;
	ccs811_data[1] = QWIIC_CONNECTED;
	send_qwiic_status_ble(ccs811_data);

	read_id(CCS811_HARDWARE_ID_REG, CCS811_ADDRESS, qwiic_id);

	//xTaskCreate(&ccs811_read_task, "read environmental sensor", 2048 * 2, (void* ) 0, 10, ccs811_task_handle);
}