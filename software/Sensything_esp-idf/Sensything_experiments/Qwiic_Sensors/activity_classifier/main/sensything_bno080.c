#include <stdlib.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "math.h"
#include "freertos/task.h"

#include "sdkconfig.h"
#include "sensything_bno080.h"

#define TAG "BNO080"
#define SENSOR_REPORTID_PERSONAL_ACTIVITY_CLASSIFIER 0x1E
#define SHTP_REPORT_PRODUCT_ID_REQUEST 0xF9
#define SHTP_REPORT_PRODUCT_ID_RESPONSE 0xF8
#define SHTP_REPORT_SET_FEATURE_COMMAND 0xFD
#define SHTP_REPORT_BASE_TIMESTAMP 0xFB
#define SHTP_REPORT_COMMAND_RESPONSE 0xF1

#define MAX_PACKET_SIZE 255
#define I2C_BUFFER_LENGTH 32
#define BNO080_INIT_VALUE (0)
#define SUCCESS ((uint8_t)0)
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */
#define PACKET_HEADER 4                            /*!< I2C nack value */

#define I2C_RW_MAX_SIZE 280
#define SAMPLING_RATE 50

void enable_ActivityClassifier(uint8_t sampling_ms, uint8_t *activity);
void printActivityName(const uint8_t activityNumber);
void begin_bno080();
bool receive_packet_data(uint8_t * buff);

i2c_port_t qwiic_port_BNO080 ;
TaskHandle_t BNO080_task_handle;

bno_classifier activity_data; 

uint8_t BNO080_data[10] = {BNO080_ADDRESS};
uint8_t shtpHeader[4];
uint8_t shtp_data[I2C_RW_MAX_SIZE+10];

const int8_t CHANNEL_EXECUTABLE = 1;
const int8_t CHANNEL_CONTROL = 2;
const int8_t CHANNEL_REPORTS = 3;
const int8_t activityConfidences[9];
uint8_t * sensor_data;
uint8_t sequenceNumber[6] = {0, 0, 0, 0, 0, 0};
uint32_t timeStamp;
uint8_t calibrationStatus;
uint8_t activityClassifier;
uint8_t *_activityConfidences;


void i2c_init(void){

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = 21;
    conf.scl_io_num = 22;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;
    i2c_param_config(I2C_NUM_0, &conf);

    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}


int8_t BNO080_I2C_bus_write(uint8_t dev_addr, uint8_t *data_wr, uint8_t cnt)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, cnt, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(qwiic_port_BNO080, cmd, 1000 / portTICK_RATE_MS);
    if(ret == ESP_OK) {
          
    }else {}

    i2c_cmd_link_delete(cmd);
    return ret;
}


int8_t BNO080_I2C_bus_write_rst(uint8_t dev_addr, uint8_t *data_wr, uint8_t cnt)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_DIS);
    i2c_master_write(cmd, data_wr, cnt, ACK_CHECK_DIS);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(qwiic_port_BNO080, cmd, 1000 / portTICK_RATE_MS);
    if(ret == ESP_OK) {
        
    }else{}

    i2c_cmd_link_delete(cmd);
    return ret;
}


int8_t BNO080_I2C_bus_read(uint8_t dev_addr, uint8_t *data_rd, uint16_t cnt)
{

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( dev_addr << 1 ) | I2C_MASTER_READ, ACK_CHECK_EN);

    int i;
    for( i=0; i<cnt-1; i++){
        i2c_master_read_byte(cmd, shtp_data + i, ACK_CHECK_DIS);
    }
    i2c_master_read_byte(cmd, shtp_data + i, ACK_CHECK_EN);

    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(qwiic_port_BNO080, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return NULL;
}

void parseCommandReport(uint8_t * shtpData)
{
  if (shtpData[0] == SHTP_REPORT_COMMAND_RESPONSE)
  {
    
    uint8_t command = shtpData[2]; 
    
    if(command == COMMAND_ME_CALIBRATE)
    {
    calibrationStatus = shtpData[4 + 0]; 
    }
  }
  else
  {}

}


