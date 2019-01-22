The Sensything board comes with the feature -

**Bluetooth 4.2 / Bluetooth Low Energy (BLE) compatible**

In order for the device to connect with the Sensything Application, the Arduino code must be set up.

### How to set up the BLE function?

The Sensything  Application has been designed to simplify the detection of sensor values. With the inbuilt ADC (ADS1220) the precise values can be detected and displayed. Few simple steps:

### Step 1: Writing the code
Using the Arduino IDE, write the code for the sensor of your choice. Refer to the above mentioned examples to get tips on writing the code.

```c
    adc_data=pc_ads1220.Read_SingleShot_SingleEnded_WaitForData(MUX_SE_CH0); // Getting analog channel 1 value
    float vout = convertToMilliV(adc_data);
    
    int32_t ble = (int32_t) (vout * 100);
    ads1220_data[0]= (uint8_t) ble;
    ads1220_data[1]= (uint8_t) (ble >> 8);
    ads1220_data[2]= (uint8_t) (ble >> 16);
    ads1220_data[3]= (uint8_t) (ble >> 24);
    
    delay(1000);

    // notify changed value
    if (deviceConnected ) {
    pCharacteristic->setValue(ads1220_data, 16);
    pCharacteristic->notify(); 
    }
```

**http://sensything.protocentral.com/sensything-with-arduino.html#step-4-writing-my-first-code-to-sensything**

### Step 2: Connecting the sensor
The next step is to connect the sensors to the Sensything boards. Sensors that can be used are both Analog and Qwiic.

**http://sensything.protocentral.com/sensything-with-arduino.html#connecting-analog-sensors-to-sensything**

### Step 3: Uploading the code on to the board
Connect the Sensything board to your system using a USB cable. Once the code is ready, go on to upload it on to the board.

### Step 4: Using the Sensything Application
Kindly refer to the Introduction video on how to download the Application and how to connect.

**http://sensything.protocentral.com/#making-the-connections**

### Step 5: Reading the sensor values
Kindly refer to the Application section on the main page

**http://sensything.protocentral.com/#understanding-the-sensything-application**
