.. _Command Reference:

Command Reference
=================

The controller implements the standard commands prefixed with two plus (``++``)
character sequence to indicate that the following sequence is an interface command.
Commands, with the exception of the ``++savecfg`` command, should be fully compatible
with the Prologix GPIB-USB controller. However, the interface also implements a number
of additional custom commands.

Prologix-compatible commands
----------------------------

``++addr``
++++++++++

This is used to set or query the GPIB address. At present, only primary addresses are
supported. In controller mode, the address refers to the GPIB address of the instrument
that the operator desires to communicate with. The address of the controller is 0. In
device mode, the address represents the address of the interface which is now acting as
a device.

When issued without a parameter, the command will return the current GPIB address.

:Modes: controller, device

:Syntax: ``++addr [1-29]``
		 where 1-29 is a decimal number representing the primary GPIB
		 address of the device.

``++auto``
++++++++++

Configure the instrument to automatically send data back to the controller. When auto is
enabled, the user does not have to issue ``++read`` commands repeatedly. This command
has additional options when compared with the Prologix version.

When set to zero, auto is disabled.

When set to 1, auto is designed to emulate the Prologix setting. The controller will
automatically attempt to read a response from the instrument after any instrument
command or, in fact, any character sequence that is not a controller command beginning
with ``++``, has been sent.

When set to 2, auto is set to “on-query” mode. The controller will automatically attempt
to read the response from the instrument after a character sequence that is not a
controller command beginning with ``++`` is sent to the instrument, but only if that
sequence ends in a ``?`` character, i.e. it is a query command such as ``*IDN?``.

When set to 3, auto is set to “continuous” mode. The controller will execute continuous
read operations after the first ``++read`` command is issued, returning a continuous
stream of data from the instrument. The command can be terminated by turning off auto
with ``++auto 0`` or performing a reset with ``++rst``.

:Modes: controller
:Syntax: ``++auto [0|1|2|3]``
		 where 0 disables and 1 enables automatically sending data to the controller

Note: Some instruments generate a “Query unterminated or “-420” error if they are
addressed after sending an instrument command that does not generate a response. This
simply means that the instrument has no information to send and this error may be
ignored. Alternatively, auto can be turned off (``++auto 0``) and a ``++read`` command
issued following the instrument command to read the instrument response.


``++clr``
+++++++++

This command sends a `Selected Device Clear` (``SDC``) to the currently addressed
instrument. Details of how the instrument should respond may be found in the instrument
manual.

:Modes: controller
:Syntax: ``++clr``

``++eoi``
+++++++++

This command enables or disables the assertion of the ``EOI`` signal. When a data
message is sent in binary format, the ``CR/LF`` terminators cannot be differentiated
from the binary data bytes. In this circumstance, the ``EOI`` signal can be used as a
message terminator. When ``ATN`` is not asserted and ``EOI`` is enabled, the ``EOI``
signal will be briefly asserted to indicate the last character sent in a multi- byte
sequence. Some instruments require their command strings to be terminated with an EOI
signal in order to properly detect the command.

The ``EOI`` line is also used in conjunction with ``ATN`` to initiate a parallel poll,
however, this command has no bearing on that activity.

When issued without a parameter, the command will return the current configuration.

:Modes: controller, device
:Syntax: ``++eoi [0|1]``
		 where 0 disables and 1 enables asserting EOI to signal the last character sent

``++eos``
+++++++++

Specifies the GPIB termination character. When data from the host (e.g. a command
sequence) is received over USB, all non-escaped ``LF``, ``CR`` or ``Esc`` characters are
removed and replaced by the GPIB termination character, which is appended to the data
sent to the instrument. This command does not affect data being received from the
instrument.

When issued without a parameter, the command will return the current configuration.

:Modes: controller, device
:Syntax: ``++eos [0|1|2|3]``
		 where 0=CR+LF, 1=CR, 2=LF, 3=none

``++eot_enable``
++++++++++++++++

