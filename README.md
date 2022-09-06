# WebFox
Ham Radio Fox with web interface for configuring and starting/stopping

This project is a mashup of code from this ham radio fox project https://wcar.ca/pg/fox-tx/ and some other pieces I found to flush it out.

The idea behind this is to make a fox that anyone in a club could grab and attach to their HT.  Then boot the device up and using a smart phone, enter thier call sign to be broadcast by the fox in CW.
Future additions will be interval between broadcast and CW symbol rate.

This project uses 3 main parts:
  1.) HiLetgo ESP32 OLED WiFi Kit
  2.) HiLetgo 2pcs 5V One Channel Relay Module Relay Switch with OPTO Isolation High Low Level Trigger
  3.) Zopsc 2PCS Electric Microphone Speaker Mic 2 Pins 4 Wires Supplies Cable Line for Baofeng UV5R

Due to some oddities with esp hardware certain pins had to be chosen to prevent strange operation of the ESP.  

The basic operation is as follows:
  1.) Power up the arduino and relay using a 5V source i.e. Anker or similar device
  2.) Follow the onscreen prompts to connect to an open ssid with a smart phone 
  3.) With a browser on the phone go to 192.168.4.1
  4.) Enter the call sign to be used
  5.) Connect HT with dual cable and power on
  6.) Set HT to proper simplex frequency and verify it's clear (may be designated in your area)
  7.) On the web interface, click start and submit to start the fox
  8.) When done, either remove power or click stop and submit in the interface
  
Schematic to come.
