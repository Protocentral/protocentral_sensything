---
menu: Experiment 1 - Alcohol Sensor
parent: connecting-sensors.md
weight: 0
---
### Experiment 1 - Alcohol detector

#### * Objective
To determine the level of Alcohol in any liquid.

#### * Application
This sensor has a good level of sensitivity it can be used as a portable alcohol detector.

#### * Procedure
MQ303A is a semiconductor sensor for Alcohol detection. It has very good sensitivity and fast response to alcohol, suitable for portable alcohol detector just plugging with sensything. Below you find the conversion of ADC data to the content of alcohol to be detected with milligram per litre. When the content of alcohol is more than 0.8 it detects the presence of alcohol.

![Alcohol Sensor](images/alcohol-sensor.jpg)

#### * Excerpts from the code:

```c
float adc_data = (float)((bit32*VFSR*1000)/FSR);     //In  mV
float v = (adc_data/10) * (5.0/1024.0);
float mgL = 0.67 * v;

if(mgL > 0.8)
{   
	Serial.print("mg/L : %f \n");
	Serial.print(" Alcohol Detected");
	Serial.println(mgL);
 }
else
{    
	Serial.print("mg/L : %f \n");
	Serial.print(" Alcohol Not Detected");
	Serial.println(mgL);
}  
```

[Download the Alcohol Sensor code](https://github.com/Protocentral/protocentral_sensything/tree/master/software/Sensything_Arduino/experiments/Analog_Sensors/examples/sensyThing_mq303A)

#### * Pin mapping and connection instructions:


|Sensything pin label| Alcohol Sensor   |
|----------------- |:--------------------:|
| A1             | Analog out                |              
| 3V3              | Vcc   |
| GND                             | GND |

|Connection Instructions | 
|----------------- |
| Place the alcohol sensor to any of the analog pins A1, A2, A3 and A4.             |   
| Here we are using A1.             | 
| Connect Vcc of MQ303A with Vcc in Sensything.            | 
| Connect GND of sensor with GND in Sensything.             | 
| Connect output of sensor with Analog1 in Sensything.             | 
