# Sensything 
Sensything development repo

**ads1220**

|ADS1220 Pin |Sensything Pin Connection|
|-----------------|-----------------|
| MISO            |  19              | 
| MOSI            |  23              |
| SCLK            |  18              |
| DRDY            |  34              |


**QWIIC**

|QWIIC Pin |QWIIC 1 Pin Connection|QWIIC 2 Pin Connection|
|-----------------|-----------------|--------------------|
| SCL            |  22              | 25
| SDA            |  21              |26


**battery monitor**

ADC ---> 2   
 
```diff
+ Please configure sensything through makemenuconfig
```
* You can enable different channel under makemenuconfig-->sensything configuration-->adc channels
* You can also enable ble, wifi, rgb led through the same sensything configuration
* This code will work with esp-idf release 2.1 and 3.0, please select your idf version from sensything configuration-->select esp-idf version


## BLE DATA FORMAT


![#1589F0](https://placehold.it/15/1589F0/000000?text=+) ADS1220 ![#1589F0](https://placehold.it/15/1589F0/000000?text=+) 

ADS1220 data is send through a proprietary BLE service, it also has a standard [battery service](https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.battery_service.xml) to dispaly the battery value.

* data type : float
* service UUID : cd5c 1105 4448 7db8 ae4c d1da8cba36d0
* char UUID :    cd5c 1106 4448 7db8 ae4c d1da8cba36d0
* read voltage in float from ads1220 is multiplied by 100 and converted to int.
* 4byte int data is added to bledata packet, 

  bledatapacket[0] = (uint8_t) ads1220 data  [lsb]
  :
  bledatapacket[3] = (uint8_t) ads1220 data >> 24  [msb]
  
* Ble datapacket[0-3] = channel 1 data
* Ble datapacket[4-7] = channel 2 data
* Ble datapacket[8-11] = channel 3 data
* Ble datapacket[12-15] = channel 4 data



![#1589F0](https://placehold.it/15/1589F0/000000?text=+) QWIIC ![#1589F0](https://placehold.it/15/1589F0/000000?text=+) 

Data from QWIIC sensors is send through another proprietary service

* service UUID : cd5c 1100 4448 7db8 ae4c d1da8cba36d0
* char UUID :    cd5c 1101 4448 7db8 ae4c d1da8cba36d0

It consist of 1byte sensor address and 5 sensor data channels each of 2bytes length
The sensor data length varies based on different sensors

|sensor address |LSB data channel 1| MSB of data channel 1 |   ..... |MSB of data channel 5|
|-----------------|-----------------:|-----------------|------------|--------------------|
|   byte 0   | byte 1    | byte 2          | ........|byte 10 |