This command enables or disables the appending of a user specified character to the USB
output from the interface to the host whenever ``EOI`` is detected while reading data
from the GPIB port. The character to send is specified using the ``++eot_char`` command.

When issued without a parameter, the command will return the current configuration.

:Modes: controller, device
:Syntax: ``++eot_enable [0|1]``
		 where 0 disables and 1 enables sending the ``EOT`` character to the USB output.

``++eot_char``
++++++++++++++

This command specifies the character to be appended to the USB output from the interface
to the host whenever an ``EOI`` signal is detected while reading data from the GPIB bus. The
character is a decimal ASCII character value that is less than 256.

When issued without a parameter, the command will return a decimal number corresponding
to the ASCII character code of the current character.

:Modes: controller, device
:Syntax: ``++eot_char [<char>]``
		 where <char> is a decimal number that is less than 256.

``++help``
++++++++++

Not currently supported.

``++ifc``
+++++++++

Assert the GPIB ``IFC`` signal for 150 microseconds, making the AR488 the
Controller-in-Charge on the GPIB bus.

:Modes: controller
:Syntax: ++ifc

``++llo``
+++++++++

Disable front panel operation on the currently addressed instrument. In the original
HPIB specification, sending the ``LLO`` signal to the GPIB bus would lock the ``LOCAL``
control on ALL instruments on the bus. In the Prologix specification, this command
disables front panel operation of the addressed instrument only, in effect taking
control of that instrument. The AR488 follows the Prologix specification, but adds a
parameter to allow the simultaneous assertion of remote control over all instruments on
the GPIB bus as per the HPIB specification.

This command requires the Remote Enable (``REN``) line to be asserted otherwise it will
be ignored. In controller mode, the ``REN`` signal is asserted by default unless its
status is changed by the ``++ren`` command.

When the ``++llo`` command is issued without a parameter, it behaves the same as it does
on the Prologix controller. The ``LLO`` signal is sent to the currently addressed
instrument and this locks out the ``LOCAL`` key on the instrument control panel. Because
the instrument has been addressed and ``REN`` is already asserted, the command
automatically takes remote control of the instrument. Most instruments will display
``REM`` on their display or control panel to indicate that remote control is active and
front/rear panel controls will be disabled.

If the ``++llo`` command is issued with the ``all`` parameter, this will send the
``LLO`` signal to the bus, putting every instrument into remote control mode
simultaneously. At this point, instruments will not yet show the ``REM`` indicator and
it may still be possible to operate the front panel controls. On some instruments the
``LOCAL`` key may be locked out. However, as soon as an instrument has been addressed
and sent a command (assuming that a ``LOC`` signal has not been sent yet first), the
controller will automatically lock in remote control of that instrument , the ``REM``
indicator will be displayed and front/rear panel controls will be disabled.

:Modes: controller
:Syntax: ``++llo [all]``

``++loc``
+++++++++

Relinquish remote control and re-enable front panel operation of the currently addressed
instrument. This command relinquishes remote control of the instrument by de-asserting
``REN`` and sending the ``GTL`` signal.

The Remote Enable (``REN``) line must be asserted and the instrument must already be
under remote control otherwise the command has no effect.

In the original HPIB specification, this command would place all instuments back into
local mode, re-enabling the ``LOCAL`` key and panel controls on ALL instruments
currently connected to the GPIB bus. In the Prologix specification, this command
relinquishes remote control of the currently addressed instrument only. The AR488
follows the Prologix specification, but adds a parameter to allow the simultaneous
release of remote control over all instruments currently addressed as listeners on the
GPIB bus as per the HPIB specification.

If the command is issued without a parameter, it will re-enable the ``LOCAL`` key on the
control panel on the currently addressed instrument and relinquish remote control of the
instrument. If issued with the ``all`` parameter, it puts all devices on the GPIB bus in
local control state. The ``REM`` indicator should no longer be visible when the
instrument has returned to local control state.

:Modes: controller
:Syntax: ``++loc [all]``

``++lon``
+++++++++

