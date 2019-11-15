/*
 * AR488 ESP8266 WiFi add-on 
 * 
 * ESP8266 WiFi module for the AR488 GPIB-USB adapter
 * Requires an ESP8266 NodeMCU board
 * 
 * (c) Twilight Logic, 15/15/2019
 */

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Hash.h>


#define VERSION "0.03.15" // 15-10-2019


/***** Enable debug modes *****/
//#define DEBUG_0   // Setup and loop
//#define DEBUG_1   // WiFi functions
#define DEBUG_2   // Webserver/telnet functions
//#define DEBUG_3   // EEPROM save


/***** Parameter variables *****/
#define BAUD 115200		// serial baud rate
#define pktTmo 5 		  // serial timout, if no more serial data then send packet

#define CR 0x13
#define LF 0x10

/***** Parameters saved in EEPROM *****/
struct wapCfg {
  uint8_t cflg;   // Config flag
  uint8_t wmode;  // WiFi mode - 0: AP; 1: client-static; 2: client-dhcp;
  char essid[20];
  char pwd[20];
  char pwdChk[20];
  IPAddress addr;
  IPAddress gate;
  IPAddress mask;
  uint16_t webp = 80;  // Web server port
  uint16_t gpibp = 8488; // GPIB passthrough port
  bool ssl = false;
  bool gpib = false;
};
struct wapCfg AP;

/***** Web server and client objects *****/
ESP8266WebServer *AR488srv = new ESP8266WebServer(AP.webp);
WiFiServer *passSrv = new WiFiServer(AP.gpibp);
WiFiClient passCli;

/***** Buffers *****/
const uint16_t htmlSize = 4096;
char html[htmlSize];
const uint16_t bufSize = 4096;
uint8_t pBuf[bufSize];
uint16_t pbPtr = 0;
//uint8_t buf2[bufSize];
//uint16_t i2 = 0;
//char rBuf[64];

/**********************************/
/*****  MAIN PROGRAM  *****/
/**********************************/

void setup() {

  Serial.begin(BAUD);

#ifdef DEBUG_0
  Serial.println();
  Serial.println(F("Starting WiFi..."));
#endif

  // Set defaults
  setWifiDefault();

  // Read config data from non-volatile memory
  //(will only read if a previous config has already been saved)
  epGetCfg();

  // Start WiFi
  startWifi();

  // Start webserver
#ifdef DEBUG_0  
  Serial.println(F("Starting webserver..."));
#endif  
  startWebServer();

  if (AP.gpib) passSrv->begin();

}


void loop() {

  // Handle requests for web server
  AR488srv->handleClient();

  // Is GPIB passthrough enabled?
  if (AP.gpib) {
    // Handle incoming client connections
    if (passCli.connected()) {
      // Handle connection
      while(passCli.available()){
        if (pbPtr<bufSize-1){
          pBuf[pbPtr] = passCli.read();
          pbPtr++;
        }
      }
      Serial.write(pBuf, pbPtr);
      pbPtr = 0;

      while(Serial.available()) {
        passCli.write(Serial.read());
      }
 
     
     }else{
      // Wait for a connection
      passCli = passSrv->available();
    }
  }

}


/*****************************/
/***** PROGRAM FUNCTIONS *****/
/*****************************/

/***** Start WiFi *****/
void startWifi(){
  // Check if already configured
  if (AP.cflg!=0x1C) setWifiDefault();
  // Start WiFi
  if (AP.wmode==0) startWifiAp(); 
  if (AP.wmode>0) {
    if (!startWifiStn()) {
      // Default AP mode fallback
      setWifiDefault();
      startWifiAp();
    }
  }
}


/***** Return WiFi to default AP settings *****/
void setWifiDefault() {
  String sha = sha1("AR488-setup");
  sha.toCharArray(AP.pwdChk, 20);


  Serial.print(F("Pwd hash: "));
  Serial.println(sha);
  
  AP.cflg = 0;
  AP.wmode = 0;
  strncpy (AP.essid, "AR488wifi\0", 20);
  strncpy (AP.pwd, "AR488-setup", 20);
  AP.addr = IPAddress(192,168,4,88);
  AP.gate = IPAddress(192,168,4,88);
  AP.mask = IPAddress(255,255,255,0);
  AP.webp = 80;
  AP.gpibp = 8488;
  AP.gpib = false;
  
}


/***** Check whether WiFi has started *****/
bool hasWifiStarted() {
  uint16_t tmo = 60000;
  uint16_t dly = 500;

  // Wait for connection
  while ((WiFi.status() != WL_CONNECTED) && (tmo>0)) {
    delay(dly);
    tmo = tmo-dly;
#ifdef DEBUG_1   
    Serial.print(".");
    Serial.print(F("WiFi status:"));
    Serial.println(WiFi.status());
#endif
  }
#ifdef DEBUG_1
  Serial.println();
  Serial.print(F("WiFi status:"));
  Serial.println(WiFi.status());
#endif

  if (WiFi.status() == WL_CONNECTED) {
    // Success!
#ifdef DEBUG_1
    Serial.println("WiFi started.");
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
#endif    
    return true;
  }else{
    // Timed out
#ifdef DEBUG_1
    Serial.println("WiFi failed.");
#endif    
    return false;
  }
}


/***** Start WiFi an access point mode ****/
/*
 * This mode allows a device to connect directly to the
 * AR488 WiFi interface without using a router
 */
void startWifiAp(){
#ifdef DEBUG_1
  Serial.println(F("SoftAP mode>"));
#endif  
  // Setup and start the access point
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP.addr, AP.addr, AP.mask); // default IP address
  WiFi.softAP(AP.essid, AP.pwd); // SSID and password
//  WiFi.begin();
  delay(500);
}


/***** Start WiFi in station mode *****/
bool startWifiStn(){
#ifdef DEBUG_1  
  Serial.println(F("Station mode>"));
#endif  
  // Station mode
  WiFi.mode(WIFI_STA);
  // Static IP address mode
  if (AP.wmode==1) WiFi.config(AP.addr, AP.gate, AP.mask, AP.gate);
  // DHCP mode
  if (AP.wmode==2) WiFi.config(0u,0u,0u);
  // Connect to AP
  WiFi.begin(AP.essid, AP.pwd); // Connect to an AP with SSID and password (DHCP address)
  if (hasWifiStarted()) return true;
  return false;
}


