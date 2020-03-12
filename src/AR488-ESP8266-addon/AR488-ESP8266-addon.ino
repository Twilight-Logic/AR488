/*
 * AR488 ESP8266 WiFi Module
 * 
 * John Chajecki, @August 2019
 * Licenced under GPL 3.0
 * 
 * WiFi module for the AR488 GPIB to USB adapter
 * using the ESP8266 NodeMCU board
 * 
 * Requires crypto library by Rhys Wetherley
 * See Tools | Manage Libraries
 */

/*
//#define _DISABLE_TLS_
*/

#include <EEPROM.h>
#include <ESP8266WiFi.h>
//#include <Crypto.h>
//#include <SHA256.h>

#define VERSION "0.04.11" // 25-02-2020


//#define DISABLE_SSL

#ifdef DISABLE_SSL
  #include <ESP8266WebServer.h>
  ESP8266WebServer AR488srv(80);
#else
  #include <ESP8266WebServerSecure.h>
static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIB6jCCAVMCFEhQVUoACz86dEhgIysUeu6NaYBAMA0GCSqGSIb3DQEBCwUAMDQx
HDAaBgNVBAoME0FSNDg4X0VTUDgyNjZfQURET04xFDASBgNVBAMMC2FyNDg4Lmxv
Y2FsMB4XDTIwMDIyNTE1NDUzMloXDTQ2MDIxNjE1NDUzMlowNDEcMBoGA1UECgwT
QVI0ODhfRVNQODI2Nl9BRERPTjEUMBIGA1UEAwwLYXI0ODgubG9jYWwwgZ8wDQYJ
KoZIhvcNAQEBBQADgY0AMIGJAoGBAM2mgDoIcHccM991vWvq6hd+fOm/5zjECrbU
04jV3qR5E+9TMRps2NkuDghNbHLIo3cBm15WvgVPNjHI8hK36gymMWKH/QoVYap9
lkQyE0gFsC77UlSMDRLAcUmAsI0qsI2X0W5py5P6eV/XMFoDf9s1rN6MrcME1Tqg
S2TZomrzAgMBAAEwDQYJKoZIhvcNAQELBQADgYEAk+PvSU2Q11FS7lhHuNEnmBoc
DNNGwl7bxR13GlYlapTsT3mkQb+YK06O8Ygvs240avZfJ/+0YX7A2eDVWZb27Gxq
V//FM5w/AT4SQydD0X+4fuUVXC05Jk7zjBgoKP+EWGBmJlOY0/9RyTCBR1MCMONC
V05pZpLtOCqP0R4Unfk=
-----END CERTIFICATE-----
)EOF";
static const char serverKey[] PROGMEM =  R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIICXAIBAAKBgQDNpoA6CHB3HDPfdb1r6uoXfnzpv+c4xAq21NOI1d6keRPvUzEa
bNjZLg4ITWxyyKN3AZteVr4FTzYxyPISt+oMpjFih/0KFWGqfZZEMhNIBbAu+1JU
jA0SwHFJgLCNKrCNl9FuacuT+nlf1zBaA3/bNazejK3DBNU6oEtk2aJq8wIDAQAB
AoGASh/HfCNqliQeNl8z5WAI/GrlD0PSTc2rpvbTQDEGTPYvT0YHHuU4zB8jg66U
dlMHSG/V1o0hdLx2Zj9tsX2E6GCnrmSgydlK67EOltjMIhXzdQU5fMZh3Z62tbCl
rx3PJSvS9zHgSv0fqaRIfWinvdkHkhLLy0eujRvjwKiFqxECQQDxdtIZdOqwCS01
o5Tn0cZupFCHFp3//07SNK32ddp6jscPtb13MqVK0x+DLeeRL/e/ih91HrWUyfJQ
1V9ca8PXAkEA2gfCFc3o+EUqorf4DwerI7heET1GDodRLazQLFHOSRDbzkSJeAhP
MDHvvf9Z1VTTLOc3OyDhDbUuk+aB2wouRQJBANEjJAhOfrLnz9OEkoVS6TaNdP38
Ne8zd1aTsBUmsIu0PG+77pVeNVIgmEurJ8VVsZShkmuDpKxxWUJv/L90y1MCQB6s
YL24jkH/WRQzVY6nwYfw9Crwt95rYxxN56uUhrT4zjOiqMReRuW84MGWHwlIOb1j
ef2zjnXjO35LO5GZSRECQFGirtquoY76VE0/a2eccACzllXi+kKIM9Lpmkpk5tl3
q0nU1TGSF3zN4hdWR07e7B22Qkg1UT9vQKCDbkYTI4Q=
-----END RSA PRIVATE KEY-----
)EOF";
  BearSSL::ESP8266WebServerSecure AR488srv(443);
#endif

// EEPROM size and start address
#define EESIZE 512
#define EESTART 2

// Uncomment this to clear the EEPROM, then comment out again
//#define EEPROM_CLEAR

// Hash and salt sizes
#define HSZ 32
#define SSZ 16

// Randomzer to use
#if defined(ESP8266)
  #define RNG_WORD ESP8266_DREG(0x20E44)
#elif defined(ESP32)
  #define RNG_WORD esp_random()
#endif

// Terminators
#define CR 0x13
#define LF 0x10


/***** Enable debug modes *****/
//#define DEBUG_0   // Setup and loop
//#define DEBUG_1   // WiFi functions
//#define DEBUG_2   // Webserver/telnet functions (General tab)
//#define DEBUG_3   // EEPROM save
//#define DEBUG_4   // AR488 config tab
//#define DEBUG_5   // Admin tab
//#define DEBUG_6   // TCP->serial passthrough

//#define EEPROM_CLEAR  // Enable this to clear the EEPROM

/***** Parameter variables *****/
#define BAUD 115200		// serial baud rate


/***** Default WiFi AP mode config *****/
const char *dfltSSID = "AR488wifi";
const char *dfltPwd = "AR488-setup";


/***** Parameters saved in EEPROM *****/
union cfgObj {
  struct {
    uint16_t webp;      // Web server port
    uint16_t gpibp;     // GPIB passthrough port
    bool ssl;           // SSL enabled switch state
    bool gpib;          // GPIB passthrough enabled switch state
    bool wclient;       // WiFi station (client) mode enabled
    bool dhcp;          // Station mode DHCP enabled
    uint8_t addr[4];    // IP address
    uint8_t gate[4];    // IP gateway
    uint8_t mask[4];    // Subnet mask
    char pwdS[SSZ];     // Password salt
    uint8_t pwdH[HSZ];  // Password hash
  };
  uint8_t db[SSZ+HSZ+20];
};
union cfgObj AP;


/***** Server and client objects *****/

WiFiServer *passSrv = new WiFiServer(8488);
WiFiClient passCli;


/***** Buffers *****/
// HTML page buffer
const uint16_t htmlSize = 4096;
char html[htmlSize];
// Serial buffer
const uint16_t sbSize = 64;
uint8_t sBuf[sbSize];
uint16_t sbPtr = 0;


/***** Hash objects *****/
//SHA256 sha256;
//Hash *hash = &sha256;


/***** Other misc items *****/
// serial timout (milliseconds) to wait before sending TCP packet
uint8_t pktTmo = 5;       
// Current (last) web page sent
char curPage[10] = {'\0'};



/*********************/
/***** Web Pages *****/
/*********************/


