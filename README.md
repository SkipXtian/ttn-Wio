# GCLora Wio

An Open source weather station that reports tempurature, humidity, barometric pressure and has a RJ11 plug to suit a WH Series tipping bucket rain guage. It has a RFM96 Radio module mounted on the back, transmitting in the 915 - 928 MHz band.

BME280 I2C Address is 0x76
Rain Gauge is on Pin 7 (INT4)

The Arduino example is to connect this Weather station to "The Things Network" through crowd sourced Gateways.
To see a list of gateways head to https://www.thethingsnetwork.org/map


Instructions:

Clone this repo into a directory,
Locate the secrets.h.template file in Arduino/ttn-Wio/
Copy secrets.h.template -> secrets.h and copy details from your TTN console.

This code depends on the lmic library - please make sure you use the version from https://github.com/secluded/arduino-lmic as we made a couple of small changes.
Remove all debugging from config.h for it to compile small enough.

Upload using the Arduino IDE the "Lilypad Arduino USB" board.

In your TTN console you should see data start to arrive. The example is configured to transmit every 5 minutes. You can change this in ttn_Wio.ino, look for TX_INTERVAL = 300; Unit is seconds.

The data should appear in this format: 
"32 36 32 38 2C 31 30 31 38 34 36 2C 35 35 34 33 2C 30" Which translates to: <br>
  2  6  2  8  ,  1  0  1  8  4  6  ,  5  5  4  3  ,  0<br>
26.28degress C,      1018.46hPa    ,55.43% Humidity, 0 Rain Ticks

The Tempurature,Barometric and Humidity are divided by 100 to give the decimal places.
Rain Ticks is the count of Rain bucket tipping, multiplying this by 0.2794 will give you rainfall in mm.
We left the processing of Rain for the purpose of indoor use where the sensor might be counting door usage, etc.
