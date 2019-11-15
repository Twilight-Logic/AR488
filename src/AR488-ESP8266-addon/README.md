# AR488 ESP8266 WiFi add-on module

The AR488 ESP8266 WiFi add-on module adds wireless connectivity to the AR488 interface and allows the interface be accessed over TCP/IP.

First, the AR488-ESP8266-addon sketch AR488-ESP8266-addon.ino must be uploaded to the board. The the module should be connected to the AR488 via the Rx/Tx serial pins. Do not attempt to program the ESP8266 board while it is connected via serial to the AR488. This will result in an error.

If properly connected and the serial connection is working, the <i>Status</i> tab will show the current status of the WiFi module as well as the GPIB mode, address and firmware version of the GPIB-USB adapter.

The <i>General</i> tab enables and disables features of the WiFi module. By default SSL support is tunred off and the module is configured for remote control only. To allow GPIB communciation over TCPIP, head to the General tab and turn on <i>Gpib Pass<i>. By default, TCP port 8488 is used for communication, but this can be changed to any TCP port up to 49151. Again, by default port 80 is used for browser communication and this will automatically switch to port 443 is used, but any port can be selected.

The <i>WiFi</i> tab allows the ESP8266 module to be set up as a standalone Access Point (AP) or connected to an existing WiFi Access Point. By default, the WiFi SSID is set to AR488wifi and the ESP8266 will be in AP mode. Sliding the switch to Client will change to WiFi client mode and the interface can be connected to an existing WiFi network.Either a static or DHCP address can be assigned. The sketch itself does not store the WiFi passkey, however, this information may be stored within the ESP8266 module itself.

The <i>GPIB</i> tab allows the user to configure some of the primary parameters of the AR488 adapter.

The <i>Admin</i> tab provides administrative functions such as allowing the WiFi module to be restarted, the AR488 interface to be re-booted, and an access password to be set. The access password is not stored or transmitted. Only the SHA1 hash of the password is ever transmitted.


