/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/home-automation-using-esp8266/
*********/

#include <ESP8266WiFi.h>

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRsend.h>

// An IR LED is controlled by GPIO pin 4 (D2)
IRsend irsend(4);

// REPLACE WITH YOUR SSID AND PASSWORD
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Stores last button pressed on web server
String lastState;

// Create an instance of the server on port 80
WiFiServer server(80);

void setup() {
  irsend.begin();

  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  delay(10);

  // prepare GPIO2
  pinMode(4, OUTPUT);
  digitalWrite(4, 0);

  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // Check which request was triggered
  int irCode;
  if (req.indexOf("/lamp/on") != -1) {
    irCode = 0xFFE01F;
    lastState = "on";
  }
  else if (req.indexOf("/lamp/off") != -1) {
    irCode = 0xFF609F;
    lastState = "off";
  }
  else if (req.indexOf("/lamp/brighten") != -1) {
    irCode = 0xFFA05F;
    lastState = "brighten";
  }
  else if (req.indexOf("/lamp/dim") != -1) {
    irCode = 0xFF20DF;
    lastState = "dim";
  }
  else if (req.indexOf("/lamp/red") != -1) {
    irCode = 0xFF906F;
    lastState = "red";
  }
  else if (req.indexOf("/lamp/green") != -1) {
    irCode = 0xFF10EF;
    lastState = "green";
  }
  else if (req.indexOf("/lamp/blue") != -1) {
    irCode = 0xFF50AF;
    lastState = "blue";
  }
  else if (req.indexOf("/lamp/yellow") != -1) {
    irCode = 0xFF8877;
    lastState = "yellow";
  }
  else if (req.indexOf("/lamp/cyan") != -1) {
    irCode = 0xFF708F;
    lastState = "cyan";
  }
  else if (req.indexOf("/lamp/purple") != -1) {
    irCode = 0xFF6897;
    lastState = "purple";
  }
  else {
    lastState = "none";
    Serial.println("Invalid request - IR signal not sent");
  }
  // Send IR signal to RGB LED lamp
  Serial.print("NEC: ");
  Serial.println(irCode, HEX);
  irsend.sendNEC(irCode, 32);
  client.flush();

  // Prepare the client response (contains web page)
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
  s += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>";
  s += "<h1>RGB Lamp</h1>Last state: ";
  s += lastState;
  s += "<p><a href=\"/lamp/on\"><button>ON</button></a><a href=\"/lamp/off\"><button>OFF</button></a></p>";
  s += "<p><a href=\"/lamp/brighten\"><button>BRIGHTEN</button></a><a href=\"/lamp/dim\"><button>DIM</button></a></p>";
  s += "<p><a href=\"/lamp/red\"><button>RED</button></a><a href=\"/lamp/green\"><button>GREEN</button></a><a href=\"/lamp/blue\"><button>BLUE</button></a></p>";
  s += "<p><a href=\"/lamp/yellow\"><button>YELLOW</button></a><a href=\"/lamp/cyan\"><button>CYAN</button></a><a href=\"/lamp/purple\"><button>PURPLE</button></a></p>";
  s += "</html>\n";

  // Send the response to the client and stop client
  client.print(s);
  delay(1);
  Serial.println("Client disconnected");
  client.stop();
}
