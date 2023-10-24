# pulsemote
Control eStim boxes like the DG-Labs 2.0 with an ESP32 based controller.

The inexpensive DG-Labs eStim box is popular but requires you to use
a phone app to control it. In many private settings where you might
want to use the box phones are not allowed, so this was the solution
to allow a minimal control over the box without a phone needed or having
to make your own hardware.

<img src="https://github.com/erotronik/pulsemote/assets/89390295/466b2ae5-c6d8-4237-ad80-009858b15b56" alt="pic" width="300"/>

## Notes

* The current code is for Arduino IDE and a M5Stack Core controller (one
that has 3 physical buttons). But it should be possible to alter it
for anything based on an ESP-32 with a display and at least 3 buttons.

* The DG-Labs 2.0 box has no authentication or pairing, so if you use
one in a setting where there are other people please be aware that anyone
with the app (or a box like this one) can take control of it if it's
not connected to any other client at the time. Make sure you've connected
to the right box!

* The current code just has one mode, a copy of the apps "breath", and
you can turn it on and off or set it to a random on/off mode. More in the
future.

## Instructions for use

* After turning on the box it will scan for a DG-Labs device that is turned on and not connected to anything else
* Use the left button to change what you want to update (either the level of output A, level of output B, or the mode), it will be highlighted with a different colour
* Use the other buttons to alter what is highlighted, either the levels of A or B (in percentage) or the mode (between 'off', 'on', or a mode which turns on and off at a random interval)
* For safety when holding the + button it will repeat but only up to 10% then you need to push and hold again (or rapid tap).  The maximum output is capped at 70% but that's a level in the code that can be changed.

## Requirements

* A DG-Labs 2.0 box
* A M5Stack Core controller
* Install the M5Stack board support and M5Stack library
* Install the NimBLE library (used for bluetooth)
* Install the LovyanGFX library (used for the GUI)
