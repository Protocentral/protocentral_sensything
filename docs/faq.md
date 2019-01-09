---
menu: FAQ
weight: 3 # smaller weights rise to the top of the menu
---

# Frequently asked questions

* How do I turn on the device?

![switch](images//switch.jpg)

Firstly, connect the battery to the Sensything board, then switch on the board.

| Situation                        | Solution                                                                                                                             |
|----------------------------------|--------------------------------------------------------------------------------------------------------------------------------------|
| How do I charge the battery?     | Ensure that the battery is plugged to the Board and then connect the USB cable.  You can use a 5v power adapter as the power source. |
| How can I determine the BLE communication?      | Once connected through the BLE, the RGB LED will turn OFF.  |
|                                  | When it is disconnected then the RGB LED turns ON     |

## Troubleshooting

| Issue                                  | Solution                                                                |
|----------------------------------------|-------------------------------------------------------------------------|
| If the port is not found?              | You can unplug the USB cable and plug it again                          |
|                                        | Try changing the USB cable                                              |
|                                        | Try with a different system                                             |
| If there is an uploading problem       | Check whether the switch is turned ON                                   |
|                                        | Select the COM port and board in the board manager.                     |
|                                        | Check for the latest version of arduino ide and esp-idf                 |
| If the RGB does not power up           | Download and upload the latest firmware from Github                     |
| Issues with channel values ?           |                                                                         |
| -> No values                           | Download and upload the latest firmware from github                     |
| -> Wrong values                        | Try giving fixed input values for the channels and read                 |
| -> Noise(Disturbance between channels) | Test if the 3.3V and 5V power source is proper                          |
| -> Cross talk                          | If one analog channel is in use the other channels need to be grounded. |
| What if BLE device is not found?       | Check if your mobile device supports BLE                                |
|                                        | Check whether the device is within the prescribed BLE range             |
|                                        | Allow the application to turn BLE ON in your device.                    |
