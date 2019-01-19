---
menu: FAQ
weight: 4 # smaller weights rise to the top of the menu
---

### Frequently asked questions

* How do I turn on the device?

 <img width="250" height="300" src="images/switch button.jpg"> 

Firstly, connect the battery to the Sensything board, then switch on the board.

| Question                                           | Solution                                                                                                                                                                      |
|----------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Which microcontroller is used?                     | The microcontroller used is ESP-PICO-D4 SIP with 4 MB Flash.                                                                                                                  |
| Name of the ADC?                                   | Texas Instruments ADS1220 24-bit Sigma-Delta                                                                                                                                  |
|                                                    |                                                                                                                                                                               |
| Is the sensor data accurate?                       | With an inbuilt ADC,ADS1220 ensure 24-bit resolution for sensor data.                                                                                                         |
| How do I make the Analog connections?              |  A1 - Channel 1                                                                                                                                                               |
|                                                    | A2 - Channel 2                                                                                                                                                                |
|                                                    | A3 - Channel 3                                                                                                                                                                |
|                                                    | A4 - Channel 4                                                                                                                                                                |
|                                                    | 3V3 - Supply - Board which supports 3.3V                                                                                                                                      |
|                                                    | GND - Ground                                                                                                                                                                  |
| What is the input range?                           | The input range is ± 2.048 V to ± 0.016 V (adjustable gain).                                                                                                                  |
| What is the Sampling  rate for Analog?             | The sampling rate is 20 samples/second (SPS) to 2000 SPS.                                                                                                                     |
| What are the current sources?                      | There is an On-board excitation current sources adjustable from 10 uA to 1500 uA.                                                                                             |
| How many I/O pins are there?                       | There are 4 general-purpose I/O pins.                                                                                                                                         |
| How can I determine the BLE communication?         | Once connected through the BLE, the RGB LED will turn OFF.                                                                                                                    |
|                                                    | When it is disconnected then the RGB LED turns ON.                                                                                                                            |
| What is the digital supply voltage?                | It is 3.3 V.                                                                                                                                                                 |
| What are the board dimensions?                     | 47x57 mm (length, width),  5 mm height (board only),  12 mm height (with included battery stacked)                                                                            |
| Can i use the Qwiic option for connecting sensors? | There are two Sparkfun Qwiic-compatible I2C ports on board.                                                                                                                   |
| What is the range of wireless frequency?           | There is a 2.4 GHz radio with on-board PCB antenna.                                                                                                                           |
| What is the voltage reference?                     | Built-in 2.048 V voltage reference with 5 ppm/°C drift.                                                                                                                       |
| Any inbuilt sensor?                                | There is an built-in temperature sensor with 0.5°C accuracy.                                                                                                                  |
| What is the power source?                          | There is a Li-ion battery (3.7 V, 1000 mAH) as power source.This would be applicable to those who ordered the basic kits.                                                    |
| How do I charge the battery?                       | There is an On-board USB battery charger.Ensure that the battery is plugged to the board and then connect the USB cable.  You can use a 5v power adapter as the power source. |
