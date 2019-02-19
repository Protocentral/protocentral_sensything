#include "Protocentral_ADS1220.h"
#include <SPI.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define PGA          1                 // Programmable Gain = 1
#define VREF         2.048            // Internal reference of 2.048V
#define VFSR         VREF/PGA
#define FULL_SCALE   (((long int)1<<23)-1)

#define ADS1220_CS_PIN    4
#define ADS1220_DRDY_PIN  34

Protocentral_ADS1220 pc_ads1220;
int32_t adc_data;

int analogPin = 36;     
volatile int adc_val=0;   // variable to store the value read
static bool startup_flag = true;
static int bat_prev=100;
static uint8_t bat_percent = 100;

int battery=0;
float bat;
int bt_rem = 0;
int bat_count=0;

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* qCharacteristic = NULL;
BLECharacteristic* bCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t value[16] = {0};
uint8_t value1[2] = {0};


#define SERVICE_UUID         "cd5c1105-4448-7db8-ae4c-d1da8cba36d0"
#define CHARACTERISTIC_UUID  "cd5c1106-4448-7db8-ae4c-d1da8cba36d0"

#define SERVICE_UUID1        "cd5c1100-4448-7db8-ae4c-d1da8cba36d0"
#define CHARACTERISTIC_UUID1 "cd5c1101-4448-7db8-ae4c-d1da8cba36d0"

#define SERVICE_UUID2        "0000180f-0000-1000-8000-00805f9b34fb" 
#define CHARACTERISTIC_UUID2 "00002a19-0000-1000-8000-00805f9b34fb" 

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
  void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


void setup()
{
    Serial.begin(115200);

    pc_ads1220.begin(ADS1220_CS_PIN,ADS1220_DRDY_PIN);\
  
    pc_ads1220.set_data_rate(DR_330SPS);
    pc_ads1220.set_pga_gain(PGA_GAIN_1);
  
    pc_ads1220.set_conv_mode_single_shot(); //Set Single shot mode

    Serial.println(F("ADS1220 enabled"));
  
    // Create the BLE Device
    BLEDevice::init("Sensything");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLEService *qService = pServer->createService(SERVICE_UUID1);
    BLEService *bService = pServer->createService(SERVICE_UUID2);

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
    qCharacteristic = qService->createCharacteristic(
                      CHARACTERISTIC_UUID1,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
    bCharacteristic = bService->createCharacteristic(
                      CHARACTERISTIC_UUID2,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

    // Create a BLE Descriptor
    pCharacteristic->addDescriptor(new BLE2902());
    qCharacteristic->addDescriptor(new BLE2902());
    bCharacteristic->addDescriptor(new BLE2902());

    // Start the service
    pService->start();
    qService->start();
    bService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify pUUID...");

    BLEAdvertising *qAdvertising = BLEDevice::getAdvertising();
    qAdvertising->addServiceUUID(SERVICE_UUID1);
    qAdvertising->setScanResponse(false);
    qAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify qUUID...");

    BLEAdvertising *bAdvertising = BLEDevice::getAdvertising();
    bAdvertising->addServiceUUID(SERVICE_UUID2);
    bAdvertising->setScanResponse(false);
    bAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify bUUID...");
}

void loop() 
{
    adc_data=pc_ads1220.Read_SingleShot_SingleEnded_WaitForData(MUX_SE_CH0);
    float vout = convertToMilliV(adc_data);
    Serial.print("\n\nCh1 (mV): ");
    Serial.print(vout);
    
    int32_t ble =(int32_t) (vout * 100); //0x11223344;
    value[0]= (uint8_t) ble;
    value[1]= (uint8_t) (ble >> 8);
    value[2]= (uint8_t) (ble >> 16);
    value[3]= (uint8_t) (ble >> 24);

    delay(100);

    
    adc_data=pc_ads1220.Read_SingleShot_SingleEnded_WaitForData(MUX_SE_CH1);
    float vout1 = convertToMilliV(adc_data);
    Serial.print("\nCh2 (mV): ");
    Serial.print(vout1);
    
    int32_t ble1 = (int32_t) (vout1 * 100);
    value[4]= (uint8_t) ble1;
    value[5]= (uint8_t) (ble1 >> 8);
    value[6]= (uint8_t) (ble1 >> 16);
    value[7]= (uint8_t) (ble1 >> 24);
    
    delay(100);


    adc_data=pc_ads1220.Read_SingleShot_SingleEnded_WaitForData(MUX_SE_CH2);
    float vout2 = convertToMilliV(adc_data);
    Serial.print("\nCh3 (mV): ");
    Serial.print(vout2);
    
    int32_t ble2 = (int32_t) (vout2 * 100);
    value[8]= (uint8_t) ble2;
    value[9]= (uint8_t) (ble2 >> 8);
    value[10]= (uint8_t) (ble2 >> 16);
    value[11]= (uint8_t) (ble2 >> 24);
    
    delay(100);

    adc_data=pc_ads1220.Read_SingleShot_SingleEnded_WaitForData(MUX_SE_CH3);
    float vout3 = convertToMilliV(adc_data);
    Serial.print("\nCh4 (mV): ");
    Serial.print(vout3);
    
    int32_t ble3 = (int32_t) (vout3 * 100);
    value[12]= (uint8_t) ble3;
    value[13]= (uint8_t) (ble3 >> 8);
    value[14]= (uint8_t) (ble3 >> 16);
    value[15]= (uint8_t) (ble3 >> 24);
    
    delay(100);

    
    // notify changed value
    if (deviceConnected ) {
    pCharacteristic->setValue(value, 16);
    pCharacteristic->notify();
    bCharacteristic->setValue(value1, 1);
    bCharacteristic->notify();
    // delay(90); 
        
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }

    adc_val = analogRead(analogPin);     // read the input pin
    
    battery += adc_val;
   
    if(bat_count == 9){           //taking average of 10 values.
    battery = (battery/10);

    battery=((battery*2)-400);
      
    if (battery > 4100)
    {
     battery = 4100;
    }
    else if(battery < 3600 )
    {
     battery = 3600;
    }
    if (startup_flag == true)
    {
     bat_prev = battery;
     startup_flag = false;
    }
     bt_rem = (battery % 100);                 

    if(bt_rem>80 && bt_rem < 99 && (bat_prev != 0))
    {
     battery = bat_prev;
    }

    if((battery/100)>=41)
    {
     battery = 100;
     
    }
    else if((battery/100)==40)
    {
     battery = 80;
     
    }
    else if((battery/100)==39)
    {
     battery = 60;
     
    }
    else if((battery/100)==38)
    {
     battery=45;
    
    }
    else if((battery/100)==37)
    {
     battery=30;
    
    }
    else if((battery/100)<=36)
    {
     battery = 20;
    
    }

    bat_percent = (uint8_t) battery;
    Serial.print("\n ");
    Serial.print(bat_percent);
     
    value1[0]= (uint8_t) bat_percent;

    bat_count=0;
    battery=0;
    }
    else
      
    bat_count++;
    delay(100);
}

float convertToMilliV(int32_t i32data)
{
    return (float)((i32data*VFSR*1000)/FULL_SCALE);
}
