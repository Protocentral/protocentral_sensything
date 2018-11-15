#ifndef sensything_ccs811_h
#define sensything_ccs811_h


#define CCS811_DATABUFF_LEN 6
#define CCS811_ADDRESS 0X5B


void start_ccs811(i2c_port_t qwiic_id);
uint8_t * get_ccs811_data(void);


#endif