/***** Start the web server *****/
void startWebServer(){
  AR488srv->begin(); // Start the HTTP server
  // Pages being served
  AR488srv->on("/", wwwMain);         // Default web page
  AR488srv->on("/seeStat", seeStat);  // Status page
  AR488srv->on("/cfgGen", cfgGen);    // General options
  AR488srv->on("/cfgWifi", cfgWifi);  // WiFi config
  AR488srv->on("/cfg488", cfg488);    // AR488 config
  AR488srv->on("/cfgAdm", cfgAdm);    // Admin options
  AR488srv->on("/style", lnkStyle);   // Stylesheet

  AR488srv->on("/script", lnkScript);     // Main Script
  AR488srv->on("/cfgGenjs", cfgGenjs);    // General page Script
  AR488srv->on("/cfgWiFijs", cfgWiFijs);  // WiFi config Script
  AR488srv->on("/cfg488js", cfg488js);    // AR488 config Script
  AR488srv->on("/cfgAdmjs", cfgAdmjs);    // Admin Script
  AR488srv->on("/SHA1js", SHA1js);        // SHA1 Script

  // Argument handlers for pages
  AR488srv->on("/setWifi", setWifi);  // Set WiFi parameters
  AR488srv->on("/setGen", setGen);    // Set WiFi parameters
  AR488srv->on("/set488", set488);    // Set GPIB parameters
  AR488srv->on("/admin", admin);      // Set WiFi parameters
  
#ifdef DEBUG_2  
  Serial.println("Web server started.");
#endif
}


/***** Read configuration from EEPROM *****/
void epGetCfg() {
  int val;
  val = EEPROM.read(0);
  if (val == 0x1C) {
    EEPROM.get(0, AP);
  }
}


/***** Save configuraton to EEPROM *****/
uint8_t epSaveCfg() {

#ifdef DEBUG_3
  long int sz;
  sz = sizeof(AP);
  Serial.print(F("Size of structure: "));
  Serial.println(sz);
#endif

  EEPROM.put(0, AP);
  return OK;
}



/****************************/
/***** WEB SERVER PAGES *****/
/****************************/

/***** Main web page *****/
void wwwMain() {
  uint8_t rsz = 0;
  char reply[64] = {'\0'};
  char fwver[10] = {'\0'};
  char *vp;
  
  rsz = getReply("++ver real", reply, 47);
  
  if (rsz>0) {
    vp = strstr(reply, "ver.") + 5;
    for (int i=0; i<7; i++) {
      fwver[i] = *vp;
      vp++;
    }
  }
  
  snprintf(html, htmlSize,
"<html>\
<head>\
<meta charset=\"utf-8\">\
<title>AR488 WiFi Configuration</title>\
<link rel=\"stylesheet\" href=\"style\">\
<script defer src=\"/script\"></script>\
<script defer src=\"/cfgGenjs\"></script>\
<script defer src=\"/cfgWiFijs\"></script>\
<script defer src=\"/cfg488js\"></script>\
<script defer src=\"/cfgAdmjs\"></script>\
<script defer src=\"/SHA1js\"></script>\
</head>\
<body onload=\"getPage('','')\";>\
<div class=\"mpage\">\
<div class=\"headr\">AR488 WiFi Configuration</div>\
<div>\
<ul>\
<li><a id='lstat' onclick=\"getPage('/seeStat',this)\">Status</a></li>\
<li><a onclick=\"getPage('/cfgGen',this)\">General</a></li>\
<li><a onclick=\"getPage('/cfgWifi',this)\">WiFi</a></li>\
<li><a onclick=\"getPage('/cfg488',this)\">AR488</a></li>\
<li><a onclick=\"getPage('/cfgAdm',this)\">Admin</a></li>\
</ul>\
</div>\
<hr>\
<div id=\"cfgPage\" class=\"conf\">\
Loading...\
</div>\
<div>\
<hr>\
<table class=\"foot\"><tr>\
<td>AR488 WiFi ver: %s</td>\
<td>Firmware ver: %s</td>\
</tr></table>\
</div>\
</div>\
</body>\
</html>",
  VERSION,
  fwver
);
  AR488srv->send(200, "text/html", html);
}


