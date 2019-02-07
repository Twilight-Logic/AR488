# AR488
AR488 Arduino GPIB Interface

The AR488 GPIB controller is an Arduino-based controller for interfacing with IEEE488 GPIB devices. The sketch has been tested on Arduino UNO and NANO boards and provides a low cost alternative to other commercial interfaces.

To build an interface, at least one Arduino board will be required to act as the interface hardware. Connecting to an instrument will require a 16 core cable and a suitable IEEE488 connector. This can be salvaged from an old GPIB cable or purchased from various electronics parts suppliers.

Details of construction and the mapping of Arduino pins to GPIB control signals and data bus are explained in the Building an AR488 GPIB Interface section of the AR488 manual.
 
The interface supports all of the standard Prologix commands but currently with the exception of the ++lon command. The commands adhere closely to the Prologix syntax but there are some differences. In particular, due to issues with the longevity of the Arduino EEPROM memory, the ++savecfg command has been implemented differently. Some commands have been enhanced and there are a number of custome commands. Details of all commands can be found in the Command Reference section of the AR488 manual.

The sketch AR488.ino should be uploaded to the the completed interface board. Once uploaded, the firmware should respond to the ++ver command with its version information.

In the event that a problem is found, this can be logged via the Issues feature on the AR488 GitHub page. Please provide at minimum:

- the firmware version number
- the type of board being used
- the make and model of instrument you are trying to control
- a description of the issue including;
- what steps are required to reproduce the issue

Further work is in progress.

