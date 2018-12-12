---
layout: page
homepage: true
---
![Sensything](images/sensything-top-iso.jpg)

If you don't already have one, you can now buy Sensything from [Crowd Supply](https://www.crowdsupply.com/protocentral/sensything)

## Getting started with the Sensything Basic kit

Sensything kit was put together to explore the option of the ** “out of the box”** solution, which meant that you were ready to without any additional programming required. To help new users to get started we have compiled some pointers to make it easier.

# What’s in the box?
{Kit contents}

# Unboxing the contents an powering up

This video throws light on how to set up your Sensything device.
Video:Unboxing

### Installation of Sensything
Downloading the App and connecting couldn’t have been much easier.
Video: Play store

### Using the App
Easy connect with just a click. Pay attention to the RGB indication.
Video: Using the App

## Data Acquisition using sensors
There are two major categories of sensors that is compatible with the Sensything board. They are
1) Analog sensors
2) Qwiic sensors

#Analog sensors
{Analog}
Steps to be followed:

- Open the Arduino software and write the code for the sensor you have chosen
- From tool bar select tools and select the board as ESP32 dev Module
- Select the port of the board connected in the tool bar
- Compile the code and check for no errors occurred
- Make the connection of the sensor with Sensything
- Connection between the sensor and Sensything
- + pin---------------> A0/A1/A2/A3 (any analog channels)
- - pin--------------->Gnd
- Upload the code to Sensything once connection is done
- You can see the output in the serial monitor

Some basic examples of how to connect to Analog sensors

Experiments

1)How clean is the Air around you?
Aim : To determine the quality of Air using Grove- Air quality sensor
Procedure:
Video:Air quality

Programming:
{Air quality}

2)Is there any alcohol content in the liquid?
Aim: To determine the level of Alcohol in any liquid
Procedure: {Alcohol sensor1}
Video: Alcohol sensor

3)What is the liquid level?
Aim: To measure the level of water in a container
Procedure: Liquid level

#Qwiic sensors

{Qwiic connections}

Example of Qwiic sensors
{BMP 180}


## Using Sensything with Arduino

This document explains how to connect your Sensything to the computer and upload your first sketch. The Sensything is programmed using the Arduino Software (IDE), the Integrated Development Environment runs both online and offline. Starting the basics of electronics, to more complex projects, the kit will help you control the physical world with sensor and actuators.
Setting up Arduino to ESP32
Welcome to sensything with arduino! Before you start controlling the world around you, you’ll need to set up the software to program your Sensything.

* Step 1: Download and Install the IDE

The Arduino Software (IDE) allows you to write programs and upload them to your sensything. Now you require arduino Desktop IDE you can download the latest version using the below link. **https://www.arduino.cc/en/Main/Software#download**
{Download}
Once downloaded, install the IDE and ensure that you enable most (if not all) of the options, INCLUDING the drivers.

* Step 2: Get the Sensything COM Port Number
Next, you’ll need to connect the Sensything board to the computer. This is done via a USB connection. When the Sensything is connected, the operating system should recognize the board as a generic COM port. The easiest way to do this is to type “device manager” into Windows Search and select Device Manager when it shows.
{device manager}
In the Device Manager window, look for a device under “Ports (COM & LPT)”, and chances are the Arduino will be the only device on the list

* Step 3: Configure the IDE
Now that we have determined the COM port that the Arduino is on, it’s time to load the Arduino IDE and configure it to use the same device and port. Start by loading the IDE. Once it’s loaded, navigate to Tools > Board > Esp32 dev module.

{IDE}

Next, you must tell the IDE which COM port the Sensything is on. To do this, navigate to Tools > Port > COM51. Obviously, if your Sensything is on a different port, select that port instead.

{Selecting port}

##Writing a simple ADS1220 acquisition
To load the simple ads1220 data acquisition example, download the arduino library from  https://github.com/Protocentral/Protocentral_ADS1220 click File > Open > Protocentral_ADS1220.
{ADS1220_read}

With the ads1220 acquisition loaded, it’s time to verify and upload the code. The verify stage checks the code for errors, then compiles the ready-for-uploading code to the Arduino. The upload stage actually takes the binary data, which was created from the code, and uploads it to the Sensything via the serial port.

To verify and compile the code, press the check mark button in the upper left window.
{Read sequential}

If the compilation stage was successful, you should see the following message in the output window at the bottom of the IDE. You might also see a similar message—just it’s one that does not have words like “ERROR” and “WARNING”.
This is a successful compilation.
{Compiling}
With the code compiled, you must now upload it the Sensything. To do this, click the arrow next to the check mark.
The “Upload” button will program the Arduino with your code.
{Read include}
 You can get the ads1220 4-channels analog readings in the Serial Monitor.
{Analog readings}
{ADS1220 Acquistion}

# License Information

This product is open source! Both, our hardware and software are open source and licensed under the following licenses:

## Hardware

All hardware is released under [Creative Commons Share-alike 4.0 International](http://creativecommons.org/licenses/by-sa/4.0/).

![CC-BY-SA-4.0](https://i.creativecommons.org/l/by-sa/4.0/88x31.png)

## Software

All software is released under the MIT License(http://opensource.org/licenses/MIT).

Please check [*LICENSE.md*](LICENSE.md) for detailed license descriptions.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