The ``++lon`` command configures the controller to listen only to traffic on the GPIB
bus. In this mode the interface does require to have a GPIB address assigned so the
assigned GPIB address is ignored. Traffic is received irrespective of the currently set
GPIB address. The interface can receive but not send, so effectively becomes a
“listen-only” device. When issued without a parameter, the command returns the current
state of ``lon`` mode.

:Modes: device
:Syntax: ``++lon [0|1]``
		 where 0=disabled; 1=enabled

``++mode``
++++++++++

This command configures the AR488 to serve as a controller or a device.

In controller mode the AR488 acts as the Controller-in-Charge (CIC) on the GPIB bus,
receiving commands terminated with ``CRLF`` over USB and sending them to the currently
addressed instrument via the GPIB bus. The controller then passes the received data back
over USB to the host.

In device mode, the AR488 can act as another device on the GPIB bus. In this mode, the
AR488 can act as a GPIB talker or listener and expects to receive commands from another
controller (``CIC``). All data received by the AR488 is passed to the host via USB
without buffering. All data from the host via USB is buffered until the AR488 is
addressed by the controller to talk. At this point the AR488 sends the buffered data to
the controller. Since the memory on the controller is limited, the AR488 can buffer only
120 characters at a time.

When sending data followed by a command, the buffer must first be read by the controller
before a subsequent command can be accepted, otherwise the command will be treated as
characters to be appended to the existing data in the buffer. Once the buffer has been
read, it is automatically cleared and the parser can then detect the ``++`` command
prefix on the next line. Therefore sufficient delay must be allowed for the buffer to be
read before sending a subsequent command.

If the command is issued without a parameter, the current mode is returned.

:Modes: controller, device
:Syntax: ``++mode [0|1]``
		 where 0=device, 1=controller

``++read``
++++++++++

This command can be used to read data from the currently addressed instrument. Data is
read until:

- the ``EOI`` signal is detected
- a specified character is read
- timeout expires

Timeout is set using the read_tmo_ms command and is the maximum permitted delay for a
single character to be read. It is not related to the time taken to read all of the
data. For details see the description of the ``read_tmo_ms`` command.

:Modes: controller
:Syntax: ``++read [eoi|<char>]``
		 where <char> is a decimal number corresponding to the ASCII character to be used
		 as a terminator and must be less than 256.

``++read_tmo_ms``
+++++++++++++++++

This specifies the timeout value, in milliseconds, that is used by the ``++read`` (and
``++spoll``) commands to wait for a character to be transmitted while reading data from
the GPIB bus. The timeout value may be set between 0 and 32,000 milliseconds (32
seconds).

:Modes: controller
:Syntax: ``++read_tmo_ms <time>``
		 where <time> is a decimal number between 0 and 32000 representing milliseconds.

``++rst``
+++++++++

Perform a reset of the controller.

Please note that the reset may fail and hang the board under certain circumstances.
These include:

- the board has an older bootloader. The older bootloader had an problem with not
  clearing the ``MCUSR`` register which triggers another reset while the bootloader is
  being executed, which causes a perpetual restart cycle. The solution here is to update
  the bootloader. The newer Optiboot bootloader does not have this problem.

- using a 32u4 board (Micro, Leonardo) programmed with an AVR programmer with no
  bootloader. There is at present no solution to this problem. When programming with an
  AVR programmer, use a recent IDE version to export the binaries and upload the version
  with the bootloader to the board.

:Modes: controller, device
:Syntax: ``++rst``

``++savecfg``
+++++++++++++

This command saves the current interface configuration. On the Prologix interface
setting this to 1 would enable the saving of specific parameters whenever they are
changed, including ``addr``, ``auto``, ``eoi``, ``eos``, ``eot_enable``, ``eot_char``,
``mode`` and ``read_tmo_ms``.

Frequent updates wear out the EEPROM and the Arduino EEPROM has a nominal lifetime of
100,000 writes. In order to minimize writes and preserve the longevity of the EEPROM
memory, the AR488 does not, at any time, write configuration parameters “on the fly”
every time they are changed. Instead, issuing the ``++savecfg`` command will update the
complete current configuration once. Only values that have changed since the last write
will be written.

