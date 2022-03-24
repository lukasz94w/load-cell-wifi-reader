# my-masters-thesis
The code included in this repository was used as the microcontroller software controlling the operation of the device measuring the output voltage of the strain gauge beam. It was developed on the basis of the previous project (https://github.com/lukasz94w/my-engineering-thesis), but now the communication interface through which the measurement result is sent is WiFi, not Bluetooth. This allows to achieve a greater range from which the communication with the device is possible (Bluetooth - 10m, WiFi - about 100m). Other changes include change of the method of triggering the measurement function. Earlier it was done by adding an artificial timeout between measurements, now it is triggered in interrupts.

Block diagram of the device and its photo are presented below. It can be observed new ESP8266 chip (WiFi module) used instead of BTM162 (Bluetooth):

![image](https://user-images.githubusercontent.com/53697813/160007311-cb9dda8b-147e-477a-b78c-eac4d16e0304.png)

