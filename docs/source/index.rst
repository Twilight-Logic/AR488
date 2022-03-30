.. AR488 documentation master file, created by
   sphinx-quickstart on Wed Mar 30 12:12:21 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

AR488 GPIB Controller
=================================

The AR488 GPIB controller is an Arduino-based controller for interfacing with `IEEE488
GPIB <https://en.wikipedia.org/wiki/IEEE-488>`_ devices. The code has been tested on
`Arduino <https://arduino.cc>`_ `Uno
<https://store.arduino.cc/products/arduino-uno-rev3/>`_, `Nano
<https://store.arduino.cc/products/arduino-nano>`_, `Mega 2560
<https://store.arduino.cc/products/arduino-mega-2560-rev3>`_ and `Micro 32U4
<https://store.arduino.cc/products/arduino-micro>`_ boards and provides a low cost
alternative to other commercial interfaces.

To build an interface, at least one of the aforementioned Arduino boards will be
required to act as the interface hardware. Connecting to an instrument will require a 16
core cable and a suitable `IEEE488 connector
<https://en.wikipedia.org/wiki/IEEE-488#Connectors>`_. This can be salvaged from an old
GPIB cable or purchased from electronics parts suppliers. Alternatively, a PCB board can
be designed to incorporate a directly mounted IEEE488 connector.

The interface firmware can optionally support the `SN75160
<https://www.ti.com/product/SN75160B>`_ and `SN75161
<https://www.ti.com/product/SN75161B>`_ GPIB transceiver integrated circuits. Details of
construction and the mapping of Arduino pins to GPIB control signals and the data bus
are explained in the Building an AR488 GPIB Interface section.

The interface firmware supports standard Prologix commands (with the exception of
``++help``) and adheres closely to the Prologix syntax but there are some minor
differences. In particular, due to issues with the longevity of the Arduino EEPROM
memory, the ``++savecfg`` command has been implemented differently. Details of all
commands can be found in the Command Reference section.


Uploading updates to your MCU board
-----------------------------------
The firmware is upgradeable by compiling the sketch sources and using the Sketch->Upload
option found in the Arduino IDE. An AVR programmer can also be used to upload the firmware
to an Arduino microcontroller. In this case, the Sketch->Upload Using programmer option
should be used. The programmer device needs to be selected using the Tools->Programmer
menu.

Updates are available from https://github.com/Twilight-Logic/AR488


Client Software
---------------

The interface can be accessed via a number of software client programs:

- Terminal software (e.g. PuTTY)
- `EZGPIB <http://www.ulrich-bangert.de/html/downloads.html>`_
- `KE5FX GPIB Configurator <http://www.ke5fx.com/>`_
- `Luke Mester’s HP3478 Control <https://mesterhome.com/gpibsw/hp3478a/>`_
- Python scripts
- Anything else that can use the Prologix syntax!

Terminal clients connect via a virtual COM port and should be set to 115200 baud, no
parity, 8 data bits and 1 stop bit when connecting to the interface. On Linux, the port
will be a TTY device such as ``/dev/ttyUSB0`` or ``/dev/ttyACM0``.

Specific considerations apply when using an Arduino based interface with EZGPIB and the
KE5FX toolkit. These are described in the `Working with EZGPIB and KE5FX` section.

Operating Modes
---------------

The interface can operate in both controller and device modes.

Controller mode
+++++++++++++++

In this mode the interface can control and read data from various instruments including
Digital multimeters (DMMs). oscilloscopes, signal generators and spectrum analyzers.
When powered on, the controller and sends out an IFC (Interface Clear) to the GPIB bus
to indicate that it is now the Controller-in-Charge (CIC).

All commands are preceded with the ``++`` sequence and terminated with a carriage return
(CR), newline [a.k.a. linefeed] (LF) or both (CRLF). Commands are sent to or affect the
currently addressed instrument which can be specified with the ``++addr`` command (see
command ``help`` for more information).

By default, the controller is at GPIB address 0.

As with the Prologix interface, the controller has an auto mode that allows data to be
read from the instrument without having to repeatedly issue ``++read`` commands. After
``++auto 1`` is issued, the controller will continue to perform reading of measurements
automatically after the next ``++read`` command is used and using the parameters that
were specified when issuing that command.