/***** Status page *****/
void seeStat() {
  char wmode[8] = {0x53,0x6F,0x66,0x74,0x20,0x41,0x50,0x0};
  char dhcp[9] = {0x44,0x69,0x73,0x61,0x62,0x6C,0x65,0x64,0x0};
  char hide[7] = {'\0'};
  char gpib[9] = {0x44,0x69,0x73,0x61,0x62,0x6C,0x65,0x64,0x0};
  IPAddress addr;
  IPAddress gate;
  IPAddress mask = WiFi.subnetMask();
  char reply[64] = {'\0'};
  uint8_t rsz;
  uint8_t gmode = 255;
  uint8_t gaddr = 255;
//  char gauto = 0x48;
  
  if (AP.wmode==0) {
    strncpy(hide, "hidden\0", 7);
    addr = WiFi.softAPIP();
  }
  
  if (AP.wmode>0) {
    strncpy(wmode, "Station\0", 8);
    addr = WiFi.localIP();
    gate = WiFi.gatewayIP();
  }

  if (AP.wmode==2) strncpy(dhcp, "Enabled\0", 8);

  if (AP.gpib) strncpy(gpib, "Enabled\0", 8);

  flushIncoming();
  rsz = getReply("++mode", reply, 64);
  if (rsz) gmode = atoi(reply);
  rsz = getReply("++addr", reply, 64);
  if (rsz) gaddr = atoi(reply);

/*
  if (gmode==1){  // Must be controler to send *idn?
    // Get status of auto
    rsz = getReply("++auto", reply, 63);
    if (rsz>0) gauto = reply[0];
    // Set auto to 2 to get automatic reply
    Serial.println("++auto 2");
    delay(200);
    // get reply to *idn?
    rsz = getReply("*idn?", reply, 63);
    if (rsz<1) strncpy(reply, "no IDN detected\0", 16);
    // Set auto back to original value
    Serial.println("++auto " + gauto);
    delay(200);
  }else{
*/
    // Blank last reply
    strncpy(reply, "n/a\0", 4); 
//  }

  snprintf(html, htmlSize, 
"<table>\
<tr><th>WiFi mode:</th><td>%s</td></tr>\
<tr><th>DHCP:</th><td>%s</td></tr>\
<tr><th>IP address:</th><td>%d.%d.%d.%d</td></tr>\
<tr %s><th>Gateway:</th><td>%d.%d.%d.%d</td></tr>\
<tr %s><th>Netmask:</th><td>%d.%d.%d.%d</td></tr>\
<tr><th>Mac addr:</th><td>%s</td></tr>\
<tr><th>ESSID:</th><td>%s</td></tr>\
<tr><th>GPIB pass:</th><td>%s</td></tr>\
<tr><th>GPIB mode:</th><td>%d</td></tr>\
<tr><th>GPIB addr:</th><td>%d</td></tr>\
</table>",
  wmode,
  dhcp,
  addr[0],
  addr[1],
  addr[2],
  addr[3],
  hide,
  gate[0],
  gate[1],
  gate[2],
  gate[3],
  hide,
  mask[0],
  mask[1],
  mask[2],
  mask[3],
  WiFi.macAddress().c_str(),
  AP.essid,
  gpib,
  gmode,
  gaddr
  );
  AR488srv->send(200, "text/html", html);

/*
   reply   <tr><th>Instrument:</th><td>%s</td></tr>\
*/
}


/***** General functions configuration page *****/
void cfgGen() {
//  char chkd1[8] = {'\0'};
  char chkd2[8] = {'\0'};
  char disd[9] = {'\0'};
  
  if (AP.gpib) {
    strncpy(chkd2, "checked\0", 8);
  }else{
    strncpy(disd, "disabled\0", 9);
  }

  snprintf(html, htmlSize,
"<form method=\"post\" action=\"/setGen\" onchange=\"enableButton(0x1);\"/>\
<table>\
<tr><th>SSL:</th><td>Off</td>\
<td><label class=\"switch\">\
  <input name=\"ssl\" type=\"checkbox\" onclick=\"toggleSSL(this);\" \"/>\
  <span class=\"slider\"></span>\
</label></td>\
<td>On</td></tr>\
<tr><th>GPIB Pass:</th><td>Off</td>\
<td><label class=\"switch\">\
  <input name=\"pass\" type=\"checkbox\" onclick=\"togglePass(this);\" %s/>\
  <span class=\"slider\"></span>\
</label></td>\
<td>On</td></tr>\
<tr><th>Admin Port:</th><td colspan=3><input name=\"webp\" value=\"%d\"/></td></tr>\
<tr><th>GPIB Port:</th><td colspan=3><input name=\"gpibp\" value=\"%d\" %s/></td></tr>\
</table>\
<input type=\"button\" class=\"btn\" id=\"btnApply\" value=\"Apply\" onclick=\"genValidate()\" disabled/>\
</form>",
  chkd2,
  AP.webp,
  AP.gpibp,
  disd
  );
    AR488srv->send(200, "text/html", html);
}


/***** WiFi configuration page *****/
void cfgWifi(){
  char chkd1[8] = {'\0'};
  char chkd2[8] = {'\0'};
  char disd[9] = {'\0'};

  if (AP.wmode==0) strncpy(disd, "disabled\0", 9);
  if (AP.wmode>0) strncpy(chkd1, "checked\0", 8);
  if (AP.wmode==2) strncpy(chkd2, "checked\0", 8);

  snprintf(html, htmlSize,
"<form method='post' action='/setWifi' onchange=\"enableButton(0x1);\"/>\
<input name='mode' value='0' hidden>\
<table>\
<tr><th>WiFi Mode:</th><td>AP</td>\
<td><label class='switch'>\
  <input name='chkM' type='checkbox' onchange='wifiMode(this);' %s/>\
  <span class='slider'></span>\
</label></td>\
<td>Client</td></tr>\
<tr><th>DHCP:</th><td>Off</td>\
<td><label class='switch'>\
  <input name='chkD' type='checkbox' onchange='wifiMode(this);' %s %s/>\
  <span class='slider'></span>\
</label></td>\
<td>On</td></tr>\
</table>\
<table>\
<tr><th>ESSID:</th><td><input name='essid' value='%s'/></td></tr>\
<tr><th>Password:</th><td><input name='pword' type='password' value=''/></td></tr>\
<tr><th>IP address:</th><td>\
<input class='ip' name='addr1' value='%d'/>.\
<input class='ip' name='addr2' value='%d'/>.\
<input class='ip' name='addr3' value='%d'/>.\
<input class='ip' name='addr4' value='%d'/>\
</td></tr>\
<tr><th>IP gateway:</th><td>\
<input class='ip' name='gate1' value='%d' disabled/>.\
<input class='ip' name='gate2' value='%d' disabled/>.\
<input class='ip' name='gate3' value='%d' disabled/>.\
<input class='ip' name='gate4' value='%d' disabled/>\
</td></tr>\
<tr><th>IP netmask:</th><td>\
<input class='ip' name='mask1' value='%d' disabled/>.\
<input class='ip' name='mask2' value='%d' disabled/>.\
<input class='ip' name='mask3' value='%d' disabled/>.\
<input class='ip' name='mask4' value='%d' disabled/>\
</td></tr>\
</table>\
<input type=\"button\" class=\"btn\" id=\"btnApply\" onclick=\"wifiValidate()\" value=\"Apply\" disabled/>\
</form>",
  chkd1,
  chkd2,
  disd,
  AP.essid,
  AP.addr[0],
  AP.addr[1],
  AP.addr[2],
  AP.addr[3],
  AP.gate[0],
  AP.gate[1],
  AP.gate[2],
  AP.gate[3],
  AP.mask[0],
  AP.mask[1],
  AP.mask[2],
  AP.mask[3]
  );
   AR488srv->send(200, "text/html", html);   
}