/***** Main page *****/
static const char wwwMainPage[] PROGMEM = R"EOF(
<html>
<head>
<meta charset="utf-8">
<title>AR488 WiFi Configuration</title>
<link rel="stylesheet" href="style">
<script defer src="/script"></script>
<script defer src="/cfgGenjs"></script>
<script defer src="/cfgWiFijs"></script>
<script defer src="/cfg488js"></script>
<script defer src="/cfgAdmjs"></script>
<script defer src="/SHA1js"></script>
</head>
<body onload="getPage('%s')";>
<div class="mpage">
<div class="headr">AR488 WiFi Configuration</div>
<div>
<ul>
<li><a id='seeStat' onclick="getPage('seeStat')">Status</a></li>
<li><a id='cfgGen' onclick="getPage('cfgGen')">General</a></li>
<li><a id='cfgWiFi' onclick="getPage('cfgWifi')">WiFi</a></li>
<li><a id='cfg488' onclick="getPage('cfg488')">AR488</a></li>
<li><a id='cfgAdm' onclick="getPage('cfgAdm')">Admin</a></li>
</ul>
</div>
<hr>
<div id="cfgPage" class="conf">
Loading...
</div>
<div>
<hr>
<table class="foot"><tr>
<td>AR488 WiFi ver: %s</td>
<td>Firmware ver: %s</td>
</tr></table>
</div>
</div>
</body>
</html>
)EOF";


/***** Status page *****/
static const char seeStatPage[] PROGMEM = R"EOF(
<table>
<tr><th>WiFi mode:</th><td>%s</td></tr>
<tr><th>WiFi status:</th><td>%s</td></tr>
<tr %s><th>DHCP:</th><td>%s</td></tr>
<tr><th>IP address:</th><td>%d.%d.%d.%d</td></tr>
<tr %s><th>Gateway:</th><td>%d.%d.%d.%d</td></tr>
<tr %s><th>Netmask:</th><td>%d.%d.%d.%d</td></tr>
<tr><th>Mac addr:</th><td>%s</td></tr>
<tr><th>ESSID:</th><td>%s</td></tr>
<tr><th>GPIB pass:</th><td>%s</td></tr>
<tr><th>GPIB mode:</th><td>%d</td></tr>
<tr><th>GPIB addr:</th><td>%d</td></tr>
</table>
)EOF";


/***** General configuration page *****/
static const char cfgGenPage[] PROGMEM = R"EOF(
<form method="post" action="/setGen" onchange="enableButton(0x1);"/>
<table>
<tr><th>GPIB Pass:</th><td>Off</td>
<td><label class="switch">
  <input name="pass" type="checkbox" onclick="togglePass(this);" %s/>
  <span class="slider"></span>
</label></td>
<td>On</td></tr>
<tr><th>HTTP Port:</th><td colspan=3><input name="webp" value="%d"/></td></tr>
<tr><th>GPIB Port:</th><td colspan=3><input name="gpibp" value="%d" %s/></td></tr>
</table>
<input type="button" class="btn" id="btnApply" value="Apply" onclick="genValidate()" disabled/>
</form>
)EOF";


/*
<tr><th>SSL:</th><td>Off</td>
<td><label class="switch">
  <input name="ssl" type="checkbox" onclick="toggleSSL(this);"/>
  <span class="slider"></span>
</label></td>
<td>On</td></tr>
*/


/***** WiFi configuration page *****/
static const char cfgWifiPage[] PROGMEM = R"EOF(
<form method='post' action='/setWifi' onchange='enableButton(0x1);'/>
<table>
<tr><th>WiFi Mode:</th><td>AP</td>
<td><label class='switch'>
  <input name='chkM' type='checkbox' onchange='wifiMode();' %s/>
  <span class='slider'></span>
</label></td>
<td>Client</td></tr>
<tr><th>DHCP:</th><td>Off</td>
<td><label class='switch'>
  <input name='chkD' type='checkbox' onchange='wifiMode();' %s %s/>
  <span class='slider'></span>
</label></td>
<td>On</td></tr>
</table>
<table>
<tr><th>ESSID:</th><td><input name='essid' value='%s'/></td></tr>
<tr><th>Passkey:</th><td><input name='pword' type='password' value=''/></td></tr>
<tr><th>IP address:</th><td>
<input class='ip' name='addr1' value='%d' onchange='octAuto(this);' %s/>.
<input class='ip' name='addr2' value='%d' onchange='octAuto(this);' %s/>.
<input class='ip' name='addr3' value='%d' onchange='octAuto(this);' %s/>.
<input class='ip' name='addr4' value='%d' %s/>
</td></tr>
<tr><th>IP gateway:</th><td>
<input class='ip' name='gate1' value='%d' %s/>.
<input class='ip' name='gate2' value='%d' %s/>.
<input class='ip' name='gate3' value='%d' %s/>.
<input class='ip' name='gate4' value='%d' %s/>
</td></tr>
<tr><th>IP netmask:</th><td>
<input class='ip' name='mask1' value='%d' %s/>.
<input class='ip' name='mask2' value='%d' %s/>.
<input class='ip' name='mask3' value='%d' %s/>.
<input class='ip' name='mask4' value='%d' %s/>
</td></tr>
</table>
<input type="button" class="btn" id="btnApply" onclick="wifiValidate()" value="Apply" disabled/>
</form>
)EOF";


/***** AR488 Configuration Page *****/
static const char cfg488Page[] PROGMEM = R"EOF(
<form method="post" action="/set488" onchange="enableButton(0x3)"/>
<input name="opt" value="0" type="hidden"/>
<table>
<tr><th>Mode:</th><td>Device</td>
<td><label class="switch">
  <input name="cmode" type="checkbox" onchange="toggleCmode(this);" %s/>
  <span class="slider"></span>
</label></td>
<td>Controller</td></tr>
<tr><th>GPIB address:</th><td colspan="3"><input name="gpib" value="%d"/></td></tr>
<tr><th>Auto:</th><td colspan="3"><input name="auto" value="%d" %s/></td></tr>
<tr><th>Read timeout:</th><td colspan="3"><input name="readtmo" value="%d" %s/></td></tr>
<tr><th>EOI:</th><td>Disabled</td>
<td><label class="switch">
  <input name="eoi" type="checkbox" %s/>
  <span class="slider"></span>
</label></td>
<td>Enabled</td></tr>
<tr><th>EOS char:</th><td colspan="3"><input name="eosch" value="%d"/></td></tr>
<tr><th>EOT Enable:</th><td>Disabled</td>
<td><label class="switch" %s>
  <input name="eot" type="checkbox"/>
  <span class="slider"></span>
</label></td>
<td>Enabled</td></tr>
<tr><th>EOT char:</th><td colspan="3"><input name="eotch" value="%d"/></td></tr>
<tr><th>Version string:</th><td colspan="3"><input name="vstr" value="%s"/></td></tr>
</table>
<input type="submit" class="btn" value="Apply" id="btnApply" onclick="apply488Cfg();" disabled/>
<input type="submit" class="btn" value="Save" id="btnSave" onclick="save488Cfg();" disabled/>
</form>
)EOF";


/***** Admin functions page *****/
static const char cfgAdmPage[] PROGMEM = R"EOF(
<form method="post" id="frmAdm" action="/admin">
<input name="chk1" value="" type="hidden"/>
<input name="chk2" value="" type="hidden"/>
<input name="opt" value=0 type="hidden"/>
<table>
<tr><th>Current password:</th><td><input id="pwd1" type="password" onchange="enableButton(0x1C);"/></td>
<tr><th>New password:</th><td><input id="pwd2" type="password"/></td>
<tr><th>Confirm new password:</th><td><input id="pwd3" type="password"/></td>
<tr><th></th><td align="right"><input type="button" id="btnChpwd" class="btn" value="Change" onclick="chgPwd();" disabled/></td></tr>
<tr><th></th><td><br></td></tr>
<tr><th>Defaults</th><td align="right"><input type="button" id="btnRst" class="btn" value="Reset" onclick="rstDflt();" disabled/></td></tr>
<tr><th>Restart WiFi</th><td align="right"><input type="button" class="btn" id="btnRbt" value="Reboot" onclick="reboot();" disabled/></td></tr>
</table>
</form>
)EOF";


/***** CSS definition *****/
static const char css[] PROGMEM = R"EOF(
body {width: 320; height: 424;}
table {width: 100%%; font-size: 10pt; text-align: left;}
th  {width: 120px;}
ul  {list-style-type: none;
  margin: 0;
  padding: 0;
  height: 24px;
  font-size: 10pt;
  text-align: center;}
