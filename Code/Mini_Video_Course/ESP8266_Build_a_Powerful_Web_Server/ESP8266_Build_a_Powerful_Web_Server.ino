/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/home-automation-using-esp8266/
*********/

// Loading libraries
#include <ESP8266WiFi.h>
#include "DHT.h"

// Uncomment one of the lines below for whatever DHT sensor type you're using
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// REPLACE THOSE VARIABLES WITH YOUR SSID and PASSWORD
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Web server runs on port 8888
WiFiServer server(8888);

// Header page
String header;

// GPIOs variables
String output1State = "Off";
String output2State = "Off";
int output1 = 16;
int output2 = 2;

// DHT sensor pin
const int DHTPin = 5;
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Temporary variables to store DHT data
static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];

// Smoke sensor pin
int smokePin = A0;
// Smoke threshold, you can adjust this value for your own readings
int smokeThres = 60;
// PIR mtion sensor
const int motionSensor = 4;

// Control variables for smoke and motion sensors
boolean armSmoke = false;
boolean smokeTriggered = false;
boolean armMotion = false;
boolean motionTriggered = false;

// LEDs for smoke and motion status
const int smokeLED = 13;
const int motionLED = 12;

// Buzzer pin
const int buzzerPin = 14;

// Timers auxiliar variables
unsigned long now = millis();
unsigned long lastSmokeCheck = 0;
unsigned long previousTime = 0;

// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Only runs once
void setup() {
  // Preparing GPIOs
  pinMode(output1, OUTPUT);
  digitalWrite(output1, LOW);
  pinMode(output2, OUTPUT);
  digitalWrite(output2, LOW);
  pinMode(smokeLED, OUTPUT);
  digitalWrite(smokeLED, LOW);
  pinMode(motionLED, OUTPUT);
  digitalWrite(motionLED, LOW);
  pinMode(buzzerPin, OUTPUT);
  pinMode(smokePin, INPUT);
  // Setting motionSensor as an interrupt
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);
  // Initializing DHT sensor
  dht.begin();

  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  delay(10);

  // Connecting to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Starting the web server
  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(10000);

  // Printing the ESP IP address in the serial monitor
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println(":8888");
}

