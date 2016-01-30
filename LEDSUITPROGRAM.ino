#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// LED
#define PIN 5
#define PARTS 5

// Position matrixes
byte right_bottom_arm [][3] = {{0,  1,  2}, {3,  4,  5}, {6, 7, 8}, {9, 10, 11}};
byte right_upper_arm [][3] = {{12,  13,  14}, {15,  16,  17}, {18, 18, 19}};
byte chest [][6] = {{20,  21,  22,  23,  24,  25},{26,  27,  28,  29,  30, 31},{32, 33, 34, 35, 36, 37},{38, 39, 40, 41, 42, 43},{44, 45, 46, 47, 48, 49}, {50, 51, 52, 52, 53, 54}};
byte left_upper_arm [][55] = {{56,  57,  58}, {59,  60, 61}, {62, 62, 63}};
byte left_bottom_arm [][3] = {{64,  65, 66}, {67,  68,  69}, {70, 71, 72}, {73, 74, 75}};

// LedStrip Object
Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);

//WIFI Connect
int status = WL_IDLE_STATUS;
char ssid[] = "";  //  your network SSID (name)
char pass[] = "";       // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)

//WebServer
const char* host = "172.31.81.194";

//WIFI Webserver 
WiFiServer server(80);

//NTP
unsigned int localPort = 2390; 
IPAddress timeServer(95, 46, 198, 21);
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
unsigned long secsSince1900 = 0;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;


void setupAccessPoint()
{
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "Led Suit  " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, "spark123");
}

void runWebServer()
{
    // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  char* charBuf;
  req.toCharArray(charBuf, req.length());
 
  Serial.println(req);
  client.flush();

  // Match the request
  int val = -1; // We'll use 'val' to keep track of both the
                // request type (read/set) and value if set.
  if (req.substring(5,10) == "setup")
      val = 0; // Will write LED low
  else  if (req.substring(5,12) == "connect")
    val = 1; // Will write LED high
 
  // Prepare the response. Start with the common header:
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  // If we're setting the LED, print out a message saying we did
  if (val == 0)
  {
    s += "<form action=\"/connect\" method=\"get\">";
    s += "ssid:<input type=\"text\" name=\"ssid\"><br>";
    s += "pass:<input type=\"text\" name=\"pass\"><br>";
    s += "<input type=\"submit\" value=\"Submit\">";
    s += "</form>";
    s += "</html>\n";
  }
  else if (val == 1)
  { // If we're reading pins, print out those values:
    s += "Connecting wifi";
    // GET /connect?ssid=rdm&pass=123
    int ssidIndex = req.indexOf("ssid=");
    //  Search for the next comma just after the first
    int passIndex = req.indexOf("&pass=", ssidIndex+1);
    
    String ssid = req.substring(ssidIndex+1, passIndex);
    String pass = req.substring(passIndex); // 

    int isIndex = ssid.indexOf("=");
    ssid = ssid.substring(isIndex+1);

    isIndex = pass.indexOf("=");
    int sndIndex = pass.indexOf(" ");
    pass = pass.substring(isIndex+1, sndIndex);
    
//    connectWiFi();
  }
  else
  {
    s += "Invalid Request.<br> Try /led/1, /led/0, or /read.";
  }
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
}

// Store NTP-Server seconds from 1900
void setTime(){
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    secsSince1900 = highWord << 16 | lowWord;
  }
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}

void connectWiFi(){
  WiFi.mode(WIFI_STA);
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.println("Attempting to connect to SSID: ");
   //connect to Wifi network
    status = WiFi.begin(ssid, pass);
  }
  if (status) {
    Serial.println("Connected to the WIFI");
  }
}
void getStatus(){
  Serial.print("connecting to: ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80 ;
  if (!client.connect(host, 80)) {
    Serial.print("webserver port 80 ");
    Serial.println("connection failed");
  }

  // We now create a URI for the request
  String url = "/scenarios";
 
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.println(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}

void hexToRGB(String hexstring, byte *buff){
  long number = (long) strtol( &hexstring[1], NULL, 16);
  buff[0]= number >> 16;
  buff[1] = number >> 8 & 0xFF;
  buff[2] = number & 0xFF;
}

void setAllColor(int id, uint32_t color){
  size_t y = 0;
  size_t x = 0;
 
  if (id == 1){
    y = sizeof (right_bottom_arm);
    x = sizeof (right_bottom_arm[0]);

    for (int i = 0; i < x; i++){
      for (int ii = 0; ii < y; ii++){
        strip.setPixelColor(right_bottom_arm[ii][i], color);
      }
    }
  }
  else if (id == 2){
    y = sizeof (right_upper_arm);
    x = sizeof (right_upper_arm[0]);

    for (int i = 0; i < x; i++){
      for (int ii = 0; ii < y; ii++){
        strip.setPixelColor(right_upper_arm[ii][i], color);
      }
    }
  }
  else if (id == 3){
    y = sizeof (chest);
    x = sizeof (chest[0]);

    for (int i = 0; i < x; i++){
      for (int ii = 0; ii < y; ii++){
        strip.setPixelColor(chest[ii][i], color);
      }
    }
  }
  else if (id == 4){
    y = sizeof (left_upper_arm);
    x = sizeof (left_upper_arm[0]);

    for (int i = 0; i < x; i++){
      for (int ii = 0; ii < y; ii++){
        strip.setPixelColor(left_upper_arm[ii][i], color);
      }
    }
  }
  else if (id == 5){
     y = sizeof (left_bottom_arm);
    x = sizeof (left_bottom_arm[0]);

    for (int i = 0; i < x; i++){
      for (int ii = 0; ii < y; ii++){
        strip.setPixelColor(left_bottom_arm[ii][i], color);
      }
    }
  }
}

// Parse json from webserver to 2d byte array
void parseScenario(){
//  char json[] = "{\"p\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
//
//  StaticJsonBuffer<200> jsonBuffer;
//  
//  JsonObject& root = jsonBuffer.parseObject(json);
//  
//  const char* sensor = root["parts"];
//  long time          = root["time"];
//  double latitude    = root["data"][0];
//  double longitude   = root["data"][1];
}
void setup() {
  setAllColor(0, strip.Color(255,0,0));
  // put your setup code here, to run once:
  //  Serial.begin(9600);
  //  setupAccessPoint();
  //  connectWiFi();
  //  getStatus();
}

void loop() {
  // put your main code here, to run repeatedly:

}