li  {float: left;width:20%;}
li a  {display: block;
  border: 1px solid #777777;
  color: #333333;
  background-color: #FFEE77;
  text-decoration: none;}
li a:hover:not(.active) {
  background-color: #777777;
  color: cyan;}
.active {background-color:#77CCFF;}
.headr  {color: #0000AA;
  font-size: 14pt;
  font-weight: bold;
  width: 320;
  text-align: center;
  padding: 10px 0px 10px 0px;}
.foot {font-size: 8pt;}
.mpage  {width: 320;
  height: 424;
  background-color: #EEEEEE;
  font-family: verdana;
  border: 1px solid #777777;
  margin: 7px;
  padding:7px;
  box-shadow: 3px 3px 3px #AAAAAA;
}
.conf {overflow-y: auto;
  height: 300px;
}
.ip {width: 40px;}
.switch {
  position: relative;
  display: inline-block;
  width: 42px;
  height: 20px;
}
.slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #2196F3;
}
.slider:before {
  position: absolute;
  content: '';
  height: 16px;
  width: 16px;
  left: 2px;
  bottom: 2px;
  background-color: silver;
}
input:checked + .slider:before {
  -webkit-transform: translateX(22px);
  -ms-transform: translateX(22px);
  transform: translateX(22px);
  background-color: gold;
}
.btn {float: right;
  width: 90px;
  margin: 10px 5px 0px 0px;
}
)EOF";


/***** Main linking JavaScript code *****/
static const char lnkScriptJs[] PROGMEM = R"EOF(
var curMnObj='';
function getPage(url) {
  var cfgPage = document.querySelector('#cfgPage');
  var curPage;
  var obj;
  if (url=='txtErr'){
    alert("SSID/password is empty or too long!");
    url='cfgWifi';
  };
  if (url) {
    curPage = '#'+url;
  }else{
    curPage = '#seeStat';
    url = 'seeStat';
  }
  obj = document.querySelector(curPage);
  if (curMnObj) curMnObj.classList.remove('active');
  curMnObj = obj;
  curMnObj.classList.add('active');
  cfgPage.innerHTML = "Updating...";
  fetch(url, {method: "POST"} )
    .then((resp) => resp.text())
    .then(function(data){
        cfgPage.innerHTML = data;
    })
    .catch(function(error){
      alert("Unable to load page!");
    });
  if (url=='cfgWiFi') wifiMode();
}
function enableButton(sw){
  if (sw&0x1) document.querySelector('#btnApply').disabled = false;
  if (sw&0x2) document.querySelector('#btnSave').disabled = false;
  if (sw&0x4) document.querySelector('#btnChpwd').disabled = false;
  if (sw&0x8) document.querySelector('#btnRst').disabled = false;
  if (sw&0x10) document.querySelector('#btnRbt').disabled = false;
}
)EOF";


/***** General configuration page JavaScript *****/
static const char cfgGenJs[] PROGMEM = R"EOF(
function toggleSSL(ssl){
  var webp = document.querySelector('[name="webp"]');
  if (ssl.checked && webp.value == '80') webp.value = 443;
  if (ssl.checked==false && webp.value == '443') webp.value = 80;
}
function togglePass(pass){
  var gpibp = document.querySelector('[name="gpibp"]');
  if (pass.checked) {
    gpibp.disabled = false;
  }else{
    gpibp.disabled = true;
  }
}
function genValidate() {
  var form = document.querySelector('form');
  var adP = document.querySelector('[name="webp"]').value;
  var gpP = document.querySelector('[name="gpibp"]').value;
  if (adP==0||gpP==0 || adP>49151||gpP>49151) {
    alert("TCP port is out of range!\\nPlease set a port value between 1 and 49151.");
  }else{
    form.submit();
  }
}
)EOF";


/***** WiFi configuration page JavaScript *****/
static const char cfgWifiJs[] PROGMEM = R"EOF(
function wifiMode(){
  var chkm = document.querySelector('[name="chkM"]');
  var chkd = document.querySelector('[name="chkD"]');
  var dhcp = document.querySelector('[name="dhcp"]');
  var octets = document.querySelectorAll('.ip');
  var octlen = octets.length;
  var i;
  if (chkm.checked){
    if (chkd.checked){
      setOctView(octets,0,octlen,true);
    }else{
      chkd.disabled = false;
      setOctView(octets,0,octlen,false);
    }
    wmod.value = 1;
  }else{
    chkd.checked = false;
    chkd.disabled = true;
    setOctView(octets,0,4,false);
    setOctView(octets,4,octlen,true);
  }
}
function setOctView(octObj,s,e,stat){
  for (i=s; i<e; i++){
    octObj[i].disabled = stat;
  }
}
function wifiValidate(){
  var form = document.querySelector('form');
  var essid = document.querySelector('[name="essid"]').value;
  var pword = document.querySelector('[name="pword"]').value;
  var err = 0;
  if (essid=='') {
    alert('WiFi ESSID required!');
    err++;
  }
  if (pword.length<8 || pword.length>63) {
    alert('Password must be between 8 and 63 characters long!');
    err++;
  }
  if (err==0 && octOk()) form.submit();
}
function octOk(){
  var octets = document.querySelectorAll('.ip');
  var chkm = document.querySelector('[name="chkM"]');
  var chkd = document.querySelector('[name="chkD"]');
  var i=0;
  var t=4;
  if (chkm.checked) {
    if (chkd.checked) {return true};
    t=octets.length;
  }else{
    t=4;
  }
  for (i=0; i<t; i++){
    if (octets[i].value<0 || octets[i].value>255) {
      alert('Invalid IP address!');
      return false;
    }
  }
  return true;
}
function octAuto(obj){
  var chkm = document.querySelector('[name="chkM"]');
  var octets = document.querySelectorAll('.ip');
  if (chkm.checked) {
    for (i=0; i<12; i++) {
      if (obj == octets[i]) octets[i+4].value = octets[i].value;
    }
  }
}
)EOF";


/***** AR488 configuration page JavaScript *****/
static const char cfg488Js[] PROGMEM = R"EOF(
function toggleCmode(cmode){
  var auto = document.querySelector('input[name="auto"]');
  var rtmo = document.querySelector('input[name="readtmo"]');
  if (cmode.checked) {
  auto.disabled = false;
    rtmo.disabled = false;
  }else{
  auto.disabled = true;
    rtmo.disabled = true;
  }
}
function apply488Cfg(){
  document.querySelector('input[name="opt"]').value = 1;
  document.querySelector('form').submit();
}
function save488Cfg(){
  document.querySelector('input[name="opt"]').value = 2;
  document.querySelector('form').submit();
}
)EOF";


/***** Admin function page JavaScript *****/
static const char cfgAdmJs[] PROGMEM = R"EOF(
function chgPwd(msg){
  var pwd1 = document.querySelector('#pwd1').value;
  var pwd2 = document.querySelector('#pwd2').value;
  var pwd3 = document.querySelector('#pwd3').value;
  if (pwd1==""||pwd2==""||pwd3=="") {
    alert("Passwords cannot be blank!");
    return;
  }
  if (pwd1==(pwd2||pwd3)){
    alert("New password cannot be the same as the old password!");
    return;
  }
  if (pwd2==pwd3){
    runAdmReq(pwd1,pwd2,1);
  }else{
    alert("New passwords do not match!");
  }
}
function rstDflt(){
  var pwd1 = document.querySelector('#pwd1').value;
  if (pwd1){
    var res = confirm("The interface will be reset to default AP mode (ESSID: AR488wifi, IP address: 192.168.4.88).\\nAre you sure you wish to proceed?");
    if (res) runAdmReq(pwd1,"",2);
  }else{
    alert("Please confirm the current admin password!");
  }
}
function reboot(){
  var pwd1 = document.querySelector('#pwd1').value;
  if (pwd1){
    var res = confirm("The WiFi connection will be interrupted. Are you sure you wish to proceed?");
    if (res) runAdmReq(pwd1,"",3);
  }else{
    alert("Please confirm the current admin password!");
  }
}
function runAdmReq(pwd1, pwd2, act){
  var prm1 = SHA1(pwd1);
  var prm2 = SHA1(pwd2);
  var prm3 = Promise.resolve(act);
  Promise.all([prm1,prm2,prm3]).then(function(results){
    formObj = document.querySelector('form');
    var chk1 = document.querySelector('[name="chk1"]');
    var chk2 = document.querySelector('[name="chk2"]');
    var opt = document.querySelector('[name="opt"]');
    chk1.value = results[0];
    if (results[2]==1) chk2.value=results[1];
    opt.value=results[2];
    formObj.submit();
  });
}
)EOF";