// Runs over and over again
void loop() {
  now = millis();
  // Checks current smoke value every 500 milliseconds
  if (now - lastSmokeCheck > 500) {
    lastSmokeCheck = now;
    int smokeValue = analogRead(smokePin);
    // If the smoke sensor is armed and smoke is detected. It sends a notification saying "Smoke Detected" that is printed in the dashboard
    if (smokeValue > smokeThres && armSmoke) {
      Serial.print("Pin A0: ");
      Serial.println(smokeValue);
      tone(buzzerPin, 1000, 200);
      if (!smokeTriggered) {
        Serial.println("SMOKE DETECTED!");
        smokeTriggered = true;
      }
    }
  }
  // Listenning for new clients
  WiFiClient client = server.available();

  // When a client is available
  if (client) {
    previousTime = now;
    Serial.println("New client");
    // Reads temperature and humidity
    readTemperatureHumidity();
    // Bolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected() && now - previousTime <= timeoutTime) {
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n' && blank_line) {
          Serial.print(header);
          // Checking if header is valid
          // You can use this website https://www.base64decode.org/ to generate a new username and password (select the Encode tab)
          // Enter with the following format: yourusername:yourpassword is encoded as eW91cnVzZXJuYW1lOnlvdXJwYXNzd29yZA==
          // In this case, I've used the username user and the password pass, so the encoded string is dXNlcjpwYXNz = 'user:pass' (user:pass) base64 encode
          if (header.indexOf("dXNlcjpwYXNz") >= 0) {
            //successful login
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            // Checks the current URL that you opened in your web server
            // For example the URL /output1off turns the output 1 off
            // The /armSmoke URL arms the smoke sensor and so on...
            // These URL are opened when you press the buttons in your dashboard
            if (header.indexOf("GET / HTTP/1.1") >= 0) {
              Serial.println("Main Web Page");
            }
            else if (header.indexOf("GET /output1on HTTP/1.1") >= 0) {
              Serial.println("Output 1 On");
              output1State = "On";
              digitalWrite(output1, HIGH);
            }
            else if (header.indexOf("GET /output1off HTTP/1.1") >= 0) {
              Serial.println("Output 1 Off");
              output1State = "Off";
              digitalWrite(output1, LOW);
            }
            else if (header.indexOf("GET /output2on HTTP/1.1") >= 0) {
              Serial.println("Output 2 On");
              output2State = "On";
              digitalWrite(output2, HIGH);
            }
            else if (header.indexOf("GET /output2off HTTP/1.1") >= 0) {
              Serial.println("Output 2 Off");
              output2State = "Off";
              digitalWrite(output2, LOW);
            }
            else if (header.indexOf("GET /armSmoke HTTP/1.1") >= 0) {
              Serial.println("Smoke sensor is: Armed");
              digitalWrite(smokeLED, HIGH);
              armSmoke = true;
            }
            else if (header.indexOf("GET /disarmSmoke HTTP/1.1") >= 0) {
              Serial.println("Smoke sensor is: Disarmed");
              digitalWrite(smokeLED, LOW);
              armSmoke = false;
              smokeTriggered = false;
            }
            else if (header.indexOf("GET /armMotion HTTP/1.1") >= 0) {
              Serial.println("Motion sensor is: Armed");
              digitalWrite(motionLED, HIGH);
              armMotion = true;
            }
            else if (header.indexOf("GET /disarmMotion HTTP/1.1") >= 0) {
              Serial.println("Motion sensor is: Disarmed");
              digitalWrite(motionLED, LOW);
              armMotion = false;
              motionTriggered = false;
            }
            // Displays your html web page
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head>");
            client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            // Loads Bootstrap framework to give you the mobile reponsive web page and the buttons look
            client.println("<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css\">");
            client.println("<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js\"></script>");
            client.println("</head><div class=\"container\">");
            client.println("<h1>ESP8266 Web Server - <a href=\"/\">Refresh</a></h1>");
            // Generates buttons to control the outputs
            client.println("<h2>Output 1 - State: " + output1State + "</h2><div class=\"row\">");
            // If LED is off, it shows the button to turn the output 1 on
            if (output1State == "Off") {
              client.println("<div class=\"col-md-2\"><a href=\"/output1on\" class=\"btn btn-block btn-lg btn-success\" role=\"button\">ON</a></div></div>");
            }
            // If LED is on, it shows the button to turn the output 1 off
            else if (output1State == "On") {
              client.println("<div class=\"col-md-2\"><a href=\"/output1off\" class=\"btn btn-block btn-lg btn-danger\" role=\"button\">OFF</a></div></div>");
            }
            client.println("<h2>Output 2 - State: " + output2State + "</h2><div class=\"row\">");
            // If LED is off, it shows the button to turn the output 2 on
            if (output2State == "Off") {
              client.println("<div class=\"col-md-2\"><a href=\"/output2on\" class=\"btn btn-block btn-lg btn-success\" role=\"button\">ON</a></div></div>");
            }
            // If LED is on, it shows the button to turn the output 2 off
            else if (output2State == "On") {
              client.println("<div class=\"col-md-2\"><a href=\"/output2off\" class=\"btn btn-block btn-lg btn-danger\" role=\"button\">OFF</a></div></div>");
            }
            // Checks if there was smoke detected in your room and prints a message according to the current state (smoke detected or no smoke)
            if (smokeTriggered) {
              client.println("<h2>Smoke Sensor - State: Smoke detected</h2><div class=\"row\">");
            }
            else {
              client.println("<h2>Smoke Sensor - State: No smoke</h2><div class=\"row\">");
            }
            // If the smoke sensor is armed, it shows the button to disarm the sensor
            if (armSmoke) {
              client.println("<div class=\"col-md-2\"><a href=\"/disarmSmoke\" class=\"btn btn-block btn-lg btn-default\" role=\"button\">Disarm</a></div></div>");
            }
            // If the smoke sensor is disarmed, it shows the button to arm the sensor
            else {
              client.println("<div class=\"col-md-2\"><a href=\"/armSmoke\" class=\"btn btn-block btn-lg btn-primary\" role=\"button\">Arm</a></div></div>");
            }
            // Checks if there was motion detected in your room and prints a message according to the current state (motion detected or no motion)
            if (motionTriggered) {
              client.println("<h2>Motion Sensor - State: Motion detected</h2><div class=\"row\">");
            }
            else {
              client.println("<h2>Motion Sensor - State: No motion</h2><div class=\"row\">");
            }
            // If the motion sensor is armed, it shows the button to disarm the sensor
            if (armMotion) {
              client.println("<div class=\"col-md-2\"><a href=\"/disarmMotion\" class=\"btn btn-block btn-lg btn-default\" role=\"button\">Disarm</a></div></div>");
            }
            // If the motion sensor is disarmed, it shows the button to arm the sensor
            else {
              client.println("<div class=\"col-md-2\"><a href=\"/armMotion\" class=\"btn btn-block btn-lg btn-primary\" role=\"button\">Arm</a></div></div>");
            }
            // Prints the latest temperatuer and humidity readings
            client.println("<div class=\"row\"><h3>Temperature: ");
            client.println(celsiusTemp);
            client.println("*C</h3><h3>Temperature: ");
            client.println(fahrenheitTemp);
            client.println("*F</h3><h3>Humidity: ");
            client.println(humidityTemp);
            client.println("%</h3></div>");
            client.println("</div></div></html>");
          }
          // If you enter wrong user or password, the http request fails...
          else {
            client.println("HTTP/1.1 401 Unauthorized");
            client.println("WWW-Authenticate: Basic realm=\"Secure\"");
            client.println("Content-Type: text/html");
            client.println();
            client.println("<html>Authentication failed</html>");
          }
          header = "";
          break;
        }
        if (c == '\n') {
          // When starts reading a new line
          blank_line = true;
        }
        else if (c != '\r') {
          // When finds a character on the current line
          blank_line = false;
        }
      }
    }
    // Closing the client connection
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
}

void readTemperatureHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // Computes temperature values in Celsius
  float hic = dht.computeHeatIndex(t, h, false);
  dtostrf(hic, 6, 2, celsiusTemp);
  // Computes temperature values in Fahrenheit
  float hif = dht.computeHeatIndex(f, h);
  dtostrf(hif, 6, 2, fahrenheitTemp);
  // Stores humidity
  dtostrf(h, 6, 2, humidityTemp);
  // Serial prints that are used for debugging purposes
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t Heat index: ");
  Serial.print(hic);
  Serial.println(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t Heat index: ");
  Serial.print(hic);
  Serial.println(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
  return;
}

// Detects motion
ICACHE_RAM_ATTR void detectsMovement() {
  Serial.println("MOTION!");
  // If the motion sensor is armed and motion is detected. It sends a notification saying "Motion Detected" that is printed in the dashboard
  if (armMotion && !motionTriggered) {
    Serial.println("MOTION DETECTED!");
    motionTriggered = true;
  }
}
