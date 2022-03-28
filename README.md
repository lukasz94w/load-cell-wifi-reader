# load-cell-wifi-reader
The code included in this repository was used as the microcontroller software controlling the operation of the device measuring the output voltage of the strain gauge beam. It was developed on the basis of the previous project (https://github.com/lukasz94w/load-cell-bluetooth-reader), but now the communication interface through which the measurement result is sent is WiFi, not Bluetooth. This allows to achieve a greater range from which the communication with the device is possible (Bluetooth - 10m, WiFi - about 100m). Other changes include change of the method of triggering the measurement function. Earlier it was done by adding an artificial timeout between measurements, now it is triggered in interrupts.

Block diagram of the device and its photo are presented below. It can be observed new ESP8266 chip (WiFi module) used instead of Bluetooth BTM162:

![image](https://user-images.githubusercontent.com/53697813/160007311-cb9dda8b-147e-477a-b78c-eac4d16e0304.png)

![received_1233340783539081](https://user-images.githubusercontent.com/53697813/160159929-9cbf959c-1938-46f5-82dd-83c87af624e6.jpeg)

![received_2313487058951506](https://user-images.githubusercontent.com/53697813/160159950-069f74f4-6c92-42ff-a753-c5a593d2f879.jpeg)


