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

#### * Pin mapping and connection instructions:

<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{font-family:Arial, sans-serif;font-size:14px;padding:10px 5px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;border-color:black;}
.tg th{font-family:Arial, sans-serif;font-size:14px;font-weight:normal;padding:10px 5px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;border-color:black;}
.tg .tg-baqh{text-align:center;vertical-align:top}
.tg .tg-s268{text-align:left}
.tg .tg-nk0m{font-size:16px;font-family:Tahoma, Geneva, sans-serif !important;;text-align:left;vertical-align:top}
</style>
<table class="tg">
  <tr>
    <th class="tg-s268"><span style="font-weight:600">Sensything pin label</span></th>
    <th class="tg-nk0m"><span style="font-weight:bold">BMP180 Sensor</span></th>
  </tr>
  <tr>
    <td class="tg-baqh">sda</td>
    <td class="tg-baqh">sda</td>
  </tr>
  <tr>
    <td class="tg-baqh">scl</td>
    <td class="tg-baqh">scl</td>
  </tr>
  <tr>
    <td class="tg-baqh">3V3</td>
    <td class="tg-baqh">Vcc</td>
  </tr>
  <tr>
    <td class="tg-baqh">GND</td>
    <td class="tg-baqh">GND</td>
  </tr>
</table>

<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{font-family:Arial, sans-serif;font-size:14px;padding:10px 5px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;border-color:black;}
.tg th{font-family:Arial, sans-serif;font-size:14px;font-weight:normal;padding:10px 5px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;border-color:black;}
.tg .tg-s6z2{text-align:center}
.tg .tg-0lax{text-align:left;vertical-align:top}
</style>
<table class="tg">
  <tr>
    <th class="tg-s6z2"><span style="font-weight:bold">Connection Instructions</span></th>
  </tr>
  <tr>
    <td class="tg-0lax">- Connect the Qwiic BMP180 directly into the sensything Qwiic-1 or Qwiic-2 port</td>
  </tr>
</table>
