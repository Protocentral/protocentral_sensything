---
layout: page
homepage: true
---

# *This page is still work in progress. Please check back for updates*

![Sensything](images/sensything-top-iso.jpg)

Sensything is a Data- acquisition board with the added capabilities of ADC precision (ADS1220). The board contains an ESP-32 microcontroller with Qwiic connectivity that is most suitable for IOT applications. Using one single platform to monitor sensor readings along with the support of an BLE Application and Web server just makes it all the more advanced and simplifies solutions.

If you don't already have one, you can now buy Sensything from [Crowd Supply](https://www.crowdsupply.com/protocentral/sensything)

# Getting started with the Sensything Basic kit

The Sensything basic kit is put together to work **out of the box**, which means that it is ready to use without any additional programming required.

### What’s in the box?
* 1x Sensything main Board
* 1x 1000 mAH, 3.7V Li-Po battery
* 10x alligator cables
* 1x micro USB cable

If you have purchased the *Sensything - board only* version then you will have to bring your own battery, and cables.
This video throws light on how to set up your Sensything device. Get set Go!

<iframe  width="640" height="360" src="https://player.vimeo.com/video/306863926" frameborder="0" allowFullScreen mozallowfullscreen webkitAllowFullScreen></iframe>

### Making the connections
1) Downloading the App and connecting the device

**Note**: The Sensything App is currently available for Android users on Google Play store. The ios version will be coming soon. Stay tuned for updates!

<iframe  width="640" height="360" src="https://player.vimeo.com/video/307040678" frameborder="0" allowFullScreen mozallowfullscreen webkitAllowFullScreen></iframe>

2) Basic connections of Analog sensors

They can be listed as follows:

• A1 - Analog Channel 1

• A2 - Analog Channel 2

• A3 - Analog Channel 3

• A4 - Analog Channel 4

• GND - Ground

• 3.3v - Power

A short video suggests how to connect a basic analog sensor like the Piezo vibration sensor to Sensything.

<iframe  width="640" height="360" src="https://player.vimeo.com/video/307044875" frameborder="0" allowFullScreen mozallowfullscreen webkitAllowFullScreen></iframe>

# Understanding the Sensything Application

![sensything_app_1](images//sensything_app_1.png)

This above image is a screenshot of the opening page of the Application. This page displays the following information
* Details of the detected Sensything device
* The"Connect" option for the device.

![sensything_app_2](images//sensything_app_2.png)

The main page of the Application can be segmented as follows:
* Graphical representation - This depicts the changes in the readings of the sensors. All major and minor fluctuations are picked up.
* Analog channels - The four channels gives the ADC values received from the plug ins on the board.
* Qwiic section - Incase any qwiic connection is made the values will be shown there.
* Data logging - Using the Application you can log your data, so that the data is saved directly to your mobile device.

**Note**: On the right hand corner there is a battery indicator, this detects the level of charge in the Sensything device.

Follow the link above on how to set up the Arduino IDE for Sensything.

# License Information

This product is open source! Both, our hardware and software are open source and licensed under the following licenses:

## Hardware

All hardware is released under [Creative Commons Share-alike 4.0 International](http://creativecommons.org/licenses/by-sa/4.0/).

![CC-BY-SA-4.0](https://i.creativecommons.org/l/by-sa/4.0/88x31.png)

## Software

All software is released under the MIT License(http://opensource.org/licenses/MIT).

Please check [*LICENSE.md*](LICENSE.md) for detailed license descriptions.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
