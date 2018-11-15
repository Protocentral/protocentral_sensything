#ifndef sensything_bme_h
#define sensything_bme_h

#define BME280_ADDRESS 0x77
#define BME280_DATABUFF_LEN 10


void start_bme280(i2c_port_t qwiic_id);
uint8_t * get_bme280_data(void);

#endif