/*
 * AR488 configuration page
 */
void cfg488() {
  char chkm[8] = {'\0'};
  char chki[8] = {'\0'};
  char chkt[8] = {'\0'};
  char dis[9] = {'\0'};
  
  bool cmode = false;
  bool ceoi = false;
  bool ceot = false;
  
  uint8_t gaddr = 0;
  uint8_t gauto = 0;
  uint16_t gtmo = 0;
  uint8_t geosch = 0;
  uint8_t geotch = 0;  

  char vstr[48] = {'\0'};
  char ver[48] = {'\0'};
  char reply[64] = {'\0'};
  
  uint8_t rsz = 0;

  // Get status of switches
  rsz=getReply("++mode", reply, 64);
  if (rsz>0) cmode = (atoi(reply) ? true : false);
  if (cmode) {
    strncpy(chkm, "checked\0", 8);
  }else{
    strncpy(dis, "disabled\0", 9);
  }

  rsz=getReply("++eoi", reply, 64);
  if (rsz>0) ceoi = (atoi(reply) ? true : false);
  if (ceoi) strncpy(chki, "checked\0", 8);

  rsz=getReply("++eot_enable", reply, 64);
  if (rsz>0) ceot = (atoi(reply) ? true : false);
  if (ceot) strncpy(chkt, "checked\0", 8);


  // Get parameters
  rsz=getReply("++addr", reply, 64);
  if (rsz>0) gaddr = atoi(reply);
  rsz=getReply("++auto", reply, 64);
  if (rsz>0) gauto = atoi(reply);
  rsz=getReply("++read_tmo_ms", reply, 64);
  if (rsz>0) gtmo = atoi(reply);
//  rsz=getReply("++eoi", reply, 64);
//  if (rsz>0) geoi = atoi(reply);
  rsz=getReply("++eos", reply, 64);
  if (rsz>0) geosch = atoi(reply);
//  rsz=getReply("++eot", reply, 64);
//  if (rsz>0) geot = atoi(reply);
  rsz=getReply("++eot_char", reply, 64);
  if (rsz>0) geotch = atoi(reply);
  
  rsz=getReply("++ver", reply, 48);
  if (rsz>0) strncpy(vstr, reply, rsz);
  
  rsz=getReply("++ver real", reply, 48);
  if (rsz>0) strncpy(ver, reply, rsz);

  snprintf(html, htmlSize,
"<form method=\"post\" action=\"/set488\" onchange=\"enableButton(0x3)\"/>\
<input name=\"opt\" value=\"0\" type=\"hidden\"/>\
<table>\
<tr><th>Mode:</th><td>Device</td>\
<td><label class=\"switch\">\
  <input name=\"cmode\" type=\"checkbox\" onchange=\"toggleCmode(this);\" %s/>\
  <span class=\"slider\"></span>\
</label></td>\
<td>Controller</td></tr>\
<tr><th>GPIB address:</th><td colspan=\"3\"><input name=\"gpib\" value=\"%d\"/></td></tr>\
<tr><th>Auto:</th><td colspan=\"3\"><input name=\"auto\" value=\"%d\" %s/></td></tr>\
<tr><th>Read timeout:</th><td colspan=\"3\"><input name=\"readtmo\" value=\"%d\" %s/></td></tr>\
<tr><th>EOI:</th><td>Disabled</td>\
<td><label class=\"switch\">\
  <input name=\"eoi\" type=\"checkbox\" %s/>\
  <span class=\"slider\"></span>\
</label></td>\
<td>Enabled</td></tr>\
<tr><th>EOS char:</th><td colspan=\"3\"><input name=\"eosch\" value=\"%d\"/></td></tr>\
<tr><th>EOT Enable:</th><td>Disabled</td>\
<td><label class=\"switch\" %s>\
  <input name=\"eot\" type=\"checkbox\"/>\
  <span class=\"slider\"></span>\
</label></td>\
<td>Enabled</td></tr>\
<tr><th>EOT char:</th><td colspan=\"3\"><input name=\"eotch\" value=\"%d\"/></td></tr>\
<tr><th>Version string:</th><td colspan=\"3\"><input name=\"vstr\" value=\"%s\"/></td></tr>\
</table>\
<input type=\"submit\" class=\"btn\" value=\"Apply\" id=\"btnApply\" onclick=\"apply488Cfg();\" disabled/>\
<input type=\"submit\" class=\"btn\" value=\"Save\" id=\"btnSave\" onclick=\"save488Cfg();\" disabled/>\
</form>",
  chkm,
  gaddr,
  gauto,
  dis,
  gtmo,
  dis,
  chki,
  geosch,
  chkt,
  geotch,
  vstr
  );
   AR488srv->send(200, "text/html", html);
}


/***** Admin functions page *****/
void cfgAdm() {
  snprintf(html, htmlSize,
"<form method=\"post\" id=\"frmAdm\" action=\"/admin\">\
<input name=\"chk1\" value=\"\" type=\"hidden\"/>\
<input name=\"chk2\" value=\"\" type=\"hidden\"/>\
<input name=\"opt\" value=0 type=\"hidden\"/>\
<table>\
<tr><th>Current password:</th><td><input id=\"pwd1\" type=\"password\" onchange=\"enableButton(0x1C);\"/></td>\
<tr><th>New password:</th><td><input id=\"pwd2\" type=\"password\"/></td>\
<tr><th>Confirm new password:</th><td><input id=\"pwd3\" type=\"password\"/></td>\
<tr><th></th><td align=\"right\"><input type=\"button\" id=\"btnChpwd\" class=\"btn\" value=\"Change\" onclick=\"chgPwd();\" disabled/></td></tr>\
<tr><th></th><td><br></td></tr>\
<tr><th>Defaults</th><td align=\"right\"><input type=\"button\" id=\"btnRst\" class=\"btn\" value=\"Reset\" onclick=\"rstDflt();\" disabled/></td></tr>\
<tr><th>Restart WiFi</th><td align=\"right\"><input type=\"button\" class=\"btn\" id=\"btnRbt\" value=\"Reboot\" onclick=\"reboot();\" disabled/></td></tr>\
</table>\
</form>"
  );
   AR488srv->send(200, "text/html", html);   
}


