# AR488 ESP8266 WiFi add-on module

The <B>AR488 ESP8266 addon</B> module adds WiFi connectivity to the AR488 interface.The ESP8266 connects to the Arduino board via a Serial port and the ESP8266 provides bi-directional TCP/Serial passthrough and allows the GPIB interface be accessed over WiFi using TCP/IP.

<b>The sketch is very much a work in progress and a number of features have not yet been completed, for example SSL and authentication. Please feel free to download it and experiment, but please be aware that this has been provided "as is" and at present all transmission is in clear text. Therefore use at your own risk.</b>

The AR488-ESP8266-addon sketch (AR488-ESP8266-addon.ino) needs to be uploaded to the ESP8266. The ESP8266 should then be connected to the AR488 via the Rx/Tx serial pins. It is not possible to progam the ESP8266 while it is connected via serial to some other device, including an Ardiono. Attempting to do so will result in a timeout error. The serial connection should be made by connecting TX on the ESP8266 to the RX pin on the Arduino and the RX pin on the ESP8266 to the TX pin on the Arduino. 

Once a WiFi connection is established to the ESP8266, the <i>Status</i> tab will show the current status,  IP address and SSID of the WiFi module, as well as the status of the GPIB passthrough mode. Once a serial connection has been established with an Arduino running the AR488 code, the status page will also show the GPIB address and firmware version of the GPIB-USB adapter.

The <i>General</i> tab enables and disables features of the WiFi module. The default port used by the server is port 80 but this can be changed to any port required. Sliding the SSL switch to the 'On' position will change the port to 443, however SSL support (HTTPS) is not currently provided so the switch presently has no effect. Any port can be selected as required and for the present, all communication will be HTTP only. Also, by default GPIB communciation over TCPIP is turned off, so will need to be enabled by sliding the <i>Gpib Pass<i> switch to the 'On' position. 

The <i>WiFi</i> tab allows the ESP8266 module to be set up as a standalone Access Point (AP) or connected to an existing WiFi Access Point. By default, the ESP8266 will be in AP mode and the WiFi SSID is set to "AR488wifi". Sliding the switch to Client will change to WiFi Station (client) mode and the interface can then be connected to an existing WiFi network by supplying its SSID and WPA password. Either a static or DHCP address can be assigned. The WiFi passkey is not stored in flash, however, this information will be transmitted when the 'Apply' button is pressed and stored (cached) within the ESP8266 WiFi chip module itself.

The <i>GPIB</i> tab allows the user to configure some of the primary parameters of the AR488 adapter.

The <i>Admin</i> tab provides administrative functions such as allowing the WiFi module to be restarted, the AR488 interface to be re-booted, and an access password to be set. The access password is not stored or transmitted although an hash of the password will be transmitted for verification. Since currently there is no authentication, this password currently has no meaning.

Work to be completed:

- local authentication
- SSL (HTTPS) support
- completion of Admin fuctions
- possible conversion to spiffs

Comments and feedback can be provided here:

https://www.eevblog.com/forum/projects/ar488-arduino-based-gpib-adapter/

In the event that a problem is found, this can be logged via the Issues feature on the AR488 GitHub page:

https://github.com/Twilight-Logic/AR488/issues




