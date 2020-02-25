# AR488 ESP8266 WiFi add-on module

The <b>AR488 ESP8266 addon</b> module adds WiFi connectivity to the AR488 interface.The ESP8266 connects to the Arduino board via a Serial port and the ESP8266 provides bi-directional TCP/Serial passthrough and allows the GPIB interface be accessed over WiFi using TCP/IP.

<b>The sketch is still very much a work in progress with features that are still to be completed, for example authentication. Please feel free to download and experiment, but please be aware that this has been provided "as is" and to be used at your own risk. Please note that if SSL is disabled, then all transmission will be in clear text!</b>

The AR488-ESP8266-addon sketch (AR488-ESP8266-addon.ino) needs to be uploaded to and ESP8266 module. The ESP8266 module should then be connected to the AR488 via the Rx/Tx serial pins. It is not possible to progam the ESP8266 module while it is connected via serial to some other device, e.g. an Arduino. Attempting to do so will result in a timeout error and the upload will fail. Once the sketch has been uploaded, the serial connection should then be made by connecting TX on the ESP8266 to the RX pin on the Arduino and the RX pin on the ESP8266 to the TX pin on the Arduino. 

Once a WiFi connection is established to the ESP8266, the <i>Status</i> tab will show the current status,  IP address and SSID of the WiFi module, as well as the status of the GPIB passthrough mode. Once the serial connection has been established with an Arduino running the AR488 code, the status page should show the GPIB address and firmware version of the GPIB-USB adapter.

The <i>General</i> tab enables and disables features of the WiFi module. The default port used by the server TCP port 443 but this can be changed to any port required. If SSL is disabled, then the default port will be TCP port 80. However, any port can be selected as required. Also, by default GPIB communciation over TCPIP is turned off, so will need to be enabled by sliding the <i>Gpib Pass</i> switch to the 'On' position. 

The <i>WiFi</i> tab allows the ESP8266 module to be set up as a standalone Access Point (AP) or connected to an existing WiFi Access Point. By default, the ESP8266 will be in AP mode and the WiFi SSID is set to "AR488wifi". Sliding the switch to Client will change to WiFi Station (client) mode and the interface can then be connected to an existing WiFi network by supplying its SSID and WPA password. Either a static or DHCP address can be assigned. The WiFi passkey is not stored in flash, however, this information will be transmitted when the 'Apply' button is pressed and stored (cached) within the ESP8266 WiFi chip module itself.

The <i>GPIB</i> tab allows the user to configure some of the primary parameters of the AR488 adapter.

The <i>Admin</i> tab provides administrative functions such as allowing the WiFi module to be restarted, the AR488 interface to be re-booted, and an access password to be set. The access password is not stored or transmitted although an hash of the password will be transmitted for verification. Since currently there is no authentication, this password currently has no meaning.

The add-on module code has its own SSL certificate which is valid for a considerable length of time, but a certificate can also be generated using openssl.

Using SSL adds considerable workload to the ESP8266 web server. While not recommended, there may be reasons why perfomance is preferred over security in which case SSL can be disabled by uncommenting the following line at the beggining of the script:

<pre>//#define DISABLE_SSL</pre>

The web pages will then be sent in clear text and without the performance overhead of SSL. At present there seems to be no way to make this a runtime configuration option so this has been included as a compile time option instead.

Work to be completed:

- local authentication
- completion of Admin fuctions
- possible conversion to spiffs

Comments and feedback can be provided here:

https://www.eevblog.com/forum/projects/ar488-arduino-based-gpib-adapter/

In the event that a problem is found, this can be logged via the Issues feature on the AR488 GitHub page:

https://github.com/Twilight-Logic/AR488/issues