/***** SHA1 function *****/
static const char SHA1funcJs[] PROGMEM = R"EOF(
function SHA1(msg){
  function rotate_left(n,s){
      var t4 = (n<<s) | (n>>>(32-s));
      return t4;
  };
  function lsb_hex(val) {
    var str='';
    var i;
    var vh;
    var vl;
    for( i=0; i<=6; i+=2 ){
      vh = (val>>>(i*4+4))&0x0f;
      vl = (val>>>(i*4))&0x0f;
      str += vh.toString(16) + vl.toString(16);
    }
    return str;
  };
  function cvt_hex(val){
    var str='';
    var i;
    var v;
    for( i=7; i>=0; i-- ) {
      v = (val>>>(i*4))&0x0f;
      str += v.toString(16);
    }
    return str;
  };
  function Utf8Encode(string) {
    var utftext = '';
    string = string.replace(/\\r\\n/g,"\\n");
    for (var n = 0; n < string.length; n++){
      var c = string.charCodeAt(n);
      if (c < 128) {
        utftext += String.fromCharCode(c);
      }else if((c > 127)&&(c < 2048)) {
        utftext += String.fromCharCode((c >> 6) | 192);
        utftext += String.fromCharCode((c & 63) | 128);
      }else{
         utftext += String.fromCharCode((c >> 12) | 224);
         utftext += String.fromCharCode(((c >> 6) & 63) | 128);
         utftext += String.fromCharCode((c & 63) | 128);
      }
    }
    return utftext;
  };
  var blockstart;
  var i, j;
  var W = new Array(80);
  var H0 = 0x67452301;
  var H1 = 0xEFCDAB89;
  var H2 = 0x98BADCFE;
  var H3 = 0x10325476;
  var H4 = 0xC3D2E1F0;
  var A,B,C,D,E;
  var temp;
  msg = Utf8Encode(msg);
  var msg_len = msg.length;
  var word_array = new Array();
  for( i=0; i<msg_len-3; i+=4 ) {
    j = msg.charCodeAt(i)<<24 | msg.charCodeAt(i+1)<<16 | msg.charCodeAt(i+2)<<8 | msg.charCodeAt(i+3);
    word_array.push(j);
  }
  switch(msg_len % 4){
    case 0:
      i = 0x080000000;
      break;
    case 1:
      i = msg.charCodeAt(msg_len-1)<<24 | 0x0800000;
      break;
    case 2:
      i = msg.charCodeAt(msg_len-2)<<24 | msg.charCodeAt(msg_len-1)<<16 | 0x08000;
      break;
    case 3:
      i = msg.charCodeAt(msg_len-3)<<24 | msg.charCodeAt(msg_len-2)<<16 | msg.charCodeAt(msg_len-1)<<8 | 0x80;
      break;
  }
  word_array.push(i);
  while( (word_array.length %% 16) != 14) word_array.push(0);
  word_array.push(msg_len>>>29);
  word_array.push((msg_len<<3)&0x0ffffffff);
  for ( blockstart=0; blockstart<word_array.length; blockstart+=16 ){
    for( i=0; i<16; i++ ) W[i] = word_array[blockstart+i];
    for( i=16; i<=79; i++ ) W[i] = rotate_left(W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16],1);
    A = H0;
    B = H1;
    C = H2;
    D = H3;
    E = H4;
    for( i= 0; i<=19; i++){
      temp = (rotate_left(A,5) + ((B&C) | (~B&D)) + E + W[i] + 0x5A827999) & 0x0ffffffff;
      E = D;
      D = C;
      C = rotate_left(B,30);
      B = A;
      A = temp;
    }
    for( i=20; i<=39; i++ ){
      temp = (rotate_left(A,5) + (B^C^D) + E + W[i] + 0x6ED9EBA1) & 0x0ffffffff;
      E = D;
      D = C;
      C = rotate_left(B,30);
      B = A;
      A = temp;
    }
    for( i=40; i<=59; i++ ){
      temp = (rotate_left(A,5) + ((B&C) | (B&D) | (C&D)) + E + W[i] + 0x8F1BBCDC) & 0x0ffffffff;
      E = D;
      D = C;
      C = rotate_left(B,30);
      B = A;
      A = temp;
    }
    for( i=60; i<=79; i++ ){
      temp = (rotate_left(A,5) + (B^C^D) + E + W[i] + 0xCA62C1D6) & 0x0ffffffff;
      E = D;
      D = C;
      C = rotate_left(B,30);
      B = A;
      A = temp;
    }
    H0 = (H0 + A) & 0x0ffffffff;
    H1 = (H1 + B) & 0x0ffffffff;
    H2 = (H2 + C) & 0x0ffffffff;
    H3 = (H3 + D) & 0x0ffffffff;
    H4 = (H4 + E) & 0x0ffffffff;
  }
  var temp = cvt_hex(H0) + cvt_hex(H1) + cvt_hex(H2) + cvt_hex(H3) + cvt_hex(H4);
  return temp.toLowerCase();
}
)EOF";


/***** Re-direct page *****/
static const char redirectPage[] PROGMEM = R"EOF(
<html>
<head>
<meta http-equiv="refresh" content="%d; URL="http%c://%d.%d.%d.%d:%d""/>
<style>
body {width: 320; height: 424;}
.mpage  {width: 320;
  height: 424;
  background-color: #EEEEEE;
  font-family: verdana;
  border: 1px solid #777777;
  margin: 7px;
  padding:7px;
  box-shadow: 3px 3px 3px #AAAAAA;
  font-family: verdana;
  font-size: 12pt;
  font-weight: normal;
}
</style>
</head>
<body>
<div class="mpage">
<p>%s</p>
<p>If you have switched to a DHCP assigned address, then manually
 enter the new IP address into your browswer.</p>
<p>If you have switched from Station to AP mode, or have restarted
 AP mode, then you may have to re-connect to the WiFi SSID and then
 enter the new IP address in your browser.</p>
<p>Otherwise, please wait a few seconds for the page to reload.<p>
<p>If the page does not re-load within a few seconds, please try
 the back button in your browser or check your WiFi connection.</p>
</div>
</body>
</html>
)EOF";



/**********************************/
/*****  MAIN PROGRAM  *****/
/**********************************/

void setup() {

#ifdef EEPROM_CLEAR
  epErase();
#endif

  Serial.begin(BAUD);

#ifdef DEBUG_0
  Serial.println();
  Serial.println(F("Starting WiFi..."));
#endif

#ifndef DISABLE_SSL
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
#endif

  // Set config defaults
  setDefaultCfg();

#ifdef DEBUG_0
  if (isEepromClear()){
    Serial.println(F("EEPROM is clear"));
  }else{
    Serial.println(F("EEPROM contains previous config"));
  }
#endif

  // Check if previous config saved and load
  if (!isEepromClear()) {
#ifdef DEBUG_0    
    Serial.println("Loading saved config...");
#endif
    // Load previous config else clear EEPROM
    if (!epReadData(EESTART, &AP, sizeof(AP))) {
      epErase();
#ifdef DEBUG_0
    Serial.println("EEPROM erased. Reset to defaults.");
#endif
    }  
  }

/*
  // Generate random salt
  if (!AP.pwdS[0]) {
    randomGen();
    Serial.print(F("\nSalt: "));Serial.println(AP.pwdS);
  }

  Serial.print(F("Hash: "));
  char data[] = "abcdefghijklmnopqrstuvwxyz";
  hashGen(data, 26);
  printHex(AP.pwdH, HSZ, UCASE);
*/

  // Start WiFi and Web server
  if (startWifi()>-1) {
#ifdef DEBUG_0
    Serial.println(F("Starting webserver..."));
#endif
    startWebServer();
    
    if (AP.gpib) {
      passSrv = new WiFiServer(AP.gpibp);
      passSrv->begin();
    }
  }

}


