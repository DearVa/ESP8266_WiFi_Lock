#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h> 

#define SSID_NAME "WiFi-Lock" //WiFi热点名

const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;
IPAddress APIP(192, 168, 1, 1); //网关

unsigned int n = 0, m = 0;
unsigned long t;
DNSServer dnsServer; 
ESP8266WebServer webServer(80);
bool locked = false, opened = false;
String html = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Password Input:</title><style>.b{background-color:#4CAF50;border:none;border-radius:5px;color:white;padding:20px 70px;text-align:center;font-size:20px;}</style><script language=\"javascript\"type=\"text/javascript\">var n=0;function b(i){t.value=t.value+i;if(n<3){n+=1;}else{window.location.href=\"?pw=\"+ t.value;t.value=\"\";n=0;}}</script><p style=\"height:10px\"align=\"center\"><input id=\"t\"type=\"password\"style=\"text-align:center;font-size:50px;background-color:transparent;border:0\"/></p><br /><p style=\"height:50px\"align=\"center\"><input type=\"button\"class=\"b\"value=\"1\"onclick=\"return b(1)\"/> <input type=\"button\"class=\"b\"value=\"2\"onclick=\"return b(2)\"/> <input type=\"button\"class=\"b\"value=\"3\" onclick=\"return b(3)\"/></p><p style=\"height: 50px\" align=\"center\"><input type=\"button\"class=\"b\"value=\"4\"onclick=\"return b(4)\"/> <input type=\"button\"class=\"b\"value=\"5\"onclick=\"return b(5)\"/> <input type=\"button\"class=\"b\"value=\"6\"onclick=\"return b(6)\"/></p><p style=\"height:50px\"align=\"center\"><input type=\"button\"class=\"b\"value=\"7\"onclick=\"return b(7)\"/> <input type=\"button\"class=\"b\"value=\"8\"onclick=\"return b(8)\"/> <input type=\"button\"class=\"b\"value=\"9\"onclick=\"return b(9)\"/></p><p style=\"height:30px\"align=\"center\"><input type=\"button\"class=\"b\"value=\"0\"onclick=\"return b(0)\"/></p></head></html>";
String closehtml = "<!DOCTYPE html><html><head><script language=\"javascript\"type=\"text/javascript\">function a(){window.location.href=\"?pw=close\";}</script><div style=\"text-align:center\"><input type=\"image\"src=\"data:image/png;base64,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\"onclick=\"a()\"></div></head></html>";
String error1 = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><h1>连续输错密码，请";
String error2 = "分钟后再试。</h1></head></html>";
String error = "";

void pw() {
  if(webServer.arg("pw")=="1328") {
    digitalWrite(15, HIGH);
    Serial.println("Correct");
    opened = true;
    n = 0;
  }else if(webServer.arg("pw")=="close"){
    digitalWrite(15, LOW);
    opened = false;
  }else if(webServer.arg("pw").length()==4){
    digitalWrite(15, LOW);
    opened = false;
    n++;
    if(n >= 5) {
      if(locked == false) {
        t = millis();
        locked = true;
      } else {
        if(int(t + 300000 - millis()) <= 0) {
          n = 0;
          locked = false;
        }
      }
    }
  }
  if(opened){
    webServer.send(200, "text/html", closehtml);
  }else if(locked){
    webServer.send(200, "text/html", error1 + int(t + 359999 - millis()) / 60000 + error2);
  }else{
    webServer.send(200, "text/html", html);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID_NAME);
  dnsServer.start(DNS_PORT, "*", APIP);
  webServer.onNotFound(pw);
  webServer.begin();
  pinMode(15, OUTPUT);
}

void loop() { 
  dnsServer.processNextRequest(); 
  webServer.handleClient(); 
}
