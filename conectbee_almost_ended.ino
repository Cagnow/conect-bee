#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
SoftwareSerial ArduinoMaster(5,6);
String msg;

//SSID and Password to your ESP Access Point
const char* ssid = "conectbee";
const char* password = "12345678";

ESP8266WebServer server(80); 


bool is_authenticated() {
  Serial.println("Enter is_authenticated");
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentication Successful");
      return true;
    }
  }
  Serial.println("Authentication Failed");
  return false;
}


void handleLogin() {
  String msg;
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
    server.send(301);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")) {
    if (server.arg("USERNAME") == "admin" &&  server.arg("PASSWORD") == "admin") {
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
      server.send(301);
      Serial.println("Log in Successful");
      return;
    }
    msg = "Mauvais combo usr/pwd";
    Serial.println("Log in Failed");
  }
  String content = "<html><body><form action='/login' method='POST'>Pour se connecter : admin/admin<br>";
  content += "Utilisateur:<input type='text' name='USERNAME' placeholder='Nom d'utilisateur'><br>";
  content += "Mot de passe:<input type='password' name='PASSWORD' placeholder='Mot de passe'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  content += "<a href='/inline'>Sans auth</a></body></html>";
  content += "<h4>"+String(msg)+"</h4>";
  server.send(200, "text/html", content);
}
void handleRoot() {
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authenticated()) {
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }
  String content = "<html><body><H2>Connect'Bee</H2><br>";
  if (server.hasHeader("User-Agent")) {
    content += "Vous utilisez : " + server.header("User-Agent") + "<br><br>";
  }
  content += "<a href=\"/login?DISCONNECT=YES\">Se déconnecter</a></body></html>";
  server.send(200, "text/html", content);

}
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void){
  Serial.begin(9600);
  Serial.println("");
  WiFi.mode(WIFI_AP);           
  WiFi.softAP(ssid, password);  

  IPAddress myIP = WiFi.softAPIP(); 
  Serial.print("ip:");
  Serial.println(myIP);
 
  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/inline", []() {
    server.send(200, "text/plain", "Mode sans authentification");
  });
    server.onNotFound(handleNotFound);
  //ask server to track these headers
  server.collectHeaders("User-Agent", "Cookie");
  server.begin();                  //Start server
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();          
  readMasterPort();
  
  }
void readMasterPort(){
while (ArduinoMaster.available()) {
  delay(10);
  if (ArduinoMaster.available() >0) {
    char c = ArduinoMaster.read();  
    msg += c; 
    Serial.println(msg);
  }
}
}