void loop() {

  // Handle requests for web server
  AR488srv.handleClient();

  // Is GPIB passthrough enabled?
  if (AP.gpib) {
    // Handle incoming client connections
    if (passCli.connected()) {

      // Handle TCP connection, output to serial
      while (passCli.available()) {
        if (sbPtr < sbSize - 1) {
          sBuf[sbPtr] = passCli.read();
          sbPtr++;
        }else{
          break;
        }
      }

      if (sbPtr) {
        Serial.write(sBuf, sbPtr);
        clrSBuf(sbPtr);
      }
        
      // Handle serial input, output to TCP
      while (Serial.available()) {
        if (sbPtr < sbSize - 1) {
          sBuf[sbPtr] = Serial.read();
          sbPtr++;
        }
      }
      passCli.write((char*)sBuf, sbPtr);
      clrSBuf(sbPtr);

#ifdef DEBUG_6
      if (!passCli.connected()) Serial.println("<= disconnected.");
#endif

    } else {
      // Wait for a connection
      passCli = passSrv->available();
      if (passCli.connected()) {
#ifdef DEBUG_6        
        Serial.println("Connected =>");
#endif
        // Initialise buffer
        clrSBuf(64);
        delay(50);
        // Clear spurious characters after connection established
        while (passCli.available()) { passCli.read(); }
      }
    }
  }
}


/*****************************/
/***** PROGRAM FUNCTIONS *****/
/*****************************/

/***** Default configuration *****/
void setDefaultCfg() {
#ifdef DISABLE_SSL
  AP = {80,8488,false,false,false,false,{192,168,4,88},{192,168,4,88},{255,255,255,0},{'\0'},{'\0'}};
#else
  AP = {443,8488,true,false,false,false,{192,168,4,88},{192,168,4,88},{255,255,255,0},{'\0'},{'\0'}};
#endif
}


/***** Start WiFi *****/
/*
 * Flag:
 * -1 : fail
 *  0 : fallback mode
 *  1 : Started in AP mode
 *  2 : Started in Client mode
 */
int startWifi() {
  int stat=-1;
  // Check if already configured
  if (isEepromClear()) {
    Serial.println(F("Starting AP with defaults..."));
    setDefaultCfg();
    startWifiAP(dfltSSID, dfltPwd, AP.addr, AP.addr, AP.mask);
    stat = 0;
  }else{
    // Start mode
    if (AP.wclient) {
      // Start station (client) mode
#ifdef DEBUG_1
      Serial.println(F("Starting WiFi client..."));
#endif
      if (startWifiStn("", "", AP.addr, AP.gate, AP.mask)) {
        stat=2;
      }else{
        // Fallback to default AP configuration
#ifdef DEBUG_1
        Serial.println(F("Falling back to AP with defaults..."));
#endif
        setDefaultCfg();
        startWifiAP(dfltSSID, dfltPwd, AP.addr, AP.addr, AP.mask);
        stat=0;
      }
    }else{
      // Start AP mode
      startWifiAP("","",AP.addr,AP.addr,AP.mask);
#ifdef DEBUG_1
      Serial.println(F("Starting AP..."));
#endif
      stat=1;
    }
  }
#ifdef DEBUG_1
  Serial.print(F("Wifi Start status: "));Serial.println(stat);
#endif
  return stat;
}


/***** Check whether WiFi has started *****/
bool hasWifiStarted() {
  uint16_t tmo = 60000;
  uint16_t dly = 500;

  // Wait for connection
#ifdef DEBUG_1
  Serial.print(F("Waiting for connection."));
#endif
  while ((WiFi.status() != WL_CONNECTED) && (tmo > 0)) {
    delay(dly);
    tmo = tmo - dly;
#ifdef DEBUG_1
    Serial.print(".");
#endif
  }
#ifdef DEBUG_1
  Serial.print(F("\nWiFi status:"));
  Serial.println(WiFi.status());
#endif

  if (WiFi.status() == WL_CONNECTED) {
    // Success!
#ifdef DEBUG_1
    Serial.println(F("WiFi started."));
    Serial.print(F("Connected, IP address: "));
    Serial.println(WiFi.localIP());
#endif
    return true;
  } else {
    // Timed out
#ifdef DEBUG_1
    Serial.println(F("WiFi failed."));
#endif
    return false;
  }
}


/***** Start WiFi an access point mode ****/
void startWifiAP(String ssid, String psks, uint8_t addr[4], uint8_t gate[4], uint8_t mask[4]) {
  uint8_t gway[4];
  bool gws = true;  // Set gateway to primary address (removes compiler warning)

  // If no SSID or password, then use the currently configure one
  if (ssid == NULL) ssid = WiFi.softAPSSID();
  if (psks == NULL) psks = WiFi.softAPPSK();

  // Stop and disable station mode
  if (WiFi.getMode()==WIFI_STA) {
    // Disconnect client from network
    WiFi.disconnect();
    WiFi.enableSTA(false);
  }

  // Gateway is primary address or separate address?
  for (uint8_t i=0; i<4; i++) {
    if (gws) {
      gway[i] = addr[i];  // Gateway set to primary address
    }else{
      gate[i] = gate[i];  // Gateway set to different IP address
    }
  }

  // Restart and configure the AP      
//  if (!WiFi.enableAP()) WiFi.enableAP(true);
  WiFi.enableAP(true);
  WiFi.mode(WIFI_AP);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.softAPConfig(addr, gway, mask); // set IP address
  WiFi.softAP(ssid, psks); // set SSID and password
  delay(500);
}


/***** Start WiFi in station mode *****/
bool startWifiStn(String ssid, String psks, uint8_t addr[4], uint8_t gate[4], uint8_t mask[4]) {

  if (ssid == NULL) ssid = WiFi.SSID();
  if (psks == NULL) psks = WiFi.psk();

#ifdef DEBUG_1
  Serial.println(F("Station mode>"));
#endif

  // Check whether in AP mode - is so, disconnect clients and turn off AP mode
  if (WiFi.getMode()==WIFI_AP) {
    // Disconnect clients and stop AP mode
    WiFi.softAPdisconnect(true);
    WiFi.enableAP(false);
    delay(500);
  }

  // Enable and start station mode
  WiFi.enableSTA(true);
  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  
  if (AP.dhcp) {
    // DHCP mode
    WiFi.config(0u, 0u, 0u);
  }else{
    // Static IP mode
//    wifi_station_dhcpc_stop();
    WiFi.config(addr, gate, mask);
  }

//  WiFi.setAutoConnect(true);
//  WiFi.setAutoReconnect(true);

  WiFi.begin(ssid, psks);
  return hasWifiStarted();
}


/***** Start the web server *****/
void startWebServer() {

#ifndef DISABLE_SSL
  AR488srv.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));
#endif

#ifdef DEBUG_2
  Serial.println(F("Configuring web pages..."));  