/***** Return CSS stylesheet *****/
void lnkStyle() {
  snprintf(html, htmlSize, 
"body {width: 320; height: 424;}\
table {width: 100%%; font-size: 10pt; text-align: left; }\
th  {width: 120px;}\
ul  {list-style-type: none;\
  margin: 0;\
  padding: 0;\
  height: 24px;\
  font-size: 10pt;\
  text-align: center;}\
li  {float: left;width:20%%;}\
li a  {display: block;\
  border: 1px solid #777777;\
  color: #333333;\
  background-color: #FFEE77;\
  text-decoration: none;}\
li a:hover:not(.active) {\
  background-color: #777777;\
  color: cyan;}\
.active {background-color:#77CCFF;}\
.headr  {color: #0000AA;\
  font-size: 14pt;\
  font-weight: bold;\
  width: 320;\
  text-align: center;\
  padding: 10px 0px 10px 0px;}\
.foot {font-size: 8pt;}\
.mpage  {width: 320;\
    height: 424;\
    background-color: #EEEEEE;\
    font-family: verdana;\
    border: 1px solid #777777;\
    margin: 7px;\
    padding:7px;\
    box-shadow: 3px 3px 3px #AAAAAA;\
    }\
.conf {overflow-y: auto;\
  height: 300px;\
}\
.ip {width: 40px; }\
.switch {\
  position: relative;\
  display: inline-block;\
  width: 42px;\
  height: 20px;\
}\
.slider {\
  position: absolute;\
  cursor: pointer;\
  top: 0;\
  left: 0;\
  right: 0;\
  bottom: 0;\
  background-color: #2196F3;\
}\
.slider:before {\
  position: absolute;\
  content: '';\
  height: 16px;\
  width: 16px;\
  left: 2px;\
  bottom: 2px;\
  background-color: silver;\
}\
input:checked + .slider:before {\
  -webkit-transform: translateX(22px);\
  -ms-transform: translateX(22px);\
  transform: translateX(22px);\
  background-color: gold;\
}\
.btn {float: right;\
  width: 90px;\
  margin: 10px 5px 0px 0px;\
}\
"
   );
  AR488srv->send(200, "text/html", html);   
}


/***** Return main JavaScript code *****/
void lnkScript() {
  snprintf(html, htmlSize,
"var curMnObj='';\
function getPage(url,obj) {\
  cfgPage = document.getElementById('cfgPage');\
  var mstat = document.querySelector('#lstat');\
  if (obj) {\
    if (curMnObj) curMnObj.classList.remove('active');\
    curMnObj = obj;\
    curMnObj.classList.add('active');\
  }else{\
    if (curMnObj) {\
      curMnObj.classlist.add('active');\
    }else{\
      url = \"seeStat\";\
      curMnObj = mstat;\
      curMnObj.classList.add('active');\
    }\
  }\
  cfgPage.innerHTML = \"Updating...\";\
  fetch(url, {method: \"POST\"} )\
    .then((resp) => resp.text())\
    .then(function(data){\
        cfgPage.innerHTML = data;\
    })\
    .catch(function(error){\
      alert(\"Unable to load page!\");\
    });\
}\
function enableButton(sw){\
  if (sw&0x1) document.querySelector('#btnApply').disabled = false;\
  if (sw&0x2) document.querySelector('#btnSave').disabled = false;\
  if (sw&0x4) document.querySelector('#btnChpwd').disabled = false;\
  if (sw&0x8) document.querySelector('#btnRst').disabled = false;\
  if (sw&0x10) document.querySelector('#btnRbt').disabled = false;\
}"
  );
  AR488srv->send(200, "text/html", html);
}


/***** JavaScript code for Ceneral config page *****/
void cfgGenjs() {
  snprintf(html, htmlSize,
"function toggleSSL(ssl){\
  var webp = document.querySelector('[name=\"webp\"]');\
  if (ssl.checked && webp.value == '80') webp.value = 443;\
  if (ssl.checked==false && webp.value == '443') webp.value = 80;\
}\
function togglePass(pass){\
  var gpibp = document.querySelector('[name=\"gpibp\"]');\
  if (pass.checked) {\
    gpibp.disabled = false;\
  }else{\
    gpibp.disabled = true;\
  }\
}\
function genValidate() {\
  var adP = document.querySelector('[name=\"webp\"]').value;\
  var gpP = document.querySelector('[name=\"gpibp\"]').value;\
  if (adP==0||gpP==0 || adP>49151||gpP>49151) {\
    alert(\"TCP port is out of range!\\nPlease set a port value between 1 and 49151.\");\
  }else{\
    form.submit();\
  }\
}"
  );
  AR488srv->send(200, "text/html", html);
}


/***** JavaScript code for WiFi config page *****/
void cfgWiFijs() {
  snprintf(html, htmlSize,
"function wifiMode(mode){\
  var chkm = document.getElementsByName('chkM')[0];\
  var chkd = document.getElementsByName('chkD')[0];\
  var mode = document.getElementsByName('mode')[0];\
  var dhcp = document.getElementsByName('dhcp')[0];\
  var octets = document.querySelectorAll('.ip');\
  var i;\
  if (chkm.checked){\
    chkd.disabled = false;\
    for (i=0; i<octets.length; i++){\
      octets[i].disabled = chkd.checked;\
    }\
    mode.value = 1;\
    if (chkd.checked) mode.value = 2;\
  }else{\
    chkd.checked = false;\
    chkd.disabled = true;\
    for (i=0; i<4; i++) {\
      octets[i].disabled = false;\
    }\
    for (i=4; i<octets.length; i++){\
      octets[i].disabled = true;\
    }\
    mode.value = 0;\
  }\
}\
function wifiValidate(){\
  var form = document.getElementById('setWifi');\
  var essid = document.getElementsByName('essid')[0];\
  if (essid.value=='') {\
    alert('WiFi ESSID required!');\
  }else{\
    if (octOk()) form.submit();\
  }\
}\
function octOk(){\
  var octets = document.querySelectorAll('.ip');\
  var chkm = document.getElementsByName('chkM')[0];\
  var chkd = document.getElementsByName('chkD')[0];\
  var i=0;\
  var t=4;\
  if (chkm.checked) {\
    if (chkd.checked) {return true};\
    t=octets.length;\
  }else{\
    t=4;\
  }\
  for (i=0; i<t; i++){\
    if (octets[i].value<0 || octets[i].value>255) {\
      alert('Invalid IP address!');\
      return false;\
    }\
  }\
  return true;\
}"
  );
  AR488srv->send(200, "text/html", html);
}