The configuration written to EEPROM will be automatically re-loaded on power-up. The
configuration can be reset to default using the ++default command and a new
configuration can be saved using the ``++savecfg`` command.

Most, if not all Arduino AVR boards support EEPROM memory, however boards from other
vendors may not provide this support. If the command is run on a board that does not
support EEPROM, then the following will be returned: EEPROM not supported.

The ``++savecfg`` command will save the following current parameter values: ``addr``,
``auto``, ``eoi``, ``eos``, ``eot_enable``, ``eot_char``, ``mode``, ``read_tmo_ms`` and
``verstr``.

:Modes: controller, device
:Syntax: ``++savecfg``


``++spoll``
+++++++++++

Performs a serial poll. If no parameters are specified, the command will perform a
serial poll of the currently addressed instrument. If a GPIB address is specified, then
a serial poll of the instrument at the specified address is performed. The command
returns a single 8-bit decimal number representing the status byte of the instrument.

The command can also be used to serial poll multiple instruments. Up to 15 addresses can
be specified. If the all parameter is specified (or the command ``++allspoll`` is used),
then a serial poll of all 30 primary instrument addresses is performed.

When polling multiple addresses, the ``++spoll`` command will return the address and
status byte of the first instrument it encounters that has the ``RQS`` bit set in its
status byte, indicating that it has requested service. The format of the response is
``SRQ:addr,status``, for example: ``SRQ:3,88`` where 3 is the GPIB address of the
instrument and 88 is the status byte. The response provides a means to poll a number of
instruments and to identify which instrument raised the service request, all in one
command. If ``SRQ`` was not asserted then no response will be returned.

When ``++srqauto`` is set to 1 (for details see the ``++srqauto`` custom command), the
interface will automatically conduct a serial poll of all devices on the GPIB bus
whenever it detects that ``SRQ`` has been asserted and the details of the instrument
that raised the request are automatically returned in the format above.

:Modes: controller
:Syntax: ``++spoll [<PAD>|all|<PAD1> <PAD2> <PAD3>...]``
		 where ``<PAD>`` and ``<PADx>`` are primary GPIB address and all specifies
		 that all instruments should be polled.

``++srq``
+++++++++

This command returns the present status of the ``SRQ`` signal line. It returns 0 if
``SRQ`` is not asserted and 1 if ``SRQ`` is asserted.

:Modes: controller
:Syntax: ``++srq``

``++status``
++++++++++++

Set or display the status byte that will be sent in response to the serial poll command.
When bit 6 of the status byte is set, the ``SRQ`` signal will be asserted indicating
Request For Service (``RQS``). The table below shows the values assigned to individual
bits as well as some example meanings that can be associated with them. Although the
meaning of each bit will vary depending on the instrument and the manufacturer, bit 6 is
always reserved as the ``RQS`` bit. Other bits can be assigned as required.

+---+--------+----+------------+-------+-------+---------+-----------+------------+
|Bit| 7      | 6  | 5          | 4     | 3     | 2       | 1         | 0          |
+===+========+====+============+=======+=======+=========+===========+============+
|Msg|Always  |RQS |Calibration |Output |Remote |Auto-zero|Auto-range |Operational |
|   |0       |    |enabled or  |Avail. |control|         |enabled.   |error       |
|   |        |    |Error       |Front/ |       |         |Front/     |            |
|   |        |    |            |Rear   |       |         |Rear sw.   |            |
+---+--------+----+------------+-------+-------+---------+-----------+------------+

The values of the bits to be set can be added together to arrive at the desired status
byte value. For example, to assert ``SRQ``, a value of ``0x40`` (64) would be
sufficient. However if we wanted to use bit 1 to indicate an operational error, then a
value of ``0x41`` (65) might be used in the event of the error occurring.

:Modes: device
:Syntax: ``++status [byte]``
		 where byte is a decimal number between 0 and 255.

``++trg``
+++++++++

