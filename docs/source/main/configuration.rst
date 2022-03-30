.. _Configuration:

Configuration
=============

Configuration of the AR488 is achieved be editing the `AR488_Config.h` file. This is a
C++ style header file containing various definition statements, also known as ‘define
macros’ , starting with keyword `#define`, that can be used to configure the firmware.
The `AR488_config.h` file must be included in the main AR488 sketch as well as any other
module header file (e.g. `AR488_Layouts.cpp` and `AR488_Layouts.h`) with an include
statement:

.. code-block:: c++

   #include "AR488_Config.h

A number of these definition statements are contained within an ``#ifdef .. #endif``
construct, some of which may contain additional #else or #elif elements. The presence of
these constructs is necessary and they should not be changed or removed. Only the
definitions within them should be changed as required. Nothing should need to be changed
in any other file.

Firmware version
----------------

This is in the format:

.. code-block:: c++

   #define FWVER "AR488 GPIB controller, ver. 0.48.08, 27/01/2020"

This entry should not exceed 47 characters and should not be changed.


Board Selection and serial port configuration
---------------------------------------------

The AR488 supports a number of Arduino AVR boards and also a custom GPIO pin layout
which can be defined by the user in the Custom Board Layout section. If a custom GPIO
pin layout is to be used, then following entry must have the comment characters
(preceding ``//``) removed:

.. code-block:: c++

   //#define AR488_CUSTOM

Otherwise, the comment characters should remain in place which has the effect of
disabling the definition by designating it as a comment. The compiler ignores comment
statements. Following this is an ``#ifdef`` statement containing several sections
preceded by an ``#elif`` keyword. Each of these is followed by a token that corresponds
to known Arduino definitions for microprocessor types. The structure looks like this:

.. code-block:: c++

   /*
   * Configure the appropriate board/layout section
   * below as required
   */
   #ifdef AR488_CUSTOM
   ...
   #elif __AVR_Atmega328P__
   ...
   #elif __AVR_Atmega32U4__
   ...
   #elif __AVR_Atmega2560__
   ...
   #endif // Board/layout selection

When the custom layout is selected, all other layouts are ignored. If the custom layout
is not selected, then the section corresponding to the automatically detected Arduino
microprocessor will apply. Each section contains a definition referencing one or more
pre-defined board layouts as well as serial port definitions corresponding to the
features of specific boards. For example here are the definitions for boards based on
the 328p microcontroller which are found within the ``__AVR_Atmega328P__`` section of
the ``#ifdef`` statement:

.. code-block:: c++

   /* Board/layout selection */
   #define AR488_UNO
   //#define AR488_NANO
   /*** Serial ports ***/
   //Select HardwareSerial or SoftwareSerial (default = HardwareSerial) ***/
   // The UNO/NANO default hardware port is 'Serial'
   // (Comment out #define AR_HW_SERIAL if using SoftwareSerial)
   #define AR_HW_SERIAL
   #ifdef AR_HW_SERIAL
   #define AR_SERIAL_PORT Serial
   #else
   // Select software serial port
   #define AR_SW_SERIAL
   #endif


The section contains definitions for two boards, namely the Uno and the Nano. Only ONE
of these should be selected by removing the preceding comment characters:

.. code-block:: c++

   #define AR488_UNO
   //#define AR488_NANO

The default entry is ``AR488_UNO``, which selects the pre-defined template for the
Arduino UNO board in ``AR488_Hardware.h``. Selecting ``AR488_NANO`` will select the
pre-defined template for the Nano board. In order to compile the sketch for the selected
board, in addition to selecting the template in ``Config.h``, the correct board must be
selected in the Board Manager within the Arduino IDE (see the ``Tools | Board:`` menu).

Following this are definitions for the serial port:

.. code-block:: c++

   #define AR_HW_SERIAL
   #ifdef AR_HW_SERIAL
     #define AR_SERIAL_PORT Serial
   #else
     // Select software serial port
     #define AR_SW_SERIAL
   #endif

By default, the most commonly used serial port for a particular board will be enabled.
In the example above, the hardware port named Serial is selected. To switch between the
default hardware port and a SoftwareSerial port, it is necessary only to comment out
``#define AR_HW_SERIAL`` by preceding the line with ``//``.

The section for the 32u4 (Micro/Leonardo) is similar, except by default
``AR_CDC_SERIAL`` is enabled and switching is between the ``USB CDC`` port and the
hardware port ``Serial1``.

The Mega 2560 has 4 hardware serial ports so either ``Serial``, ``Serial1``, ``Serial2``
or ``Serial3`` must be selected. Most likely the default port named Serial will be used
although other options are possible if required. However, please note that the default
GPIO pin layout for the Mega 2560 board (``AR488_MEGA2560_D``) uses the pins assigned to
``Serial2`` for other purposes, so this cannot be used as a serial port with that
particular layout definition. However, it can be used with the E1 and E2 definitions.

For any board, adding the line ``#define AR_SW_SERIAL`` and commenting out the
``AR_HW_SERIAL`` and/or ``AR_CDC_SERIAL`` definitions will invoke the `SoftwareSerial
<https://www.arduino.cc/en/Reference/SoftwareSerial>`_ library. When a SoftwareSerial_
port is required, then the GPIO pins as well as the baud rate to be used will need to be
configured in the following `Software Serial port configuration`_ section.

Where a board has more than one hardware or CDC serial port available, it will be
necessary to correctly specify the Arduino name of the serial port to be used. For the
Uno and Nano this will always be ``Serial`` because those boards have only one UART and
therefore only one hardware serial port. Other boards have more than one serial port
available. For example, on the Pro Micro , enabling the hardware port rather than the
CDC port will automatically select ``Serial1`` instead of ``Serial``. For the Mega 2560
there are four choices and the correct port name must be chosen by uncommenting the
appropriate line, e.g:

.. code-block:: c++

   #define AR_SERIAL_PORT Serial1

The remaining choices must be commented out by preceding them with ``//``.

It should be noted that for any board, only ONE serial port can be used and therefore
only one port should be enabled.

**It is important to make sure that the correct board is selected in the Arduino IDE
Boards Manager (Tools => Board) otherwise the sketch will not compile correctly.**

Software Serial port configuration
----------------------------------

The SoftwareSerial_ library can be used with any board provided that at least two pins
are available. One of these must be a ``PWM`` enabled GPIO pin which is required to
emulate the transmit (``Tx``) output for the serial two wire connection. The receive
(``Rx``) pin can be assigned any available GPIO pin.

Enabling SoftwareSerial can be done by removing the comment characters (``//``)
preceding the ``#define AR_SW_SERIAL`` entry in the relevant board selection section. In
addition, the pins to be used as well as the board rate will need to be configured in
the Software Serial Support section as follows:

.. code-block:: c++

   #ifdef AR_SW_SERIAL
   #define AR_SW_SERIAL_RX 53
   #define AR_SW_SERIAL_TX 51
   #define AR_SERIAL_BAUD 57600
   #else
   #define AR_SERIAL_BAUD 115200
   #endif

The appropriate GPIO pin numbers should be specified after the ``#define
AR_SW_SERIAL_RX`` and the ``#define AR_SW_SERIAL_TX`` statements within the ``#ifdef
AR_SW_SERIAL`` clause. The baud rate should be specified here as well after ``#define
AR_SERIAL_BAUD``. Please note that, when using SoftwareSerial, the maximum baud rate
that can be achieved reliably is 57600 baud.

The hardware/CDC serial port baud rate is specified after the ``#else`` statement. The
default hardware baud rate is 115200, but any valid baud rate can be specified.

Please note also, that when using USB CDC ports, the Arduino board will NOT be reset
when a serial connection is made over USB as is the case with Uno and Nano boards. The
reset button must be pressed in order to reset the board. The Arduino IDE seems to take
care of programming the board automatically but when using the Arduino IDE on Linux, the
modemmanager service will need to be disabled as it interferes with serial ports and
disrupts the normal operation of the programming process, causing boards to end up in a
state where they can no longer be programmed over USB. Boards that have been disabled in
this way can be recovered by uploading a bootloader to them using an AVR programmer.

Linux Mint (and probably Ubuntu) will have this service enabled and running in the
background by default. This service is not required but may be useful in the event that
a serial modem is connected to the PC.

Serial Interrupt Handling
-------------------------

All AVR boards support `serialEvent
<https://www.arduino.cc/reference/en/language/functions/communication/serial/serialevent/>`_.
This was used in previous versions of AR488 but its use is now deprecated. The Arduino
has no hardware interrupt to signal a character being received into the Arduino serial
buffer. The `serialEvent`_ function is actually aliased to the `serial.available()
<https://www.arduino.cc/reference/en/language/functions/communication/serial/available/>`_
function and is executed at the end of every iteration of ``void loop()``. Non-Arduino
boards (e.g. STM32) may not support `serialEvent`_ so for reasons of consistency between
different boards, `serialEvent`_ is not used. Instead, a serial event handler is called
at the end of every loop iteration.

When working with programs and scripts (e.g. Python), it should be bourne in mind that
the Arduino is only 64 bytes in size. Due to the memory constraints of the Arduino, the
additional processing buffer provided by the AR488 program is also limited to only 128
bytes. There is also no handshaking between the PC and the Arduino serial port. Although
the Arduino can keep up pretty well, the serial input buffer can easily overflow with
loss of characters if data is passed too quickly. This means that a bit of trial and
error may be required when working with scripts to establish whether and how much delay
is required between commands. A short delay may sometimes be needed to avoid a buffer
overflow. The amount of delay will depend on factors such as the interface hardware
being used, the time taken for the instrument to respond, as well as the GPIB speed of
the instrument being addressed.

Detection of SRQ and ATN pin states
-----------------------------------

Arduino AVR boards support interrupts to detect a change in pin states and this has been
implemented for the UNO, NANO and MEGA boards to improve response. When a supported
board template has been selected, (see the `Board Selection and serial port
configuration`_ section) and ``AR488_CUSTOM`` is not in use, then ``USE_INTERRUPTS``
will be defined and certain interrupts activated by default.

Other boards may not support interrupts and interrupts cannot be used with the custom
GPIO pin layout. When ``AR488_CUSTOM`` is defined and in used, ``USE_INTERRUPTS`` will
not get defined and interrupts are not activated. Instead, pin states are detected
during each iteration of the ``void loop()`` function. When a non-AVR or unsupported
board is selected as the compilation target, then ``USE_INTERRUPTS`` should be commented
out and not used.

The section in AR488_Config.h looks as follows:

.. code-block::

   #ifdef __AVR__
     // For supported boards use interrupt handlers
     #if defined (AR488_UNO) || defined (AR488_NANO) || defined (AR488_MEGA2560) || defined (AR488_MEGA32U4)
       #ifndef AR488_CUSTOM
         #define USE_INTERRUPTS
	   #endif
     #endif
   #endif


The entry should be preceded by ``//`` to indicate that it has been commented out.
Interrupts are used by default on supported boards because they usually respond faster
than in-loop checking.

Macro support
-------------

Macros in this context are short sequences of commands that can be used to accomplish a
particular task. Controlling an instrument usually requires sequences of commands to be
sent to the device to configure it, or to perform a particular task. Sometimes such
sequences are performed frequently or repetitively. In those circumstances, it may be
more efficient to pre- program the required sequence and then execute it when required
using a single command.

The AR488 supports a macro feature which allows user programmed command sequences to be
run when the interface starts up, as well as up to 9 user defined command sequences to be
executed at runtime.

Macros must be programmed before the sketch is compiled and uploaded to the interface.
Macros can be added to the designated ``AR488 MACROS SECTION`` in the ``AR488_Config.h``
file. Both interface ``++`` commands and direct instrument commands can be included in
macros. Programming specific instruments is beyond the scope of this manual as commands
will be specific to each instrument or implemented according to the manufacturers choice
of programming language or protocol. However, in general, in order to create macros, a
few simple rules will need to be followed.

Firstly, macros need to be enabled. In the ``AR488_Config.h`` file there are two
definitions under the heading "Enable Macros":

.. code-block::

   #define USE_MACROS   // Enable the macro feature
   #define RUN_STARTUP  // Run MACRO_0 (the startup macro)

The ``#define USE_MACROS`` construct enables or disables the macro feature. When this line
is commented out by preceding it with ``//`` then macros are disabled. Removing the
preceding ``//`` will enable the macro feature.

The ``#define RUN_STARTUP`` statement controls whether the start-up macro will run when
the interface is powered up or re-started. The start-up macro is designated ``MACRO_0``
and if ``#define RUN_STARTUP`` is enabled, this macro will run when the interface is
powered on or reset.

When ``#define USE_MACROS`` is disabled, then the start-up macro will not be activated
when the interface is powered up or reset and none of the user macros (1-9) will be
available at runtime.

When enabled, ``MACRO_0`` will run when the interface is powered up or reset but only if
``#define RUN_STARTUP`` is also enabled. The user macros (1-9) will always be available
and can be executed by the user at runtime by using the ``++macro`` command. For more
information please see the ``++macro`` command heading in the :ref:`Command Reference`.

The start-up macro can be used in addition to the interface settings that can be saved
using the ``++savecfg`` command, to not only to set up the interface, but also to
initialise and configure the instrument for a specific function. In this way, instrument
commands that select function, range and other control features can be sent
automatically as the interface starts up.

Unless steps have been taken to disable the automatic reset that occurs when a USB
serial connection is opened to the interface, the start-up macro will run every time
that a serial connection is initiated to the interface. On the other hand, disabling
reset prevents the Arduino from being programmed via USB, so is not advised unless the
intention is to program the Arduino using a suitable AVR programmer.

In the ``AR488_Config.h`` file, sketch, below the help information there is a section
that starts:

.. code-block:: c++

   /********************************/
   /***** AR488 MACROS SECTION *****/
   /***** vvvvvvvvvvvvvvvvvvvv *****/
   #ifdef USE_MACROS

Macros are defined here. The first macro is the startup macro, an example of which might
be defined as follows:

.. code-block:: c++

   #define MACRO_0 "\
   ++addr 9\n\
   ++auto 2\n\
   *RST\n\
   :func 'volt:ac'
   "
   /* End of MACRO_0 (Startup macro)*/

All macro commands comprising the macro must be placed after the ``\`` on the first line
and before the final quote on the line before the ending comment. Nothing outside of
these lines, including the quote marks and the ``\`` and after the macro name should be
modified. The final quote mark can be appended to the last command in the sequence if
preferred. It is shown here on a separate line for clarity. Everything between the two
quote marks is a string of characters and must be delimited. The ``\`` character
indicate to the pre-processor that the string continues on the next line.

Each command ends with ``\n`` which is the newline terminator and serves to delimit each
command. The actual sequence shown above is therefore comprised of 4 commands, each
command ending with ``\n`` and then a ``\`` to indicate that the next command is to
follow on the next line. Try to avoid leaving or including any unnecessary spaces.

Each of these commands is either a standard AR488 interface command found in the command
reference, or an instrument specific command. All AR488 interface Prologix style
commands begin with ``++`` so the first two commands set the GPIB ``address`` to 7 and
``auto`` to 1. The next two commands are direct instrument commands using the SCPI
protocol, the first of which resets the instrument and the second selects the instrument
AC voltage function.

As shown, each command must be terminated with a ``\n`` (newline) or ``\r`` (carriage
return) delimiter character.

User defined macros that can be run using the ``++macro`` command follow next, and have
a similar format, e.g:

.. code-block:: c++

   #define MACRO_2 "\
   "
   /*<-End of macro 2*/

Once again, the required command sequence must be placed between the two quotes and
after the first ``/`` and be terminated with a ``\n`` or ``\r`` delimiter. Each line
must be wrapped with ``\``.

There is a slightly shorter method of defining a macro by placing all commands on a single line. For
example this:

.. code-block:: c++

   #define MACRO_1 "++addr 7\n++auto 1\n*RST\n:func 'volt:ac'"

Is exactly the same as this:

.. code-block:: c++

   #define MACRO_1 "\
   ++addr 7\n\
   ++auto 1\n\
   *RST\n\
   :func 'volt:ac'\
   "

The first definition is more condensed and requires no line wrap characters, but it is
perhaps easier to see what is going on in the latter example. Either will function just
the same and take up the same amount of memory.

The macro definition area provided in the sketch ends with:

.. code-block:: c++

   #endif
   /***** ^^^^^^^^^^^^^^^^^^^^ *****/
   /***** AR488 MACROS SECTION *****/
   /********************************/

Anything outside of this section does not relate to macros.

Provided that the commands have been specified correctly and the syntax is correct, the
sketch should compile and can be uploaded to the Arduino. The start-up macro will run as
soon as the upload is completed so the instrument should respond immediately. Please be
aware that, unless serial reset has been disabled, it will run again when a USB serial
connection is made to the interface. The instrument will probably respond and
reconfigure itself again.

Please note that, although AR488 interface ``++`` commands are verified by the
interface, and will respond accordingly, there is no sanity checking by the interface of
any direct instrument commands. These command sequences are sent directly to the
instrument, which should respond as though the command sequence were typed directly into
the terminal or sent from a suitable instrument control program. Please consult the
instrument user manual for information about the behaviour expected in response to
instrument commands.

Macro sequences can include any number delimiter separated of commands, but any
individual command sequence should not exceed 126 characters. This may be particularly
relevant to SCPI commands which can be composed of multiple instructions separated by
colons.

SN7516x GPIB transceiver support
--------------------------------

Support for the SN75160 and SN75161 GPIB transceiver integrated circuits can be enabled
by uncommenting the following line:

.. code-block:: c++

   //#define SN7516X

The pins used to control the ICs are defined in the section that follows:

.. code-block:: c++

   #ifdef SN7516X
     #define SN7516X_TE 6
   // #define SN75161_DC 13
   #endif

Specify the pin to be used for the ``SN7516X_TE`` signal. The above example shows pin 6
being used and this is connected to the talk-enable (``TE``) pin on both ICs. The
``SN75161`` handles the GPIB control signals and in addition to the ``TE`` pin, also has
a direction-control (``DC``) pin. This is used to determine controller or device mode
operation. A GPIO pin can be assigned to drive this pin, in which case the
``SN75151_DC`` definition shown above should be uncommented and an appropriate GPIO pin
number assigned.

Alternatively, since the ``REN`` signal is asserted in controller mode and un-asserted
in device mode, this signal can be used to drive the ``DC`` pin of the ``SN75161``. In
this case, the ``SN75161_DC`` definition should remain commented out and the GPIO pin
assigned to the REN signal should be connected to both ``DC`` and ``REN`` on the
``SN75161`` IC. There is one small caveat when using this configuration. The custom
``++ren`` command, which is used to turn the ``REN`` line on and off, cannot be used and
will just return::

  Unavailable.

If a separate GPIO pin is used to control DC then the ++REN command will return the
status of ``REN`` as usual. (See ``++ren`` in the `Custom Commands` section of the
`Command Reference`_).

Bluetooth HC05 module Options
-----------------------------

This section is used to configure Bluetooth HC05 module options and looks like the
below:

.. code-block:: c++

   //#define AR_BT_EN 12  // Bluetooth enable and control pin
   #ifdef AR_BT_EN
    #define AR_BT_BAUD 115200     // Bluetooth module preferred baud rate
    #define AR_BT_NAME "AR488-BT" // Bluetooth device name
    #define AR_BT_CODE "488488"   // Bluetooth pairing code
   #endif

To enable Bluetooth HC05 module auto-configuration, the first line needs to have the
preceding comment characters (``//``) removed and a GPIO pin assigned. It is then
necessary to set the configuration parameters, including baud rate, the name that the
device will be identified with and the pairing code. The ``AR_BT_BAUD`` parameter must
not have double quotes around it.

By default, the name is ``AR488-BT`` and the pairing code is ``488488``. The HC05 module
only needs connecting to the ``RX/TX`` pins of a serial port and it will be
automatically configured with these parameters on interface start-up.

This feature cannot work with the HC06 module as it does not have management mode or an
enable pin implemented. Full details of Bluetooth configuration and wiring are included
in the separate `AR488 Bluetooth Support` supplement.

Debug options
-------------

The AR488 can send certain debug messages to a serial port which can be helpful when
trying to diagnose a problem. These should not be required or enabled for normal running
of the interface, but if required for debugging, one or more of the following can be
enabled by removing the preceding ``//`` comment characters:

.. code-block:: c++

   //#define DEBUG1  // getCmd
   //#define DEBUG2  // setGpibControls
   //#define DEBUG3  // gpibSendData
   //#define DEBUG4  // spoll_h
   //#define DEBUG5  // attnRequired
   //#define DEBUG6  // EEPROM
   //#define DEBUG7  // gpibReceiveData
   //#define DEBUG8  // ppoll_h
   //#define DEBUG9  // bluetooth

By default, debug messages will be sent to the serial port that is used for
communication. Where the interface provides additional serial ports or where there are
sufficient GPIO pins available to use `SoftwareSerial`_, it is possible to send debug
messages to an alternative serial port. This has the advantage that debug messages will
no longer interfere with normal interface communications.

The debug messages can be viewed on the alternative ``debug`` port while normal
interface operations are in progress on the communications port.

To enable this feature uncomment the following line in the ``Debug Options`` section in
``AR488_Config.h``:

.. code-block:: c++

   //#define DB_SERIAL_PORT Serial1

Set the serial port to the port that will receive the debug messages. Configure the baud
rate, set the serial port type, and if using `SoftwareSerial`_, the GPIO pins to be
used, for example:

.. code-block:: c++

   #define DB_SERIAL_BAUD 57600
   #define DB_SW_SERIAL
   #ifdef DB_SW_SERIAL
     #define DB_SW_SERIAL_RX 53
     #define DB_SW_SERIAL_TX 51
   #endif

The above will configure a SoftwareSerial_ port at 57600 baud on GPIO pins 53 and 51.
Please note that the maximum advisable speed for a SoftwareSerial_ port is 57600 baud.

Debug messages do not include messages shown when verbose mode is enabled with the
``++verbose`` command. When the interface is being directly controlled by another
program, verbose mode should be turned off otherwise verbose messages may interfere with
normal operations.

Custom Board Layout Section
---------------------------

The custom board layout section in the Config.h file can be used to create a custom pin
layout for the AR488. This can be helpful for non-Arduino boards and where an adjustment
to the layout is required in order to accommodate additional hardware. By default, the
definition implements the Uno layout:


.. code-block:: c++

   #define DIO1  A0  /* GPIB 1  */
   #define DIO2  A1  /* GPIB 2  */
   #define DIO3  A2  /* GPIB 3  */
   #define DIO4  A3  /* GPIB 4  */
   #define DIO5  A4  /* GPIB 13 */
   #define DIO6  A5  /* GPIB 14 */
   #define DIO7  4   /* GPIB 15 */
   #define DIO8  5   /* GPIB 16 */

   #define IFC   8   /* GPIB 9  */
   #define NDAC  9   /* GPIB 8  */
   #define NRFD  10  /* GPIB 7  */
   #define DAV   11  /* GPIB 6  */
   #define EOI   12  /* GPIB 5  */

   #define SRQ   2   /* GPIB 10 */
   #define REN   3   /* GPIB 17 */
   #define ATN   7   /* GPIB 11 */


To make use of a custom layout, ``AR488_CUSTOM`` must be selected from the list of
boards at the beginning of the ``Config.h`` file and the pin numbers/designations in the
centre column (shown in bold) should be configured as required.

Please note that on some MCU boards, a number of GPIO pins may not be available as
inputs and/ or outputs despite a pad or connector being present. Please check the board
documentation. Sometimes such information is revealed only in online forum discussions
or blogs.

When ``AR488_CUSTOM`` is defined, interrupts cannot be used to detect pin states and
therefore ``USE_INTERRUPTS`` will not be defined and interrupts will not be activated.
Pin states will be checked on every iteration of ``void loop()`` instead.