void parseInputReport(uint8_t * shtpData)
{
  //Calculate the number of data bytes in this packet
  int16_t dataLength = ((uint16_t)shtpHeader[1] << 8 | shtpHeader[0]);
  dataLength &= ~(1 << 15); //Clear the MSbit. This bit indicates if this package is a continuation of the last.
  
  dataLength -= 4; //Remove the header bytes from the data count

  timeStamp = ((uint32_t)shtpData[4] << (8 * 3)) | (shtpData[3] << (8 * 2)) | (shtpData[2] << (8 * 1)) | (shtpData[1] << (8 * 0));

    if (shtpData[5] == SENSOR_REPORTID_PERSONAL_ACTIVITY_CLASSIFIER)
  {
    activityClassifier = shtpData[5+5]; //Most likely state
     _activityConfidences[0] = shtpData[11];
     _activityConfidences[1] = shtpData[12];
     _activityConfidences[2] = shtpData[13];
     _activityConfidences[3] = shtpData[14];
     _activityConfidences[4] = shtpData[15];
     _activityConfidences[5] = shtpData[16];
     _activityConfidences[6] = shtpData[17];
     _activityConfidences[7] = shtpData[18];
     _activityConfidences[8] = shtpData[19];
  }
  else
  {}
}


bool get_data_if_available(uint8_t * buff){

    receive_packet_data(buff);
    memcpy(shtpHeader, shtp_data, 4);

    if (shtp_data[2] == CHANNEL_REPORTS && shtp_data[4] == SHTP_REPORT_BASE_TIMESTAMP)
    {
        parseInputReport(&shtp_data[4]);

        activity_data.activity_values = activityClassifier;
        const uint8_t mostLikelyActivity = activity_data.activity_values;
        printActivityName(mostLikelyActivity);
         
        return (true);
    }
    else if (shtpHeader[2] == CHANNEL_CONTROL)
    {
        printf("channel control\n");
        parseCommandReport(&shtp_data[4]);
        return (true);
    }

    return false;
}

void printActivityName(const uint8_t activityNumber)
{
  if(activityNumber == 0) {printf("Unknown\n");}
  else if(activityNumber == 1) { printf("In vehicle\n");}
  else if(activityNumber == 2) {printf("On bicycle\n");}
  else if(activityNumber == 3) {printf("On foot\n");}
  else if(activityNumber == 4) {printf("Still\n");}
  else if(activityNumber == 5) {printf("Tilting\n");}
  else if(activityNumber == 6) {printf("Walking\n");}
  else if(activityNumber == 7) {printf("Running\n");}
  else if(activityNumber == 8) {printf("On stairs\n");}
}

void task_BNO080(void *ignore)
{
    while(true) {
        
        get_data_if_available(shtp_data);
        vTaskDelay(SAMPLING_RATE / portTICK_RATE_MS);
    }
}


void get_data(uint16_t bytes_remaining, uint8_t * buff){
    
    uint16_t data_spot = 0;
    uint16_t numberOfBytesToRead = bytes_remaining;
    if (numberOfBytesToRead > (I2C_RW_MAX_SIZE )) numberOfBytesToRead = (I2C_RW_MAX_SIZE);
    BNO080_I2C_bus_read(BNO080_ADDRESS, buff + (data_spot++ * I2C_RW_MAX_SIZE), numberOfBytesToRead);

    return true; 
}


void enable_ActivityClassifier(uint8_t sampling_ms, uint8_t *activity){

    long microsBetweenReports = (long)sampling_ms * 1000;
    uint32_t enableActivities = 0x1F; //Enable all 9 possible activities including Unknown

    _activityConfidences = activity;

    uint8_t shtp_data[25];

    shtp_data[0] = 0x15;
    shtp_data[1] = 0x00;
    shtp_data[2] = 0x02;
    shtp_data[3] = 0x00;
    shtp_data[4] = SHTP_REPORT_SET_FEATURE_COMMAND; //Set feature command. Reference page 55
    shtp_data[5] = SENSOR_REPORTID_PERSONAL_ACTIVITY_CLASSIFIER;
    shtp_data[6] = 0; //Feature flags
    shtp_data[7] = 0; //Change sensitivity (LSB)
    shtp_data[8] = 0; //Change sensitivity (MSB)
    shtp_data[9] = (microsBetweenReports) & 0xFF; //Report interval (LSB) in microseconds. 0x7A120 = 500ms
    shtp_data[10] = (microsBetweenReports >> 8) & 0xFF; //Report interval
    shtp_data[11] = (microsBetweenReports >> 16) & 0xFF; //Report interval
    shtp_data[12] = (microsBetweenReports >> 24) & 0xFF; //Report interval (MSB)
    shtp_data[13] = 0; //Batch Interval (LSB)
    shtp_data[14] = 0; //Batch Interval
    shtp_data[15] = 0; //Batch Interval
    shtp_data[16] = 0; //Batch Interval (MSB)
    shtp_data[17] = (enableActivities >> 0) & 0xFF; //Sensor-specific config (LSB)
    shtp_data[18] = (enableActivities >> 8) & 0xFF; //Sensor-specific config
    shtp_data[19] = (enableActivities >> 16) & 0xFF; //Sensor-specific config
    shtp_data[20] = (enableActivities >> 24) & 0xFF; //Sensor-specific config (MSB)

    //Transmit packet on channel 2, 17 bytes
    int ret;
    do{
        ret = BNO080_I2C_bus_write(BNO080_ADDRESS, shtp_data, 21);
        vTaskDelay(20 / portTICK_RATE_MS);
      
    }while(ret != ESP_OK);

    vTaskDelay(100 / portTICK_RATE_MS);
}


