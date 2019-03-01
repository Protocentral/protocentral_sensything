#ifndef sensything_bno080_h
#define sensything_bno080_h

#include "driver/i2c.h"

#define SENSOR_REPORTID_PERSONAL_ACTIVITY_CLASSIFIER 0x1E
#define COMMAND_ME_CALIBRATE 7

#define BNO080_DATABUFF_LEN 32
#define BNO080_ADDRESS 0x4B
#define QWIIC_OK 2
#define QWIIC_CONNECTED 1
#define QWIIC_DISCONNECTED 0
#define QWIIC_ERROR 3


typedef struct{

	uint8_t activity_values;


}bno_classifier;

void start_imu(void);

void start_BNO080(i2c_port_t qwiic_id);
bno_classifier * get_imudata();



#endif