Sends a ``Group Execute Trigger`` to selected devices. Up to 15 addresses may be
specified and must be separated by spaces. If no address is specified, then the command
is sent to the currently addressed instrument. The instrument needs to be set to single
trigger mode and remotely controlled by the GPIB controller. Using ``++trg``, the
instrument can be manually triggered and the result read with ``++read``.

:Modes: controller
:Syntax: ``++trg [pad1 … pad15]``


``++ver``
+++++++++

Display the controller firmware version. If the version string has been changed with
``++setvstr``, then ``++ver`` will display the new version string. Issuing the command
with the parameter ``real`` will always display the original AR488 version string.

:Modes: controller, device
:Syntax: ``++ver [real]``

.. _Custom commands:

Custom commands
---------------

``++allspoll``
++++++++++++++

Alias equivalent to ``++spoll all``. See ``++spoll`` for further details.

``++dcl``
+++++++++

Send Device Clear (``DCL``) to all devices on the GPIB bus.

:Modes: controller
:Syntax: ``++dcl``

``++default``
+++++++++++++


This command resets the AR488 to its default configuration.

When powered up, the interface will start with default settings in controller mode.
However, if the configuration has been saved to EEPROM using the savecfg command, the
controller will start with the previously saved settings. This command can be used to
reset the controller back to its default configuration.

The interface is set to controller mode with the following parameters:

:auto: 0
:eoi: 0 (disabled)
:eor: 0 (CR+LF)
:eos: 0 (CR+LF)
:eot_enable: 0 (disabled)
:eot_char: 0
:GPIB: address - controller 0
:GPIB: address - primary 1
:GPIB: address - secondary 0
:mode: controller
:read_tmo_ms: 1200
:status: byte 0
:version: string default version string

.. NOTE::
   Unless the ``++savecfg`` command is used to overwrite the previously saved
   configuration, the previous configuration will be re-loaded from non-volatile memory
   the next time that the interface is powered up. To ensure that settings are saved,
   after using the ``++default`` command, configure the interface as required and then use
   ``++savecfg`` to save the settings to EEPROM*. The interface can be returned to its
   default state by using ``++default`` followed by ``++savecfg`` without making any further
   configuration changes.

   * this assumes that the board being used supports saving to EEPROM.

:Modes: controller, device
:Syntax: ``++default``

``++eor``
+++++++++

End of receive. While ``++eos`` (end of send) selects the terminator to add to commands
and data being sent to the instrument, the ``++eor`` command selects the expected
termination sequence when receiving data from the instrument.

The following termination sequences are supported::

  Option   Sequence        Hex
  0        CR + LF         0D 0A
  1        CR              0D
  2        LF              0A
  3        None            N/A
  4        LF + CR         0A 0D
  5        ETX             03
  6        CR + LF + ETX   0D 0A 03
  7        EOI signal      N/A

The default termination sequence is ``CR`` + ``LF``. If the command is specified with
one of the above numeric options, then the corresponding termination sequence will be
used to detect the end of the data being transmitted from the instrument. If the command
is specified without a parameter, then it will return the current setting. If option 7
(``EOI``) is selected, then ``++read eoi`` is implied for all ``++read`` instructions as
well as any data being retuned by the instrument in response to direct instrument
commands. An EOI is expected to be signalled by the instrument with the last character
of any transmission sent. All characters sent over the GPIB bus are passed to the serial
port for onward transmission to the host computer.

:Modes: controller
:Syntax: ``++eor[0-9]``

``++id``
++++++++

This command sets the identification parameters for the interface. Here you can set the
instrument name and optional serial number. This command also sets the information that
can be used by the interface to respond to a SCPI ``*idn?`` which may be useful where
the instrument itself cannot provide such a response. For further information also see
the ``++idn`` command. The command has one of three invocations and an optional
parameter:

``++id name``

This sets a short name for the interface. The name can be up to 15 characters long and
should not include spaces. If the command is specified without a parameter, it will
return the current name of the interface. By default, the name is not set and the
command will not return a value.

``++id serial``

