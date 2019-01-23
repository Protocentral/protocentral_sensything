---
menu: Experiment 2 - Water level sensor
parent: connecting-sensors.md
weight: 1
---

### Experiment 2 - Water level check

#### * Objective
To determine the level of any liquid.

#### * Application
It can determine the continuous liquid level monitoring of water, non-corrosive water or dry fluids.

#### * Procedure
The eTape Liquid Level Sensor is a solid-state sensor with a resistive output that varies with the level of the fluid. It does away with clunky mechanical floats, and easily interfaces with electronic control systems. The eTape sensor's envelope is compressed by the hydrostatic pressure of the fluid in which it is immersed. This results in a change in resistance that corresponds to the distance from the top of the sensor to the surface of the fluid. The sensor's resistive output is inversely proportional to the height of the liquid: the lower the liquid level, the higher the output resistance; the higher the liquid level, the lower the output resistance.

Here we can calculate the output resistance from converting the ADC data in sensything. Below we measure the e-tape liquid level sensor output resistance.

![Etape Sensor](images/etape.png)

#### * Excerpts from the code:

```c
float Vout = (float)((bit32*VFSR*1000)/FSR);     //In  mV
// convert the value to resistance
 float reading = (1023 / Vout)  - 1;
  reading = 560 / reading;
  Serial.print("Sensor resistance ");
  Serial.println(reading);
```

[Download the etape Liquid level code](https://github.com/Protocentral/protocentral_sensything/tree/master/software/Sensything_Arduino/experiments/Analog_Sensors/examples/sensyThing_etape)

#### * Pin mapping and connection instructions:

|Sensything pin label| etape liquid Sensor   |
|----------------- |:--------------------:|
| A1             | Pin 3                |              
| 3V3              | Pin 2   |
| GND               | Pin 3-Across resistor |

|Connection Instructions | 
|----------------- |
| Place the etape liquid level sensor to any of the analog pins A1, A2, A3 and A4.             |   
| Here we are using A1.             | 
| Connect pin2 of etape with Vcc in Sensything.            | 
| Connect pin3 of sensor with Analog1 in Sensything.             |
| Using a breadboard connect a resistor between pin3 in sensor and GND in breadboard.        | 
| Connect GND across the resistor with the GND pin in Sensything.            | 

