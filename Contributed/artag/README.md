<table>
  <tr><th>Contributor:</th><td>artag</td></tr>
  <tr><th>Source:</th><td>EEVblog member</td></tr>
  <tr><th>Description:</th><td>32u4 port of the AR488 GPIB-USB adapter</td></tr>
  <tr><th>Version:</th><td>AR488-artag 190913a</td></tr>
</table>

This version contains both Mega and Uno support, as well as introducing support for the Arduino Pro Micro (32u4). It breaks out the customised hardware code into separate included files. The use of the 32u4 should make it possible to support the RTS/CTS pins correctly (although that isn't supported yet) and maybe at some point USBTMC could be added as an alternative to serial.

Tested and working with the Arduino Pro Micro, which has a built-in USB port.