/***** JavaScript code for AR488 config page *****/
void cfg488js() {
  snprintf(html, htmlSize,
"function toggleCmode(cmode){\
  var auto = document.querySelector('input[name=\"auto\"]');\
  var rtmo = document.querySelector('input[name=\"readtmo\"]');\
  if (cmode.checked) {\
  auto.disabled = false;\
    rtmo.disabled = false;\
  }else{\
  auto.disabled = true;\
    rtmo.disabled = true;\
  }\
}\
function apply488Cfg(){\
  document.querySelector('input[name=\"opt\"]').value = 1;\
  document.querySelector('form').submit;\
}\
function save488Cfg(){\
  document.querySelector('input[name=\"opt\"]').value = 2;\
  document.querySelector('form').submit;\
}"
  );
  AR488srv->send(200, "text/html", html);
}


/***** JavaScript code for Admin page *****/
void cfgAdmjs() {
  snprintf(html, htmlSize,
"function chgPwd(msg){\
  var pwd1 = document.querySelector('#pwd1').value;\
  var pwd2 = document.querySelector('#pwd2').value;\
  var pwd3 = document.querySelector('#pwd3').value;\
  if (pwd1==\"\"||pwd2==\"\"||pwd3==\"\") {\
    alert(\"Passwords cannot be blank!\");\
    return;\
  }\
  if (pwd1==(pwd2||pwd3)){\
    alert(\"New password cannot be the same as the old password!\");\
    return;\
  }\
  if (pwd2==pwd3){\
    runAdmReq(pwd1,pwd2,1);\
  }else{\
    alert(\"New passwords do not match!\");\
  }\
}\
function rstDflt(){\
  var pwd1 = document.querySelector('#pwd1').value;\
  if (pwd1){\
    var res = confirm(\"The interface will be reset to default AP mode (ESSID: AR488wifi, IP address: 192.168.4.88).\\nAre you sure you wish to proceed?\");\
    if (res) runAdmReq(pwd1,\"\",2);\
  }else{\
    alert(\"Please confirm the current admin password!\");\
  }\
}\
function reboot(){\
  var pwd1 = document.querySelector('#pwd1').value;\
  if (pwd1){\
    var res = confirm(\"The WiFi connection will be interrupted. Are you sure you wish to proceed?\");\
    if (res) runAdmReq(pwd1,\"\",3);\
  }else{\
    alert(\"Please confirm the current admin password!\");\
  }\
}\
function runAdmReq(pwd1, pwd2, act){\
  var prm1 = SHA1(pwd1);\
  var prm2 = SHA1(pwd2);\
  var prm3 = Promise.resolve(act);\
  Promise.all([prm1,prm2,prm3]).then(function(results){\
    formObj = document.querySelector('form');\
    var chk1 = document.querySelector('[name=\"chk1\"]');\
    var chk2 = document.querySelector('[name=\"chk2\"]');\
    var opt = document.querySelector('[name=\"opt\"]');\
    chk1.value = results[0];\
    if (results[2]==1) chk2.value=results[1];\
    opt.value=results[2];\
    formObj.submit();\
  });\
}"
  );
  AR488srv->send(200, "text/html", html);
}


