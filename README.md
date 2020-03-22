
# LeakDetectorIO

This project is a simple wifi leak detector based on Blynk. It wakes up every 5 minute, check for leak and sleep to save battery. If leak is detected it sends message on the phone via Blynk app.

Hardware:
ESP-01S, in which GPIO16 must be manually connected to reset pin to wakeup the ESP
after the deep sleep cycle (https://www.youtube.com/watch?v=foljGPuLjYk). It is tricky to do that but doable.

Schematics:
Find the schematics.png in this repository. Rest pin, GPIO0 and GPIO2 must be pulled up using a 5K resistor to VCC, my experiment showed it is much more robust this way.
In order to trigger the leak detector message, the INPUT pin (3 = Rx) needs to pulled down. According to the schematics, when water reaches to the exposed pins, the Input pin is pulled down using the PNP transistor. 
Supply no more than 3V as input, otherwise ESP consumes much more power in deep sleep mode.

Software:
VSCode+PlatformIO extension to compile and upload this firmware
Blynk App, with Display widget and RTC widget

Setup:
Setup Blynk App, add your ESP8266 device and get the auth code
Add a display widget on Virtual port V1, with refresh rate of 2 min
Power the circuit, on your PC search for IOTxxx AP,  and connect to it,
In your browser, goto http://192.168.4.1, enter your Blynk auth and access point information
(server blynk-cloud.com, port 443)

If ESP fails to connect to Blynk after 80 sec, it goes to config mode and you can upload new settings from http://192.168.4.1

Tested Scenarios:
ESP goes to sleep when wifi is off, OK
ESP goes to config mode if wifi password changes, OK
ESP goes to config mode if SSID changes, OK
ESP works if Blynk project gets deleted and remade, OK
ESP wakes up after 1 hour sleep, OK
