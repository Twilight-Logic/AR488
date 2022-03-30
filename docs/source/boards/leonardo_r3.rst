=====================
 Arduino Leonardo R3
=====================

The pinout on the `Leonardo R3
<https://store.arduino.cc/products/arduino-leonardo-with-headers>`_ is as follows:

======= ================= ========
Arduino GPIB connector    Function
======= ================= ========
A0      1                 DIO1
A1      2                 DIO2
A2      3                 DIO3
A3      4                 DIO4
D12     5                 EOI
D11     6                 DAV
D10     7                 NRFD
D9      8                 NDAC
D8      9                 IFC
D2      10                SRQ
D7      11                ATN
GND     12                Shield
A4      13                DIO5
A5      14                DIO6
D4      15                DIO7
D5      16                DIO8
D3      17                REN
GND     18,19,20,21,22,23 GND
======= ================= ========

The Leonardo R3 has a similar form factor to the Uno. It uses a 32u4 MCU rather than a
328P and has a micro USB port. Instead of a CH340 UART it uses USB CDC emulated serial
ports and has one separate hardware serial port available on on ``RX1`` and ``TX1``,
whereas the Uno shares these pins with USB. It requires no modification to work with
KE5FX tools. The board pin layout is the same as the Uno and the above pinout is
identical to the Uno.