void SHA1js() {
  snprintf(html, htmlSize,
"function SHA1(msg){\
  function rotate_left(n,s){\
      var t4 = (n<<s) | (n>>>(32-s));\
      return t4;\
  };\
  function lsb_hex(val) {\
    var str='';\
    var i;\
    var vh;\
    var vl;\
    for( i=0; i<=6; i+=2 ){\
      vh = (val>>>(i*4+4))&0x0f;\
      vl = (val>>>(i*4))&0x0f;\
      str += vh.toString(16) + vl.toString(16);\
    }\
    return str;\
  };\
  function cvt_hex(val){\
    var str='';\
    var i;\
    var v;\
    for( i=7; i>=0; i-- ) {\
      v = (val>>>(i*4))&0x0f;\
      str += v.toString(16);\
    }\
    return str;\
  };\
  function Utf8Encode(string) {\
    var utftext = '';\
    string = string.replace(/\\r\\n/g,\"\\n\");\
    for (var n = 0; n < string.length; n++){\
      var c = string.charCodeAt(n);\
      if (c < 128) {\
        utftext += String.fromCharCode(c);\
      }else if((c > 127)&&(c < 2048)) {\
        utftext += String.fromCharCode((c >> 6) | 192);\
        utftext += String.fromCharCode((c & 63) | 128);\
      }else{\
         utftext += String.fromCharCode((c >> 12) | 224);\
         utftext += String.fromCharCode(((c >> 6) & 63) | 128);\
         utftext += String.fromCharCode((c & 63) | 128);\
      }\
    }\
    return utftext;\
  };\
  var blockstart;\
  var i, j;\
  var W = new Array(80);\
  var H0 = 0x67452301;\
  var H1 = 0xEFCDAB89;\
  var H2 = 0x98BADCFE;\
  var H3 = 0x10325476;\
  var H4 = 0xC3D2E1F0;\
  var A,B,C,D,E;\
  var temp;\
  msg = Utf8Encode(msg);\
  var msg_len = msg.length;\
  var word_array = new Array();\
  for( i=0; i<msg_len-3; i+=4 ) {\
    j = msg.charCodeAt(i)<<24 | msg.charCodeAt(i+1)<<16 | msg.charCodeAt(i+2)<<8 | msg.charCodeAt(i+3);\
    word_array.push(j);\
  }\
  switch(msg_len %% 4){\
    case 0:\
      i = 0x080000000;\
      break;\
    case 1:\
      i = msg.charCodeAt(msg_len-1)<<24 | 0x0800000;\
      break;\
    case 2:\
      i = msg.charCodeAt(msg_len-2)<<24 | msg.charCodeAt(msg_len-1)<<16 | 0x08000;\
      break;\
    case 3:\
      i = msg.charCodeAt(msg_len-3)<<24 | msg.charCodeAt(msg_len-2)<<16 | msg.charCodeAt(msg_len-1)<<8 | 0x80;\
      break;\
  }\
  word_array.push(i);\
  while( (word_array.length %% 16) != 14) word_array.push(0);\
  word_array.push(msg_len>>>29);\
  word_array.push((msg_len<<3)&0x0ffffffff);\
  for ( blockstart=0; blockstart<word_array.length; blockstart+=16 ){\
    for( i=0; i<16; i++ ) W[i] = word_array[blockstart+i];\
    for( i=16; i<=79; i++ ) W[i] = rotate_left(W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16],1);\
    A = H0;\
    B = H1;\
    C = H2;\
    D = H3;\
    E = H4;\
    for( i= 0; i<=19; i++){\
      temp = (rotate_left(A,5) + ((B&C) | (~B&D)) + E + W[i] + 0x5A827999) & 0x0ffffffff;\
      E = D;\
      D = C;\
      C = rotate_left(B,30);\
      B = A;\
      A = temp;\
    }\
    for( i=20; i<=39; i++ ){\
      temp = (rotate_left(A,5) + (B^C^D) + E + W[i] + 0x6ED9EBA1) & 0x0ffffffff;\
      E = D;\
      D = C;\
      C = rotate_left(B,30);\
      B = A;\
      A = temp;\
    }\
    for( i=40; i<=59; i++ ){\
      temp = (rotate_left(A,5) + ((B&C) | (B&D) | (C&D)) + E + W[i] + 0x8F1BBCDC) & 0x0ffffffff;\
      E = D;\
      D = C;\
      C = rotate_left(B,30);\
      B = A;\
      A = temp;\
    }\
    for( i=60; i<=79; i++ ){\
      temp = (rotate_left(A,5) + (B^C^D) + E + W[i] + 0xCA62C1D6) & 0x0ffffffff;\
      E = D;\
      D = C;\
      C = rotate_left(B,30);\
      B = A;\
      A = temp;\
    }\
    H0 = (H0 + A) & 0x0ffffffff;\
    H1 = (H1 + B) & 0x0ffffffff;\
    H2 = (H2 + C) & 0x0ffffffff;\
    H3 = (H3 + D) & 0x0ffffffff;\
    H4 = (H4 + E) & 0x0ffffffff;\
  }\
  var temp = cvt_hex(H0) + cvt_hex(H1) + cvt_hex(H2) + cvt_hex(H3) + cvt_hex(H4);\
  return temp.toLowerCase();\
}"
  );
  AR488srv->send(200, "text/html", html);
}


/***** Set WiFi parameters *****/
void setWifi() {
  char *APp = (char*)&AP;
  uint8_t chkAPb;
  uint8_t chkAPa;
  uint16_t apsz = sizeof(AP);

  // Checksum current config
  chkAPb = chkSumCfg(APp, apsz);
#ifdef DEBUG_2  
  Serial.println(F("Received WiFi config page..."));
  showArgs();
#endif

  AP.wmode = AR488srv->arg("mode").toInt();
  AR488srv->arg("essid").toCharArray(AP.essid, 20);
  if (AR488srv->arg("pword")!="") AR488srv->arg("pword").toCharArray(AP.pwd, 20);

  // Get IP address
  if (AR488srv->arg("addr1")!="") {
    AP.addr[0] = AR488srv->arg("addr1").toInt();
    AP.addr[1] = AR488srv->arg("addr2").toInt();
    AP.addr[2] = AR488srv->arg("addr3").toInt();
    AP.addr[3] = AR488srv->arg("addr4").toInt();
  }

  // For AP mode, gateway = IP address
  if (AP.wmode==0) {
    if (AR488srv->arg("gate1")) {
      AP.gate[0] = AR488srv->arg("addr1").toInt();
      AP.gate[1] = AR488srv->arg("addr2").toInt();
      AP.gate[2] = AR488srv->arg("addr3").toInt();
      AP.gate[3] = AR488srv->arg("addr4").toInt();
    }
  }

  // For static IP we need the gateway and mask as well
  if (AP.wmode == 1) {
    if (AR488srv->arg("gate1")) {
      AP.gate[0] = AR488srv->arg("gate1").toInt();
      AP.gate[1] = AR488srv->arg("gate2").toInt();
      AP.gate[2] = AR488srv->arg("gate3").toInt();
      AP.gate[3] = AR488srv->arg("gate4").toInt();
    }
    if (AR488srv->arg("mask1")) {
      AP.mask[0] = AR488srv->arg("mask1").toInt();
      AP.mask[1] = AR488srv->arg("mask2").toInt();
      AP.mask[2] = AR488srv->arg("mask3").toInt();
      AP.mask[3] = AR488srv->arg("mask4").toInt();
    }
  }

  // Has the configuration changed?
  chkAPa = chkSumCfg(APp, apsz);

  if (chkAPb==chkAPa) {
#ifdef DEBUG_2
    Serial.println(F("No change."));
#endif
  }else{
#ifdef DEBUG2    
    Serial.println(F("Config updated."));
#endif
// Set config indicator
    AP.cflg = 0x1C;
    startWifi();
  }
  // Go to status page
  wwwMain();
}


