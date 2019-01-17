---
menu: Experiment 3 - Pressure Sensor
parent: Connecting-qwiic-sensors.md
weight: 0
---

### Experiment 3 - Barometric pressure sensor

#### * Objective
To determine the Barometric pressure.

#### * Application

BMP180 barometric pressure sensor can be used to predict the weather, detect altitude, and measure vertical velocity.

#### * Procedure
This sensor is one of the low-cost solutions for sensing applications related to barometric pressure and temperature. The BMP180 can communicate directly with a microcontroller in the device through I2C or SPI as a variant. The applications for this sensor is navigation, GPS positioning as well as a tracker for hikers. We have enabled Qwiic connection using the channels.

![BMP180 Sensor](images/bmp180-sensor.jpg)

#### * Excerpts from the code:
```c
baseline = getPressure();
Serial.print("baseline pressure: ");
Serial.print(baseline);
```
```c
double a,P;

P = getPressure();             // Get a new pressure reading:
a = pressure.altitude(P,baseline); // Show the relative altitude difference between the new reading and the baseline reading

char status;
double T,P;

status = pressure.startTemperature();
status = pressure.getTemperature(T);

status = pressure.startPressure();
status = pressure.getPressure(P,T);
```

[Download the full code here](https://github.com/Protocentral/protocentral_sensything/tree/master/software/Sensything_Arduino/experiments/Qwiic/examples/sensything_bmp180)
