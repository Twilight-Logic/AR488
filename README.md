# AR488 Arduino GPIB Interface


The AR488 GPIB controller is an Arduino-based controller for interfacing with IEEE488 GPIB devices via USB. This work was inspired by and has been based on the work originally released by Emanuele Girlando and has been released with his permission.

This sketch represents a re-write of that work and implements the full set of Prologix ++ commands (with the exception of the ++lon device mode command), and also provides a macro feature as well as Bluetooth support. Secondary GPIB addressing is not yet supported.

To build an interface, at least one Arduino board will be required to act as the interface hardware. The Arduino Uno R3, Nano and Mega 2560 boards are currently supported and provide a low cost alternative to other commercial interfaces. The Uno and Nano share the same pinout and features and nearly all pins are used to interface with the GPIB busa. Only tow pins (6 & 13) remain spare. The Mega 2560 costs slightly more but has a different design and layout with more control pins, communication ports and a more powerful ATmega2560 processor. This provides more flexibility and potential for further expansion. Pins 2-5, 13, analog pins A8 to A15, as well as the entire "digital" connector (pins 22 to 53) at the end of the board are not used by the sketch and remain available for adding displays and other devices. Please be aware that pins 16 and 17, corresponding to TXD2 and RXD2 (the Serial2 port) have been used by the sketch and cannot be used for serial communication, however serial ports 0, 1 and 3 remain available for use.

To use the sketch, create a new directory and unpack the .zip file into this directory. Open the main sketch, AR488.ino, in the Ardduino IDE. This should also load all of the linked files. Have a look at Config.h and make any adjustment you require there (see the AR488 manual for details) and then compile and upload the sketch. There should be no need to change anything else in any other file. Once uploaded, the firmware should respond to the ++ver command with its version information.

Connecting to an instrument will require a 16 core cable and a suitable IEEE488 connector. This can be salvaged from an old GPIB cable or purchased from various electronics parts suppliers. Details of interface construction and the mapping of Arduino pins to GPIB control signals and data bus are explained in the "Building an AR488 GPIB Interface" section of the AR488 manual.
 
Commands generally adhere closely to the Prologix syntax, however there are some minor differences, additions and enhancements. For example, due to issues with longevity of the Arduino EEPROM memory, the ++savecfg command has been implemented differently to save EEPROM wear. Some commands have been enhanced with additional options and a number of new custom commands have been added to provide new features that are not found in the standard Prologix implementation. Details of all commands and features can be found in the Command Reference section of the AR488 manual:
https://github.com/Twilight-Logic/AR488/blob/master/AR488-manual.pdf

Once uploaded, the firmware should respond to the ++ver command with its version information.

Please note that Bluetooth support has been temporarily removed in this version as the code needs to be updated to work with the current version. The previous version is still available in thr Archive/Uno or Archive/Mega directory if required.

In the event that a problem is found, this can be logged via the Issues feature on the AR488 GitHub page. Please provide at minimum:

- the firmware version number
- the type of board being used
- the make and model of instrument you are trying to control
- a description of the issue including;
- what steps are required to reproduce the issue

Further work to add WiFi and additional boards is in progress.

Comments and feedback can be provided here:
https://www.eevblog.com/forum/projects/ar488-arduino-based-gpib-adapter/

The original work by Emanuele Girlando is found here:
http://egirland.blogspot.com/2014/03/arduino-uno-as-usb-to-gpib-controller.html

