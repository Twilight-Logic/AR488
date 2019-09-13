<table>
  <tr><th>Contributor:</th><td>artag</td></tr>
  <tr><th>Source:</th><td>EEVblog member</td></tr>
  <tr><th>Description:</th><td>32u4 port of the AR488 GPIB-USB adapter</td></tr>
</table>

This version contains both Mega and Uno support. It breaks the customised bits out into separate included files. The use of the 32u4 should make it possible to support the RTS/CTS pins correctly (although that isn't supported yet) and maybe at some point USBTMC could be added as an alternative to serial.

Tested and working with the Arduino Pro Micro, which has a built-in USB port.