bool receive_packet_data(uint8_t * buff){

    int ret;

    do{
        ret = BNO080_I2C_bus_read(BNO080_ADDRESS, buff, 4);
        vTaskDelay(30 / portTICK_RATE_MS);
        if(ret == ESP_OK) {
         
        }else {}

    }while (ret != ESP_OK);

    int16_t data_length = ((uint16_t)buff[1] << 8 | buff[0]);
    data_length &= ~(1 << 15); //Clear the MSbit.

    if(!data_length) return false;

    vTaskDelay(10 / portTICK_RATE_MS);
    get_data(data_length, shtp_data);

    return true;
}


void send_data_packet(void * data_buff, uint8_t len, uint8_t channel_number){

    uint8_t wr_data[len];
    wr_data[0] = (uint8_t) len;
    wr_data[1] = (uint8_t) (len>>8);
    wr_data[2] = channel_number;
    wr_data[3] = sequenceNumber[channel_number]++;

    memcpy(&wr_data[4], data_buff, (len - 4));
    
    int ret;
    do{
        ret = BNO080_I2C_bus_write(BNO080_ADDRESS, wr_data, len);
        vTaskDelay(20 / portTICK_RATE_MS);

    }while (ret != ESP_OK);

    vTaskDelay(150 / portTICK_RATE_MS);
}

void send_data_packet_rst(void * data_buff, uint8_t len, uint8_t channel_number){

    uint8_t wr_data[len];
    wr_data[0] = (uint8_t) len;
    wr_data[1] = (uint8_t) (len>>8);
    wr_data[2] = 1;//channel_number;
    wr_data[3] = 0;//sequenceNumber[channel_number]++;
    wr_data[4] = 1;

    sequenceNumber[channel_number]++;
    
    int ret;
    ret = BNO080_I2C_bus_write_rst(BNO080_ADDRESS, wr_data, len);
}


void soft_reset(){

    shtp_data[4] = 0;
    uint8_t recv_buff[300];

    send_data_packet_rst(shtp_data, 5, CHANNEL_EXECUTABLE);
    vTaskDelay(50 / portTICK_RATE_MS);

    receive_packet_data(recv_buff);
    vTaskDelay(50 / portTICK_RATE_MS);
    receive_packet_data(recv_buff);
    vTaskDelay(50 / portTICK_RATE_MS);
}


void begin_bno080()
{
    printf("begin\n");

    soft_reset();
    shtp_data[0] = SHTP_REPORT_PRODUCT_ID_REQUEST;
    shtp_data[1] = 0;

    receive_packet_data(shtp_data);
    vTaskDelay(50 / portTICK_RATE_MS);

    enable_ActivityClassifier(SAMPLING_RATE, (&activityConfidences)[9]);
    vTaskDelay(300 / portTICK_RATE_MS);
}

bno_classifier * get_imudata(){

    if(get_data_if_available(shtp_data)){
      return &activity_data;
    }else return NULL;
}


void start_imu(void){

    i2c_init();
   
    qwiic_port_BNO080 = I2C_NUM_0;
    i2c_set_timeout(I2C_NUM_0, 60000);

    int t;
    i2c_get_timeout(I2C_NUM_0, &t);
    begin_bno080();  

}