#endif

  // Pages being served
  AR488srv.on("/", wwwMain);         // Default web page
  AR488srv.on("/seeStat", seeStat);  // Status page
  AR488srv.on("/cfgGen", cfgGen);    // General options
  AR488srv.on("/cfgWifi", cfgWifi);  // WiFi config
  AR488srv.on("/cfg488", cfg488);    // AR488 config
  AR488srv.on("/cfgAdm", cfgAdm);    // Admin options
  AR488srv.on("/style", lnkStyle);   // Stylesheet
  AR488srv.on("/script", lnkScript);     // Main Script
  AR488srv.on("/cfgGenjs", cfgGenjs);    // General page Script
  AR488srv.on("/cfgWiFijs", cfgWiFijs);  // WiFi config Script
  AR488srv.on("/cfg488js", cfg488js);    // AR488 config Script
  AR488srv.on("/cfgAdmjs", cfgAdmjs);    // Admin Script
  AR488srv.on("/SHA1js", SHA1js);        // SHA1 Script

  // Argument handlers for pages
  AR488srv.on("/setWifi", setWifi);  // Set WiFi parameters
  AR488srv.on("/setGen", setGen);    // Set WiFi parameters
  AR488srv.on("/set488", set488);    // Set GPIB parameters
  AR488srv.on("/admin", admin);      // Set WiFi parameters

#ifdef DEBUG_2
  Serial.println(F("Initialising webserver..."));  
#endif
  // Start the server
  AR488srv.begin(AP.webp); // Start the HTTP server
#ifdef DEBUG_2
  Serial.println("Web server started.");
#endif
}


/***** Main web page *****/
void wwwMain() {
  uint8_t rsz = 0;
  char reply[64] = {'\0'};
  char fwver[10] = {'\0'};
  char *vp;

  rsz = getReply("++ver real", reply, 47);
  if (rsz > 0) {
    vp = strstr(reply, "ver.") + 5;
    for (int i = 0; i < 7; i++) {
      fwver[i] = *vp;
      vp++;
    }
  }

  snprintf(html, htmlSize, wwwMainPage, 
           curPage,
           VERSION,
           fwver
          );
  AR488srv.send(200, "text/html", html);
}


/***** Status page *****/
void seeStat() {
  char wmodestr[8] = {0x53,0x74,0x6F,0x70,0x70,0x65,0x64,0x0};
  char wstatstr[14] = {0x52,0x75,0x6E,0x6E,0x69,0x6E,0x67,0x0};
  char dhcp[9] = {0x44,0x69,0x73,0x61,0x62,0x6C,0x65,0x64,0x0};
  char hide[7] = {'\0'};
  char gpibpass[9] = {0x44,0x69,0x73,0x61,0x62,0x6C,0x65,0x64,0x0};
  char reply[64] = {'\0'};
  uint8_t rsz;
  uint8_t gpibmode = 255;
  uint8_t gpibaddr = 255;

  String ssid;
  uint8_t wmode = WiFi.getMode();
  int wstatus = WiFi.status();
  IPAddress addr = {0};
  IPAddress gate = {0};
  IPAddress mask = {0xFF,0xFF,0xFF,0x0};

  // AP mode
  if (wmode == 2) {
    strncpy(wmodestr, "Soft AP\0", 8);
    strncpy(hide, "hidden\0", 7);
    ssid = WiFi.softAPSSID();
    addr = WiFi.softAPIP();
  }

  // Station (client) mode
  if (wmode == 1) {
    strncpy(wmodestr, "Station\0", 8);
    ssid = WiFi.SSID();
    addr = WiFi.localIP();
    gate = WiFi.gatewayIP();
    mask = WiFi.subnetMask();
    if (AP.dhcp == true) strncpy(dhcp, "Enabled\0", 8);
    switch(wstatus) {
      case WL_CONNECTED:
          strncpy(wstatstr, "Connected\0", 14);
          break;
      case WL_CONNECTION_LOST:
          strncpy(wstatstr, "Conn. lost\0", 14);
          break;
      case WL_DISCONNECTED:
          strncpy(wstatstr, "Disconnected\0",14); 
          break;
      default:
          strncpy(wstatstr, "Conn. failed\0",14); 
    }
  }

  if (AP.gpib) strncpy(gpibpass, "Enabled\0", 8);

  flushIncoming();
  rsz = getReply("++mode", reply, 64);
  if (rsz) gpibmode = atoi(reply);
  rsz = getReply("++addr", reply, 64);
  if (rsz) gpibaddr = atoi(reply);

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
      // Blank last reply
      strncpy(reply, "n/a\0", 4);
    }
  */

  snprintf(html, htmlSize, seeStatPage,
           wmodestr,
           wstatstr,
           hide,
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
           ssid.c_str(),
           gpibpass,
           gpibmode,
           gpibaddr
          );  
//  AR488srv->send(200, "text/html", html);
  AR488srv.send(200, "text/html", html);
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
  } else {
    strncpy(disd, "disabled\0", 9);
  }

  snprintf(html, htmlSize, cfgGenPage,
           chkd2,
           AP.webp,
           AP.gpibp,
           disd
          );
//  AR488srv->send(200, "text/html", html);
  AR488srv.send(200, "text/html", html);
}


/***** WiFi configuration page *****/
/*
 * WiFi mode - 0: AP; 1: client-static; 2: client-dhcp;
 */
void cfgWifi() {
  char chkd1[8] = {'\0'};
  char chkd2[8] = {'\0'};
  char disc2[9] = {'\0'};
  char disd1[9] = {'\0'};
  char disd2[9] = {'\0'};

  String ssid;
  uint8_t wmode = WiFi.getMode();
  
  IPAddress addr = {0};
  IPAddress gate = {0};
  IPAddress mask = {0xFF,0xFF,0xFF,0x0};

  // Mode
  if (wmode==WIFI_AP) {
    // AP mode - disable DHCP switch
    strncpy(disc2, "disabled\0", 9);
    strncpy(disd2, "disabled\0", 9);
    ssid = WiFi.softAPSSID();
    addr = WiFi.softAPIP();
  }else{
    // Station (client) mode - move switch to 'Client'
    strncpy(chkd1, "checked\0", 8);
    ssid = WiFi.SSID();
    addr = WiFi.localIP();
    gate = WiFi.gatewayIP();
    mask = WiFi.subnetMask();
    // DHCP?
    if (AP.dhcp) {
      // mode switch to DHCP
      strncpy(chkd2, "checked\0", 8);
      // Disable IP address input
      strncpy(disd1, "disabled\0", 9);
      strncpy(disd2, "disabled\0", 9);
    }
  }

  snprintf(html, htmlSize, cfgWifiPage,
           chkd1,
           chkd2,
           disc2,
           ssid.c_str(),
           addr[0],
           disd1,
           addr[1],
           disd1,
           addr[2],
           disd1,
           addr[3],
           disd1,
           gate[0],
           disd2,
           gate[1],
           disd2,
           gate[2],
           disd2,
           gate[3],
           disd2,
           mask[0],
           disd2,
           mask[1],
           disd2,
           mask[2],
           disd2,
           mask[3],
           disd2
          );
  AR488srv.send(200, "text/html", html);
}


/***** Send AR488 configuration page *****/
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
  rsz = getReply("++mode", reply, 64);
  if (rsz > 0) cmode = (atoi(reply) ? true : false);
  if (cmode) {
    strncpy(chkm, "checked\0", 8);
  } else {
    strncpy(dis, "disabled\0", 9);
  }

  rsz = getReply("++eoi", reply, 64);
  if (rsz > 0) ceoi = (atoi(reply) ? true : false);
  if (ceoi) strncpy(chki, "checked\0", 8);

  rsz = getReply("++eot_enable", reply, 64);
  if (rsz > 0) ceot = (atoi(reply) ? true : false);
  if (ceot) strncpy(chkt, "checked\0", 8);


  // Get parameters
  rsz = getReply("++addr", reply, 64);
  if (rsz > 0) gaddr = atoi(reply);
  rsz = getReply("++auto", reply, 64);
  if (rsz > 0) gauto = atoi(reply);
  rsz = getReply("++read_tmo_ms", reply, 64);
  if (rsz > 0) gtmo = atoi(reply);
  //  rsz=getReply("++eoi", reply, 64);
  //  if (rsz>0) geoi = atoi(reply);
  rsz = getReply("++eos", reply, 64);
  if (rsz > 0) geosch = atoi(reply);
  //  rsz=getReply("++eot", reply, 64);
  //  if (rsz>0) geot = atoi(reply);
  rsz = getReply("++eot_char", reply, 64);
  if (rsz > 0) geotch = atoi(reply);

  rsz = getReply("++ver", reply, 48);
  if (rsz > 0) strncpy(vstr, reply, rsz);

  rsz = getReply("++ver real", reply, 48);
  if (rsz > 0) strncpy(ver, reply, rsz);

  snprintf(html, htmlSize, cfg488Page,
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
  AR488srv.send(200, "text/html", html);
}


