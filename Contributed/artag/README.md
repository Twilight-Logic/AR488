Contributor: artag
Source: EEVblog
Files: 32u4 port of the AR488 adapter

This version that contains both Mega and Uno versions. It breaks the customised bits out into separate included files. The SerialEvent call isn't supported on USB  but otherwise just an include file. The use of the 32u4 should make it possible to support the RTS/CTS pins correctly (although that isn't supported yet) and maybe at some point USBTMC could be added as an alternative to serial.

Tested and working with the Arduino Pro Micro, which has a built-in USB port.