This sets an optional serial number for the interface. In the event that there are
multiple instances of identical instruments on the bus, each instrument can be given a
unique serial number up to 9 digits long. When specified without a parameter, the
command returns the currently configured serial number. By default the serial number is
not set and the command will return ``000000000``.

``++id verstr``

Sets the version string that the controller responds with on boot-up and in response to
the ``++ver`` command. This may be helpful where software on the computer is expecting a
specific string from a known controller, for example ‘GPIB-USB’. When no parameter is
given, the command returns the current version string.

Examples::

  ++id name HP3478A
  ++id serial 347800001
  ++id verstr GPIB-USB
  ++id verstr

:Modes: controller
:Syntax: ``++id name [name]``
		 ``++id serial [serialnum]``
		 ``++id verstr [version string]``


``++idn``
+++++++++

This command is used to enable the facility for the interface to respond to a SCPI
``*idn?`` Command. Some older instruments do no respond to a SCPI ``ID`` request but
this feature will allow the interface to respond on behalf of the instrument using
parameters set with the ``++id`` command. When set to zero, response to the SCPI
``*idn?`` command is disabled and the request is passed to the instrument. When set to
1, the interface responds with the name set using the ``++idn`` name command. When set
to 2, the instrument also appends the serial number using the format ``name-99999999``.


:Modes: controller
:Syntax: ``++idn[0-2]``

``++macro``
+++++++++++

Instrument control usually requires a sequence of commands to be sent to the instrument
to set it up or to perform a particular task. Where such a sequence of commands is
performed regularly and repeatedly, it is beneficial to have a means to pre-program the
sequence and to be able to run it with a single command.

The AR488 allows up to 9 sequences to be programmed into the Arduino sketch that can be
run using the ``++macro`` command. When no parameters have been specified, the macro
command will return a list of numbers indicating which macros have been defined and are
available to use.

When called with a single number between 1 and 9 as a parameter, the command will run
the specified macro.

Programming macros is beyond the scope of this manual and will be specific to each
instrument or implemented programming language or protocol.

:Modes: controller
:Syntax: ``++macro [1-9]``


``++ppoll``
+++++++++++

When many devices are involved, Parallel Poll is faster than Serial Poll but is not
widely used. With a Parallel Poll, the controller can query up to eight devices quite
efficiently using the ``DIO`` lines. Since there are 8 ``DIO`` lines, up to 8 devices
can be queried at once. In order to get an unambiguous response, each device should
ideally assign to a separate data line. Devices assigned to the same line are simply
OR’ed. Devices respond to the parallel poll by asserting the DIO line they have been
assigned.

Response to a Parallel Poll is a data byte corresponding to the status of the ``DIO``
lines when the Parallel Poll request is raised. The state of each individual bit of the
8-bit byte corresponds to the state of each individual ``DIO`` line. In this way it is
possible to determine which instrument raised the request.

Because a single bit can only be 0 or 1, the response to a parallel poll is binary,
simply indicating whether or not an instrument has raised the request. In order to get
further status information, a Serial Poll needs to be conducted on the instrument in
question.

:Modes: controller
:Syntax: ``++ppoll``


``++ren``
+++++++++

In controller mode, this command turns the ``REN`` signal on and off. When ``REN`` is
asserted, the controller can remote-control any device on the BUS. With the ``REN``
signal turned off, the controller can no longer remote-control devices, but can still
communicate with them. This is used primarily for diagnostics.

When set to 0, ``REN`` is un-asserted. When set to 1, ``REN`` is asserted. By default,
in controller mode, ``REN`` will be asserted.

When ``REN`` is used to control the ``SN75161`` GPIB transceiver integrated-circuit,
this command is unavailable and will simply return Unavailable (see the
``Configuration`` and the ``Building an AR488 GPIB Interface`` sections for more
information). When issued without a parameter, the command returns the current status of
the ``REN`` signal.

:Modes: controller
:Syntax: ``++ren [0|1]``

``++repeat``
++++++++++++

Provides a way of repeating the same command multiple times, for example, to request
multiple measurements from the instrument.

