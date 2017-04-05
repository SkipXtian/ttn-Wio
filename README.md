# ttn-Wio
Arduino Code for a our weather station that publishes to TTN using LoRaWan

Copy secrets.h.template -> secrets.h and copy details from your TTN console.

Also this code depends on the lmic library - please make sure you use the version from https://github.com/secluded/arduino-lmic as we made a couple of small changes.
And you have to remove all debugging from config.h for it to compile small enough.