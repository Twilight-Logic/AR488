# AR488 Arduino GPIB Interface


The AR488 GPIB controller is an Arduino-based controller for interfacing with IEEE488 GPIB devices via USB. This work was inspired by and has been based on the work originally released by Emanuele Girlando and has been released with his permission.

This sketch represents a rewrite of that work and implements the full set of Prologix ++ commands in both controller and device mode, with the exception of ++help. Secondary GPIB addressing is not yet supported. A number of additional features are provided, for example, a macro feature is provided to allow automation of frequently used command sequences was well as controller and instrument initialisation at startup. As of version 0.48.x, interfacing with SN75160 and SN75161 GPIB transceiver integrated circuits is supported.

To build an interface, at least one Arduino board will be required to act as the interface hardware. The Arduino Uno R3, Nano, Mega 2560 and Micro 23u4 boards are currently supported and provide a low cost alternative to other commercial interfaces.

Uno and Nano boards are both based around the ATmega328p micro-controller and have similar pin-out and features. Only two pins (6 & 13) remain spare as the remainder are all used to interface with the GPIB bus. On the Micro, which is based around the ATmega32u4 micro-controller, all GPIO pins are used and none remain spare. The Mega 2560 costs slightly more but has a different design and layout with more control pins, communication ports and a more powerful ATmega2560 processor. This provides more flexibility and potential for further expansion. Currently, 3 layouts are provided for the AtMega2650 using either the lower numbered pins on the sides of the board, or either row of pins on the two row header at the end of the board. This provides a flexible way to allows various displays and other devices to be connected if desired. Please be aware that pins 16 and 17, corresponding to TXD2 and RXD2 (the Serial2 port) have been used by the sketch and cannot be used for serial communication, however serial ports 0, 1 and 3 remain available for use.

Including the SN7516x chipset into the interface design will naturally add to the cost, but has the advantage of providing the full 48mA driving capacity regardless of capability of the Arduino board being used, as well as providing proper tri-state output. The latter isolates the Arduino micro-controller from the GPIB bus when the interface is powered down, preventing GPIB bus communication problems due to 'parasitic power' from signals present on the GPIB bus, thereby allowing the interface to be safely powered down while not in use. Construction will involve adding a daughter-board between the Arduino GPIO pins and the GPIB bus. This could be constructed using prototyping board or shield, or custom designed using KiCad or other PCB layout design software.

To use the sketch, create a new directory, and then unpack the .zip file into this location. Open the main sketch, AR488.ino, in the Arduino IDE. This should also load all of the linked files. Have a look at Config.h and make any adjustment you require there (see the 'Configuration' section of the AR488 manual for details), in particular the board layout selection. Set the target board in the Board Manager of your Arduino IDE, and then compile and upload the sketch. There should be no need to make any changes to any other files. Once uploaded, the firmware should respond to the ++ver command with its version information.

Please note that Arduino Micro (and other 32u4 boards, e.g. Leonardo) do not automatically reset when a connection is made to the serial port. The Arduino IDE takes care of the programming process via USB which should work normally. Some Micro boards may not have a reset button, in which case the reset pin need to be briefly shorted to ground by some other means. When using the Arduino IDE on Linux (Linux Mint and possibly other Ubuntu derivatives), the modemmanager service must be disabled, otherwise it will interfere with the programming process and the boards will be rendered inaccessible via USB. If this curers, then the board can be returned to normal working by uploading a bootloader to it using an AVR programmer. This issue does not seem to affect Uno, Nano or Mega 2560 boards.

Unless some form of shield or custom design with integral IEEE488 connector is used, connecting to an instrument will require a 16 core cable and a suitable IEEE488 connector. This can be salvaged from an old GPIB cable or purchased from various electronics parts suppliers. Searching for a 'centronics 24-way connector' sometimes yields better results that searching for 'IEEE 488 connector' or 'GPIB connector'. Details of interface construction and the mapping of Arduino pins to GPIB control signals and data bus are explained in the "Building an AR488 GPIB Interface" section of the AR488 manual.
 
Commands generally adhere closely to the Prologix syntax, however there are some minor differences, additions and enhancements. For example, due to issues with longevity of the Arduino EEPROM memory, the ++savecfg command has been implemented differently to save EEPROM wear. Some commands have been enhanced with additional options and a number of new custom commands have been added to provide new features that are not found in the standard Prologix implementation. Details of all commands and features can be found in the Command Reference section of the AR488 manual:
https://github.com/Twilight-Logic/AR488/blob/master/AR488-manual.pdf

Once uploaded, the firmware should respond to the ++ver command with its version information.

<b><i>Wireless Communication:</i></b>

The AR488 interface can communicate over Bluetooth using a Bluetooth HC05 or HC06 module. The firmware sketch supports auto-configuration of the Bluetooth HC05 module, the details of which can be found in the <i>AR488 Bluetooth Support</i> supplement. Although it can still be used to provide Bluetooth communication for the AR488, automatic configuration is not possible with a HC06 module so a HC06 module will have to be configured manually.

<b><i>Obtaining support:</i></b>

In the event that a problem is found, this can be logged via the Issues feature on the AR488 GitHub page. Please provide at minimum:

- the firmware version number
- the type of board being used
- the make and model of instrument you are trying to control
- a description of the issue including;
- what steps are required to reproduce the issue

Further work to add WiFi and support for additional boards is in progress.

Comments and feedback can be provided here:
https://www.eevblog.com/forum/projects/ar488-arduino-based-gpib-adapter/

<b><i>Acknowledgements:</i></b>
<table>
<tr><td>Emanuelle Girlando</td><td>Original project for the Arduino Uno</td></tr>
<tr><td>Luke Mester</td><td>Testing of original Uno/Nano verions against Prologix</td></tr>
<tr><td>Artag</td><td>Porting to the Arduino Micro (32u4) board</td></tr>
 </table>

Also, thank you to all the contributors to the AR488 EEVblog thread for their suggestions and support.

The original work by Emanuele Girlando is found here:
http://egirland.blogspot.com/2014/03/arduino-uno-as-usb-to-gpib-controller.html

