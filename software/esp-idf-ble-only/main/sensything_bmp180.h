#ifndef sensything_bmp180_h
#define sensything_bmp180_h
#include "driver/i2c.h"


#define BMP180_ADDRESS 0x77

#define BMP180_CAL_AC1       0xAA
#define BMP180_CAL_AC2         0xAC
#define BMP180_CAL_AC3         0xAE
#define BMP180_CAL_AC4         0xB0
#define BMP180_CAL_AC5         0xB2
#define BMP180_CAL_AC6         0xB4
#define BMP180_CAL_B1          0xB6
#define BMP180_CAL_B2          0xB8
#define BMP180_CAL_MB          0xBA
#define BMP180_CAL_MC          0xBC
#define BMP180_CAL_MD          0xBE

#define BMP180_CHIPID          0xD0
#define BMP180_VERSION         0xD1
#define BMP180_SOFTRESET       0xE0
#define BMP180_CONTROL         0xF4
#define BMP180_TEMPDATA        0xF6
#define BMP180_PRESSUREDATA    0xF6
#define BMP180_READTEMPCMD     0x2E
#define BMP180_READPRESSURECMD  0x34

#define BMP180_DATABUFF_LEN 8
void start_bmp180(i2c_port_t qwiic_id);
uint8_t * get_bmp_data(void);


#endif