Between 2 and 255 repetitions can be requested. It is also possible to request a delay
between 0 to 10,000 milliseconds (or 10 seconds) between each repetition. The parameter
buffer has a maximum capacity of 64 characters, so the command string plus any
parameters cannot exceed 64 characters in total. Once started, there is no mechanism to
stop the repeat loop once it has begun.

The command will run the number of iterations requested and stop only when the request
is complete.

:Modes: controller
:Syntax: ``++repeat count delay cmdstring``
		 where:
		 ``count`` is the number of repetitions from 2 to 255
		 ``delay`` is the time to wait between repetitions from 0 to 10,000 milliseconds
		 ``cmdstring`` is the command to execute

``++setvstr``
+++++++++++++

This command is DEPRECATED and will be removed in future versions. Please refer to the
notes for the ``++id verstr`` command instead. It sets the version string that the
controller responds with on boot-up and in response to the ``++ver`` command. This may
be helpful where software on the computer is expecting a specific string from a known
controller, for example "GPIB-USB".

The ``++ver`` command can be used to confirm that the string has been set correctly.

:Modes: controller, device
:Syntax: ``++verstr [string]``
		 where ``[string]`` is the new version string

``++srqauto``
+++++++++++++

When conducting a serial poll using a Prologix controller, the procedure requires that
the status of the ``SRQ`` signal be checked with the ``++srq`` command. If the response
is a 1, indicating that ``SRQ`` is asserted, then an ``++spoll`` command can be issued
to determine the status byte of the currently addressed instrument or optionally an
instrument at a specific GPIB address.

When polling multiple devices, the AR488 will provide a custom response that includes
the address and status byte of the first instrument encountered that has the ``RQS`` bit
set. Usually, the ``++spoll`` command has to be issued manually to obtain this
information.

When ``++srqauto`` is set to 0 (default), in order to obtain the status byte when
``SRQ`` is asserted, a serial poll has to be conducted manually using the ``++spoll``
command.

When ``++srqauto`` is set to 1, the interface will automatically detect when the ``SRQ``
signal has been asserted by an instrument and will automatically conduct a serial poll,
returning the address and status byte of the first instrument encountered that has the
``RQS`` bit set in its status byte. If multiple instruments have asserted ``SRQ``, then
another subsequent serial poll will be conducted to determine the next instrument that
has requested service. The process continues until all instruments that have requested
service have had their status byte read and the ``SRQ`` signal has been cleared.

Without parameters, this command returns the present status of the ``SRQauto``. It
returns 0 if a serial poll is not automatically executed (default) and 1 if a serial
poll is automatically executed.

:Modes: controller
:Syntax: ``++srqauto [0|1]``
		 where 0=disabled, 1=enabled

``++tmbus``
+++++++++++


The GPIB bus protocol is designed to allow the bus to synchronise to the speed of the
slowest device. However, under some circumstances it may be desirable to slow down the
bus. The ``tmbus`` parameter introduces a periodic delay of between 0 to 30,000
microseconds between certain operations on the bus and so slows down the operation of
the GPIB bus. The greater the delay, the slower the bus will operate. Under normal
running conditions this parameter should be set to zero, which is the default setting.

:Modes: controller, device
:Syntax: ``++tmbus [value]``
		 where [value] is between 0 and 30,000 microseconds


``++ton``
+++++++++

The ``++ton`` command configures the controller to send data only on the GPIB bus. When
in this mode, the interface does not require to have a GPIB address assigned and the
address that is set will be ignored. Data is placed on the GPIB bus as soon as it is
received via USB. Only one sender can exist on the bus, but multiple receivers can
listen to and accept the transmitted data. The interface can send, but not receive, so
effectively becomes a “talk-only” device. When issued without a parameter, the command
returns the current state of “ton” mode.

:Modes: device
:Syntax: ``++ton [0|1]``
		 where 0=disabled; 1=enabled

``++verbose``
+++++++++++++

Toggle verbose mode ON and OFF

:Modes: controller, device
:Syntax: ``++verbose``
