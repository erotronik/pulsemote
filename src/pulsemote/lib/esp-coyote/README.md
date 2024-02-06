# CoyoStim

This library lets you control the DG-Labs 2.0 estim box (aka Coyote) with ESP-32 based boards.

The DG-Labs eStim box is popular but requires you to use
a phone app to control it. This library facilitates the development of
small applications capable of controlling it.

For an example project using this library, please look at [pulsemote](https://github.com/erotronik/pulsemote).

# Notes

* The DG-Labs 2.0 box has no authentication or pairing, so if you use
one in a setting where there are other people please be aware that anyone
can take control of it if it's
not connected to any other client at the time. Make sure you are connecting
to the right box!

* The current code has two modes, a copy of the apps "breath", and a custom
"waves" mode.
New modes can be added via a callback function.

* The maximum output is capped at 50% but that's a level in the code that can be changed.

# Requirements

* EP32 board, using either Arduino or ESP-IDF frameworks

* When using Arduino, you need to use [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) as the BLE stack.

* When using ESP-IDF, you need to enable NimBLE and use [esp-nimble-cpp](https://github.com/h2zero/esp-nimble-cpp)

# Example

The [pulsemote project](https://github.com/erotronik/pulsemote) uses this library.