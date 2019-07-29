# AR488 Arduino GPIB Interface


The AR488 GPIB controller is an Arduino-based controller for interfacing with IEEE488 GPIB devices via USB. This work was inspired by and has been based on the work originally released by Emanuele Girlando and has been released with his permission.

This sketch represents a re-write of that work and implements the full set of Prologix ++ commands (with the exception of the ++lon device mode command), and also provides a macro feature as well as Bluetooth support. Secondary GPIB addressing is not yet supported.

To build an interface, at least one Arduino board will be required to act as the interface hardware. The Arduino UNO R3, NANO and MEGA2560 boards are currently supported and provide a low cost alternative to other commercial interfaces. The UNO and NANO are the most cost-effective of these boards and are based around the ATmega328P processor. They share the same pinout and features and the AR488.ino sketch can be used for either of these boards.

The MEGA2560 costs slightly more but has a different design with more control pins, communication ports and a more powerful ATmega2560 processor. While this provides more flexibility and potential for further expansion, it also has a different pin mapping scheme to the UNO or NANO. The AR488.ino sketch will NOT work on the MEGA2560 board. Instead, the sketch named AR488-mega.ino should be used. This has been designed to work with the pin layout of the MEGA2560. Pins 2-5, 12, 13, analog pins A8 to A15, as well as the entire "digital" connector (pins 22 to 53) at the end of the board are not used by the sketch and remain available for adding displays and other devices. Please be aware that pins 16 ands 17, corresponding to TXD2 and RXD2 (the Serial2 port) have been used by the sketch and cannot be used for serial communication, however serial ports 0, 1 and 3 remain available for use.

Connecting to an instrument will require a 16 core cable and a suitable IEEE488 connector. This can be salvaged from an old GPIB cable or purchased from various electronics parts suppliers. Details of interface construction and the mapping of Arduino pins to GPIB control signals and data bus are explained in the "Building an AR488 GPIB Interface" section of the AR488 manual.
 
Commands generally adhere closely to the Prologix syntax, however there are some minor differences, additions and enhancements. For example, due to issues with longevity of the Arduino EEPROM memory, the ++savecfg command has been implemented differently to save EEPROM wear. Some commands have been enhanced with additional options and a number of new custom commands have been added to provide new features that are not found in the standard Prologix implementation. Details of all commands and features can be found in the Command Reference section of the AR488 manual:
https://github.com/Twilight-Logic/AR488/blob/master/AR488-manual.pdf

Information on adding Bluetooth support can be found in the AR488 Bluetooth supplement:
https://github.com/Twilight-Logic/AR488/blob/master/AR488-manual.pdf.

For the UNO or NANO, the sketch AR488.ino should be uploaded to the the completed interface board. For the MEGA2560, the sketch AR488-mega.ino should be used. Once uploaded, the firmware should respond to the ++ver command with its version information.

In the event that a problem is found, this can be logged via the Issues feature on the AR488 GitHub page. Please provide at minimum:

- the firmware version number
- the type of board being used
- the make and model of instrument you are trying to control
- a description of the issue including;
- what steps are required to reproduce the issue

Further work is in progress.

Comments and feedback can be provided here:
https://www.eevblog.com/forum/projects/ar488-arduino-based-gpib-adapter/

The original work by Emanuele Girlando is found here:
http://egirland.blogspot.com/2014/03/arduino-uno-as-usb-to-gpib-controller.html