/***** Admin functions page *****/
void cfgAdm() {
  AR488srv.send(200, "text/html", cfgAdmPage);
}


/***** Return CSS stylesheet *****/
void lnkStyle() {
  AR488srv.send(200, "text/html", css);
}


/***** Return main JavaScript code *****/
void lnkScript() {
  AR488srv.send(200, "text/html", lnkScriptJs);
}


/***** JavaScript code for Ceneral config page *****/
void cfgGenjs() {
  AR488srv.send(200, "text/html", cfgGenJs);
}


/***** JavaScript code for WiFi config page *****/
void cfgWiFijs() {
  AR488srv.send(200, "text/html", cfgWifiJs);
}


/***** JavaScript code for AR488 config page *****/
void cfg488js() {
  AR488srv.send(200, "text/html", cfg488Js);
}


/***** JavaScript code for Admin page *****/
void cfgAdmjs() {
  AR488srv.send(200, "text/html", cfgAdmJs);
}


void SHA1js() {
  AR488srv.send(200, "text/html", SHA1funcJs);
}


/***** Set WiFi parameters *****/
void setWifi() {
//  bool stat = false;
  char msg[35] = {"Re-directing to "};
  uint8_t dly = 10;

  // Checksum current config
#ifdef DEBUG_2
  Serial.println(F("Received WiFi config page..."));
  showArgs();
#endif

  AP.wclient = ((AR488srv.arg("chkM").length()>0) ? true : false);
  AP.dhcp = ((AR488srv.arg("chkD").length()>0) ? true : false);

#ifdef DEBUG_2
if (AP.wclient) {
  Serial.println(F("Station mode requested"));
}else{
  Serial.println(F("AP mode requested"));
}
if (AP.dhcp) {
  Serial.println(F("DHCP IP address requested"));
}else{
  Serial.println(F("Static IP address provided"));
}
#endif

  if (AP.wclient) {
    // Station (client) mode requested
    if (!AP.dhcp) { // Static IP provided
      // Get IP address
      if (AR488srv.arg("addr1") != "") {
        AP.addr[0] = AR488srv.arg("addr1").toInt();
        AP.addr[1] = AR488srv.arg("addr2").toInt();
        AP.addr[2] = AR488srv.arg("addr3").toInt();
        AP.addr[3] = AR488srv.arg("addr4").toInt();
      }
      if (AR488srv.arg("gate1")) {
        AP.gate[0] = AR488srv.arg("gate1").toInt();
        AP.gate[1] = AR488srv.arg("gate2").toInt();
        AP.gate[2] = AR488srv.arg("gate3").toInt();
        AP.gate[3] = AR488srv.arg("gate4").toInt();
      }
      if (AR488srv.arg("mask1")) {
        AP.mask[0] = AR488srv.arg("mask1").toInt();
        AP.mask[1] = AR488srv.arg("mask2").toInt();
        AP.mask[2] = AR488srv.arg("mask3").toInt();
        AP.mask[3] = AR488srv.arg("mask4").toInt();
      }
    }
  }else{
    // AP mode requested (no DHCP available)
    if (AR488srv.arg("addr1") != "") {
      AP.addr[0] = AR488srv.arg("addr1").toInt();
      AP.addr[1] = AR488srv.arg("addr2").toInt();
      AP.addr[2] = AR488srv.arg("addr3").toInt();
      AP.addr[3] = AR488srv.arg("addr4").toInt();
      AP.gate[0] = AR488srv.arg("addr1").toInt();
      AP.gate[1] = AR488srv.arg("addr2").toInt();
      AP.gate[2] = AR488srv.arg("addr3").toInt();
      AP.gate[3] = AR488srv.arg("addr4").toInt();

      AP.mask[0] = 255;
      AP.mask[1] = 255;
      AP.mask[2] = 255;
      AP.mask[3] = 0;
    }
  }

  // Write changes to EEPROM
  epWriteData(EESTART, &AP, sizeof(AP));

  // Create re-direct message
  if (AP.wclient && AP.dhcp) {   // Note: we can't determine the DHCP address so no re-direct message!
    strcpy(msg,"\0\0");
  }else{
    strncat(msg, AR488srv.arg("addr1").c_str(), sizeof(AR488srv.arg("addr1")));
    strncat(msg, ".",1);
    strncat(msg, AR488srv.arg("addr2").c_str(), sizeof(AR488srv.arg("addr2")));
    strncat(msg, ".",1);
    strncat(msg, AR488srv.arg("addr3").c_str(), sizeof(AR488srv.arg("addr3")));
    strncat(msg, ".",1);
    strncat(msg, AR488srv.arg("addr4").c_str(), sizeof(AR488srv.arg("addr4")));
    strncat(msg,"...\0",4);

   if (AP.ssl==true) dly = 15;
  }

  // Re-direct page
  redirect(dly,AP.addr, AP.webp, msg);
  delay(200);
  
  // Stop webserver
  AR488srv.stop();
  
  // Disconnect stations (AP mode) or from network (Station mode)
  if (AP.wclient) {
#ifdef DEBUG_1
    Serial.println(F("Station mode>"));
#endif
    startWifiStn(AR488srv.arg("essid"), AR488srv.arg("pword"), AP.addr, AP.gate, AP.mask);
//      stat = hasWifiStarted();
  }else{
#ifdef DEBUG_1
    Serial.println(F("SoftAP mode>"));
#endif
    startWifiAP(AR488srv.arg("essid"), AR488srv.arg("pword"), AP.addr, AP.addr, AP.mask);
  }

  // Restart webserver (AP mode needs reset?)
  if (AP.wclient) {
#ifdef DEBUG_1
  Serial.println(F("Starting webserver..."));
#endif
    startWebServer();
  }else{
    if (!AP.wclient) ESP.restart();
  }
//  if (stat) updatePage("cfgWifi");

//  ESP.restart();

}


/***** Set general configuration options *****/
void setGen() {
  uint16_t webp;
  uint16_t gpibp;
  bool sc = false; // State change

#ifdef DEBUG_2
  Serial.println(F("Received general config page..."));
  showArgs();
#endif

// Change state of GPIB pass-through
  if (AR488srv.arg("pass") == "on") {
#ifdef DEBUG_2
    Serial.println(F("Passthrough turned on."));
#endif
    if (!AP.gpib) sc = true;
    AP.gpib = true;
    gpibp = AR488srv.arg("gpibp").toInt();
    if (AP.gpibp != gpibp) {
      sc = true;
      AP.gpibp = gpibp;
    }
    if (sc) {
#ifdef DEBUG_2
      Serial.print(F("Restarting TCP server on port: "));
      Serial.println(AP.gpibp);
#endif
      if (passSrv) delete passSrv;
      passSrv = new WiFiServer(gpibp);
      passSrv->begin();
    }
  } else {
    if (AP.gpib) {
      AP.gpib = false;
      // Disconnect any client
      if (passCli.connected()) passCli.stop();
      // Kill the server (note: causes reboot!)
      passSrv->stop();
      //      delete passSrv;
    }
  }

  // Save settings
  webp = AR488srv.arg("webp").toInt();
  epWriteData(EESTART, &AP, sizeof(AP));

  // If neccessary, restart web server
  if (webp != AP.webp) {
    AP.webp = webp;
    redirect(3, AP.addr, AP.webp, "Redirecting...");
    AR488srv.stop();
    startWebServer();
  }else{
    // Update the page
    updatePage("cfgGen");
  }
}