Device mode
+++++++++++

The interface supports device mode allowing it to be used to send data to GPIB devices
such as plotters via a serial USB connection. All device mode commands are supported.


Transmission of data
--------------------

Interrupting transmission of data
+++++++++++++++++++++++++++++++++

While reading of data for the GPIB bus is in progress, the interface will still respond
to the ``++`` sequence that indicates a command. For example, under certain conditions
when the instrument is addressed to talk (e.g. when ``eos`` is set to 3 [no terminator
character] and the expected termination character is not received from the instrument,
or read with eoi and the instrument is not configured to assert ``eoi``, or ``auto
mode`` is enabled), data transmission may continue indefinitely. The interface will
still respond to the ``++`` sequence followed by a command (e.g. ``++auto 0`` or
``++rst``). Data transmission can be stopped and the configuration can then be adjusted.


Sending Data and Special characters
+++++++++++++++++++++++++++++++++++

Carriage return (``CR``, ``0x0D``, ``13d``), newline (a.k.a. linefeed) (``LF``,
``0x0A``, ``10d``), escape (``0x1B``, ``27d``) and ``+`` (``0x2B``, ``43d``) are special
control characters.

Carriage return and newline terminate command strings and direct instrument commands,
whereas a sequence of two ``+`` precedes a command token. Special care needs to be taken
when sending binary data to an instrument, because in this case we do not want control
characters to prompt some kind of action. Rather, they need to be treated as ordinary
and added to the data that is to be transmitted.

When sending binary data, the above mentioned characters must be escaped by preceding
them with a single escape (``0x1B``, ``27d``) byte. For example, consider sending the
following binary data sequence::

  54 45 1B 53 2B 0D 54 46

It would be necessary to escape the 3 control characters and send the following::

  54 45 **1B** 1B 53 **1B** 2B **1B** 0D 54 46

Without these additional escape character bytes, the special control characters present
in the sequence will be interpreted as actions and an incomplete or incorrect data
sequence will be sent.

It is also necessary to prevent the interface from terminating the binary data sequence
with a carriage return and newline (``0D 0A``) as this will confuse most instruments.
The command ``++eos 3`` can be used to turn off termination characters. The command
``++eos 0`` will restore default operation. See the command help that follows for more
details.


Receiving data
++++++++++++++

Binary data received from an instrument is transmitted over GPIB and then via serial
over USB to the host computer PC unmodified. Since binary data from instruments is not
usually terminated by CR or LF characters (as is usual with ASCII data), the EOI signal
can be used to indicate the end of the data transmission. Detection of the EOI signal
while reading data can be accomplished with the ``++read eoi`` command, while an
optional character can be added as a delimiter with the ``++eot_enable`` command (see
the command help that follows). The instrument must be configured to send the EOI
signal. For further information on enabling the sending of EOI see your instrument
manual.

Listen-only and talk-only (lon and ton) modes
+++++++++++++++++++++++++++++++++++++++++++++

In device mode, the interface supports both “listen-only” and “talk-only” modes (for
more details see the ``++lon`` and ``++ton`` commands. These modes are not addressed
modes and do not require a GPIB address to be set. Therefore if any GPIB address is
already set, it is simply ignored. Moreover, when in either of these modes, devices are
not controlled by the CIC. Data characters are sent using standard GPIB handshaking ,
but GPIB commands are ignored. The bus acts as a simple one to many transmission medium.
In lon mode, the device will receive any data placed on the bus by any talker, including
any other addressed device or controller. Since only ONE talker can exist on the bus at
a time, there can only be one device in "talk-only" mode on the bus, however multiple
"listen-only" devices can be present and all will receive the data sent by the talker.

Wireless communication
----------------------

The AR488 interface can communicate using a Bluetooth module (HC05 or HC06). The
firmware sketch supports auto-configuration of the Bluetooth HC05 module, the details of
which can be found in the Configuration section and the AR Bluetooth Support supplement.
Automatic configuration is not possible with a HC06 module so although this can be used
to provide Bluetooth communication, it has to be configured manually.


.. toctree::
   :maxdepth: 3
   :caption: Contents:

   main/configuration
   main/commands
   main/building
   main/tools
   main/appendixes



Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
