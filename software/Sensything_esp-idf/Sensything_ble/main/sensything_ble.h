#ifndef SENSYTHING_BLE_H
#define SENSYTHING_BLE_H

#define GATTS_SERVICE_UUID_HR   0x180F
#define GATTS_CHAR_UUID_HR      0x2A19
#define GATTS_DESCR_UUID_TEST_A     0x2902
#define GATTS_NUM_HANDLE_HR     4
#define GATTS_NUM_HANDLE_QWIIC     4

#define GATTS_SERVICE_UUID_TEMP   0x1104
#define GATTS_CHAR_UUID_TEMP      0x1105
#define GATTS_DESCR_UUID_TEST_B     0x2902

#define GATTS_SERVICE_UUID_BAT   0x180F
#define GATTS_CHAR_UUID_BAT      0x2A19
#define GATTS_DESCR_UUID_TEST_C     0x2902

#define TEST_DEVICE_NAME         "SensyThing"
#define TEST_MANUFACTURER_DATA_LEN  17
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 0x40

#define PROFILE_NUM 3
#define PROFILE_A_APP_ID 0
#define PROFILE_C_APP_ID 1
#define PROFILE_B_APP_ID 2

#define GATTS_TAG "SensyThing"

#define SENSOR_CONTACT_DETECTED 0x06
#define SENSOR_CONTACT_NOT_DETECTED 0x04

void sensything_ble_init(void);

void send_qwiic_status_ble(uint8_t  * qwiic_sensor_data);
void update_ble_atts(void(*update)(uint16_t),uint16_t value);
void updateads1220_ble( float vout);
void update_bat(uint8_t value);
void updatechannel1_ble(int adsdata);
void updatechannel2_ble(int adsdata);
void updatechannel3_ble(int adsdata);
void updatechannel4_ble(int adsdata);

#endif