/***** Configure AR488 parameters *****/
void set488() {
  bool ctrlr = false;

#ifdef DEBUG_4
  Serial.println(F("Received GPIB config page..."));
  showArgs();
#endif

  if (AR488srv.arg("cmode") == "on") ctrlr = true;

  // Process switches
  if (AR488srv.arg("eoi") == "on") {
    Serial.println(F("++eoi 1"));
  } else {
    Serial.println(F("++eoi 0"));
  }
  flushIncoming();
  if (AR488srv.arg("eot") == "on") {
    Serial.println(F("++eot_enable 1"));
  } else {
    Serial.println(F("++eot_enable 0"));
  }
  flushIncoming();
  delay(200);

  // Process parameters
  Serial.print(F("++addr ")); Serial.println(AR488srv.arg("gpib"));
  flushIncoming();
  Serial.print(F("++eos ")); Serial.println(AR488srv.arg("eosch"));
  flushIncoming();
  Serial.print(F("++eot_char ")); Serial.println(AR488srv.arg("eotch"));
  flushIncoming();
  Serial.print(F("++setvstr ")); Serial.println(AR488srv.arg("vstr"));
  flushIncoming();


  // Set these only of controller mode is on
  if (ctrlr) {
    Serial.print(F("++auto ")); Serial.println(AR488srv.arg("auto"));
    flushIncoming();
    Serial.print(F("++read_tmo_ms ")); Serial.println(AR488srv.arg("readtmo"));
    flushIncoming();
  }
  delay(200);

  // Mode switch
  if (ctrlr) {
    Serial.println(F("++mode 1"));
  } else {
    Serial.println(F("++mode 0"));
  }
  flushIncoming();
  delay(200);

  // Save configuration
  if (AR488srv.arg("opt") == "2") {
    Serial.println(F("++savecfg"));
  }
  delay(200);

  // Update the page
  updatePage("cfg488");
}


/***** Reboot WiFi *****/
void admin() {
  uint8_t opt = 0;
  char hash[20];

#ifdef DEBUG_5
  Serial.println(F("Received admin page..."));
  showArgs();
#endif

  opt = AR488srv.arg("opt").toInt();
  AR488srv.arg("chk1").toCharArray(hash, 20);
  if (memcmp(hash, AP.pwdH, 20) == 0) {
    switch (opt) {
      case 1:
        break;
      case 2:
#ifdef DEBUG_5
        Serial.println(F("Resetting to defaults..."));
#endif
        setDefaultCfg();
        epWriteData(EESTART,&AP,sizeof(AP));
        startWifiAP(dfltSSID, dfltPwd, AP.addr, AP.addr, AP.mask);
      case 3:
#ifdef DEBUG_5
        Serial.println(F("Rebooting..."));
#endif
        redirect(10, AP.addr, AP.webp, "Success!");
        delay(500);
        ESP.restart();
        break;
    }
  } else {
    redirect(3, AP.addr, AP.webp, "Fail - check password!");
  }
  // Update the page
  updatePage("cfgAdm");
}


/***** Redirect to status page*****/
void redirect(uint8_t dly, uint8_t addr[4], uint16_t port, const char *msg) {
//  char pfx[6] = {0x68, 0x74, 0x74, 0x70, 0x73, 0x0};
//  IPAddress addr;

  char s = '\0';

//  if (AP.ssl==false) strncpy(pfx, "http\0", 5);
  if (AP.ssl==true) s = 0x73;

  snprintf(html, htmlSize, redirectPage,
           dly,
           s,
           addr[0],
           addr[1],
           addr[2],
           addr[3],
           port,
           msg
          );
//  AR488srv->send(200, "text/html", html);
  AR488srv.send(200, "text/html", html);
}


#ifdef DEBUG_2
/***** Display submitted arguments *****/
void showArgs() {
  uint8_t acnt = AR488srv.args();
  for (uint8_t i = 0; i < acnt; i++) {
    Serial.print(AR488srv.argName(i) + ":\t");
    Serial.println(AR488srv.arg(i));
  }
}
#endif


/***** Return updated selected page *****/
void updatePage(const char* page) {
  uint8_t len = strlen(page);
  strncpy(curPage, page, len);
  wwwMain();
  memset(curPage, '\0', len);
}



/***** Generate 8 bit checksum *****/
uint8_t chkSumCfg(char *cfg, uint8_t len) {
  uint8_t chk;
  long int sum = 0;

  for (int i = 0; i < len; i++) {
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
  p = Serial.readBytesUntil(LF, reply, rsize - 1);
  return p;
}


/***** Flush the incoming buffer *****/
void flushIncoming() {
  while (Serial.available()) {
    Serial.read();
  };
}


/***** Clear buffer *****/
void clrSBuf(uint8_t ptr) {
  memset(sBuf, '\0',ptr);
  sbPtr = 0;
}


/***** Random key generator *****/
/*
void randomGen() {
  uint8_t num = 0;
  for(uint8_t i=0; i<SSZ; i++){
    while (num<33 || num>126) num = RNG_WORD & 0xFF;
    AP.pwdS[i] = num;
    num = 0;
  }
}
*/

/***** Hash generator *****/
/*
void hashGen(char *str, uint8_t sz) {
  hash->reset();
  hash->update(str, sz);
  hash->finalize(AP.pwdH, HSZ);
}
*/


/***************************/
/***** EEPROM routines *****/
/***************************/

/***** Clear the EEPROM *****/
void epErase() {
  int i = EESIZE;

  // Load EEPROM data from Flash
  EEPROM.begin(EESIZE);
  for (i=0; i<EESIZE; i++)
    EEPROM.write(i, 0xFF);
  EEPROM.commit();
  EEPROM.end();
}


/***** Write data to EEPROM (with CRC) *****/
/*
 * addr = EEPROM address
 * cfg = config data union object
 * csize = size of config data object
 */
void epWriteData(uint16_t addr, cfgObj * cptr, uint16_t csize) {
  uint16_t crc;

  // Load EEPROM data from Flash
  EEPROM.begin(EESIZE);
  // Write data
  EEPROM.put(addr,*cptr);
  // Write CRC
  crc = getCRC16(cptr->db, csize);
  EEPROM.put(0, crc);
  // Commit write to Flash
  EEPROM.commit();
  EEPROM.end();
}


/***** Read data from EEPROM (with CRC check) *****/
/*
 * addr = EEPROM address
 * cfg = config data union object
 * csize = size of config data object
 */
bool epReadData(uint16_t addr, cfgObj * cptr, uint16_t csize) {
  uint16_t crc1;
  uint16_t crc2;

  // Load EEPROM data from Flash
  EEPROM.begin(EESIZE);
  // Read CRC
  EEPROM.get(0,crc1);
  // Read data
  EEPROM.get(addr, *cptr);
  EEPROM.end();
  // Get CRC of config
  crc2 = getCRC16(cptr->db, csize);
  if (crc1==crc2) {
    return true;
  }else{
    return false;
  }
}


bool isEepromClear(){
  int16_t crc = 0;

  // Load data from EEPROM
  EEPROM.begin(EESIZE);
  // Read data
  EEPROM.get(0, crc);
  EEPROM.end(); 
  // Return result
  if (crc==-1) {
    return true;
  }else{
    return false;
  }
}


/***** Generate 16 bit CRC *****/
uint16_t getCRC16(uint8_t bytes[], uint16_t bsize){
  uint8_t x;
  uint16_t crc = 0xFFFF;

  for (uint16_t idx=0; idx<bsize; ++idx) {
    x = crc >> 8 ^ bytes[idx];
    x ^= x>>4;
    crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
  }
  return crc;
}

/**********************************/
/***** End of EEPROM routines *****/
/**********************************/