/***** Set general configuration options *****/
void setGen() {
  uint16_t webp;
  uint16_t gpibp;
  bool sc = false; // State change

  // Checksum current config
//  chkAPb = chkSumCfg(APp, apsz);

#ifdef DEBUG_2
  Serial.println(F("Received general config page..."));
  showArgs();
#endif

  webp = AR488srv->arg("webp").toInt();
  if (webp != AP.webp) {
    AP.webp = webp;
    delete AR488srv;
    AR488srv = new ESP8266WebServer(webp);
    startWebServer();

//    ESP.reset();
  }

  if (AR488srv->arg("pass")=="on") {
    Serial.println(F("Passthrough turned on."));
    if (!AP.gpib) sc = true;
    AP.gpib = true;
    gpibp = AR488srv->arg("gpibp").toInt();
    if (AP.gpibp != gpibp) {
      sc = true;
      AP.gpibp = gpibp;
    }
    if (sc) {
      Serial.print(F("Restarting TCP server on port: "));
      Serial.println(AP.gpibp);
      delete passSrv;
      passSrv = new WiFiServer(gpibp);
      passSrv->begin();
    }
  }else{
    if (AP.gpib) {
      AP.gpib = false;
      // Disconnect any client
      if (passCli.connected()) passCli.stop();
      // Kill the server (note: causes reboot!)
      passSrv->stop();
//      delete passSrv;
    }
  }
  wwwMain();  
}


/***** Configure AR488 parameters *****/
void set488(){
  bool ctrlr = false;

#ifdef DEBUG_2  
  Serial.println(F("Received GPIB config page..."));
  showArgs();
#endif

  if (AR488srv->arg("cmode")=="on") ctrlr = true;

  // Process switches
  if (AR488srv->arg("eoi")=="on") {
    Serial.println(F("++eoi 1"));
  }else{
    Serial.println(F("++eoi 0"));
  }
  flushIncoming();
  if (AR488srv->arg("eot")=="on") {
    Serial.println(F("++eot_enable 1"));
  }else{
    Serial.println(F("++eot_enable 0"));
  }
  flushIncoming();
  delay(200);

  // Process parameters
  Serial.print(F("++addr "));Serial.println(AR488srv->arg("gpib"));
  flushIncoming();
  Serial.print(F("++eos "));Serial.println(AR488srv->arg("eosch"));
  flushIncoming();
  Serial.print(F("++eot_char "));Serial.println(AR488srv->arg("eotch"));
  flushIncoming();
  Serial.print(F("++setvstr "));Serial.println(AR488srv->arg("vstr"));
  flushIncoming();

  // Set these only of controller mode is on
  if (ctrlr) {
    Serial.print(F("++auto "));Serial.println(AR488srv->arg("auto"));
    flushIncoming();
    Serial.print(F("++read_tmo_ms "));Serial.println(AR488srv->arg("readtmo"));
    flushIncoming();
  }
  delay(200);

  // Mode switch
  if (ctrlr) {
    Serial.println(F("++mode 1"));
  }else{
    Serial.println(F("++mode 0"));
  }
  flushIncoming();
  delay(200);

  // Save configuration
  if (AR488srv->arg("opt")=="2") {
    Serial.println(F("++savecfg"));
  }
  delay(200);
    
  // Go to status page
  wwwMain();

}


/***** Reboot WiFi *****/
void admin() {
  uint8_t opt = 0;
  char hash[20];

#ifdef DEBUG_2
  Serial.println(F("Received admin page..."));
  showArgs();
#endif

  opt = AR488srv->arg("opt").toInt();
  AR488srv->arg("chk1").toCharArray(hash,20);
    if (strncmp(hash,AP.pwdChk,20) == 0) {
      switch (opt) {
        case 1:
          break;
        case 2:
          Serial.println(F("Resetting to defaults..."));
          setWifiDefault();
        case 3:
          Serial.println(F("Rebooting..."));
          redirect(10, "Success!");
          delay(500);
          ESP.restart();
          break;
      }
    }else{
      redirect(3, "Fail - check password!");
    }
  // Go to status page
  wwwMain();
 }


/***** Redirect to status page*****/
void redirect(uint8_t dly, const char *msg){
  char pfx[6] = {0x68,0x74,0x74,0x70,0x73,0x0};
  IPAddress addr;

  if (!AP.ssl) strncpy(pfx,"http\0",5);
  if (AP.wmode==0) addr = WiFi.softAPIP();
  if (AP.wmode>0)  addr = WiFi.localIP();

  snprintf(html, htmlSize, 
"<html>\
<head>\
<meta http-equiv=\"refresh\" content=\"%d; URL='%s://%d.%d.%d.%d'\"/>\
</head>\
<body>\
<p>%s</p>\
<p>If this page does not re-direct you to the status page<br>\
 within a few seconds, please check your WiFi connection<br>\
 or try the back button in your browser.</p>\
</body>\
</html>",
  dly,
  pfx,
  addr[0],
  addr[1],
  addr[2],
  addr[3],
  msg
    );
  AR488srv->send(200, "text/html", html);
}
 


#ifdef DEBUG_2
/***** Display submitted arguments *****/
void showArgs(){
  uint8_t acnt = AR488srv->args();
  for (uint8_t i=0; i<acnt; i++){
    Serial.print(AR488srv->argName(i)+":\t");
    Serial.println(AR488srv->arg(i));
  }
}
#endif


/***** Generate 8 bit checksum *****/
uint8_t chkSumCfg(char *cfg, uint8_t len){
  uint8_t chk;
  long int sum = 0;

  for (int i=0; i<len; i++) {
    sum += cfg[i];
  }
  chk = sum & 255;
  return chk;
}


/***** Is the reply what we expected? *****/
uint8_t getReply(const char* cmd, char *reply, int rsize) {
  uint8_t p = 0;

  // Clear the reply buffer
  memset(reply, '\0', rsize);

  // Send cmd to controller and get response
  Serial.println(cmd);
  p = Serial.readBytesUntil(LF, reply, rsize-1);
  return p;
}


/***** Flush the incoming buffer *****/
void flushIncoming(){
  while (Serial.available()) { Serial.read(); };
}
