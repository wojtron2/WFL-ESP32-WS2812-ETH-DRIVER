#include "Freenove_WS2812_Lib_for_ESP32.h"
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h> // Dodaje biblioteke ArduinoJson do obsługi JSON

#include <OneWire.h>
#include <DallasTemperature.h>

#define LEDS_COUNT 1
#define LEDS_PIN 27
#define CHANNEL 0

Freenove_ESP32_WS2812 ledstrip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);
//Adafruit_NeoPixel ledstrip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

byte mac[] = { 0xFE, 0xFE, 0xBE, 0xFE, 0xDE, 0xED };
IPAddress ip(192, 168, 2, 177);

EthernetServer server(80);
String header;

int currentRed = 0;
int currentGreen = 0;
int currentBlue = 0;
String LEDState = "off";

const int output26 = 26;
const int output25 = 25;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

// temp meas
// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// ## temp meas

void setup() {
  pinMode(output26, OUTPUT);
  pinMode(output25, OUTPUT);
  digitalWrite(output26, LOW);
  digitalWrite(output25, LOW);
  Ethernet.init(5);

  ledstrip.begin();
  ledstrip.setBrightness(100);
  ledstrip.setLedColorData(0, 0, 0, 0);
 // ledstrip.clear(); 
  ledstrip.show();

  Serial.begin(115200);
  while (!Serial) { ; }
  Serial.println("Ethernet WebServer Example");

  Ethernet.begin(mac, ip);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) { delay(1); }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  ledstrip.setLedColorData(0, 255, 0, 0);
  ledstrip.show();

  sensors.begin();
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";
    
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;

        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf("POST /updateColor") >= 0) {
              // Read POST data
              String postData = "";
              while (client.available()) {
                postData += (char)client.read();
              }
              
              // Parse JSON data
              DynamicJsonDocument doc(1024);
              deserializeJson(doc, postData);
              int red = doc["red"];
              int green = doc["green"];
              int blue = doc["blue"];
              //Serial.printf("Received color - R:%d G:%d B:%d\n", red, green, blue);
              ledstrip.setLedColorData(0, red, green, blue);
              currentRed = red;
              currentGreen = green;
              currentBlue = blue;
              LEDState = "on";
              ledstrip.show();
            } else if (header.indexOf("GET /LEDs/brightness/30") >= 0) {
              Serial.println("brightness 30");
              ledstrip.setBrightness(30);
              LEDState = "on";
              ledstrip.show();
            }
              else if (header.indexOf("GET /LEDs/brightness/on") >= 0) {
              Serial.println("brightness 100");
              ledstrip.setBrightness(100);
              LEDState = "on";
              ledstrip.show();
            } else if (header.indexOf("GET /LEDs/brightness/off") >= 0) {
              Serial.println("brightness 0");
              ledstrip.setBrightness(0);
              LEDState = "off";
              ledstrip.show();
            } else if (header.indexOf("GET /RGB/r") >= 0) {
              // Extract RGB values from URL
              int red = header.substring(header.indexOf("r") + 1, header.indexOf("g")).toInt();
              int green = header.substring(header.indexOf("g") + 1, header.indexOf("b")).toInt();
              int blue = header.substring(header.indexOf("b") + 1).toInt();
              //Serial.printf("Received color - R:%d G:%d B:%d\n", red, green, blue);
              ledstrip.setLedColorData(0, red, green, blue);
              currentRed = red;
              currentGreen = green;
              currentBlue = blue;
              LEDState = "on";
              ledstrip.show();
            }
              else if (header.indexOf("GET /RGBall/r") >= 0) {
              // Extract RGB values from URL
              int red = header.substring(header.indexOf("r") + 1, header.indexOf("g")).toInt();
              int green = header.substring(header.indexOf("g") + 1, header.indexOf("b")).toInt();
              int blue = header.substring(header.indexOf("b") + 1).toInt();
              //Serial.printf("Received color - R:%d G:%d B:%d\n", red, green, blue);
              ledstrip.setAllLedsColorData(red, green, blue);
              currentRed = red;
              currentGreen = green;
              currentBlue = blue;
              LEDState = "on";
              ledstrip.show();
            } 
              else if (header.indexOf("GET /RGBall/red") >= 0) {
              Serial.println("Set color to Red all");
              ledstrip.setAllLedsColorData(255, 0, 0);
              currentRed = 255;
              currentGreen = 0;
              currentBlue = 0;
              LEDState = "on";
              ledstrip.show();
            } 
              else if (header.indexOf("GET /RGBall/green") >= 0) {
              Serial.println("Set color to Green all");
              ledstrip.setAllLedsColorData(0, 255, 0);
              currentRed = 255;
              currentGreen = 0;
              currentBlue = 0;
              LEDState = "on";
              ledstrip.show();
            } 
              else if (header.indexOf("GET /RGBall/blue") >= 0) {
              Serial.println("Set color to Blue all");
              ledstrip.setAllLedsColorData(0, 0, 255);
              currentRed = 255;
              currentGreen = 0;
              currentBlue = 0;
              LEDState = "on";
              ledstrip.show();
            } 
            else if (header.indexOf("GET /RGBall/pink") >= 0) {
              Serial.println("Set color to Blue all");
              ledstrip.setAllLedsColorData(255, 0, 125);
              currentRed = 255;
              currentGreen = 0;
              currentBlue = 0;
              LEDState = "on";
              ledstrip.show();
            } 
            
              else if (header.indexOf("GET /RGB/r255g0b0") >= 0) {
              Serial.println("Set color to Red");
              ledstrip.setLedColorData(0, 255, 0, 0);
              currentRed = 255;
              currentGreen = 0;
              currentBlue = 0;
              LEDState = "on";
              ledstrip.show();
            } else if (header.indexOf("GET /RGB/r0g255b0") >= 0) {
              Serial.println("Set color to Green");
              ledstrip.setLedColorData(0, 0, 255, 0);
              currentRed = 0;
              currentGreen = 255;
              currentBlue = 0;
              LEDState = "on";
              ledstrip.show();
            } else if (header.indexOf("GET /RGB/r0g0b255") >= 0) {
              Serial.println("Set color to Blue");
              ledstrip.setLedColorData(0, 0, 0, 255);
              currentRed = 0;
              currentGreen = 0;
              currentBlue = 255;
              LEDState = "on";
              ledstrip.show();
            } else if (header.indexOf("GET /RGB/r255g255b255") >= 0) {
              Serial.println("Set color to White");
              ledstrip.setLedColorData(0, 255, 255, 255);
              currentRed = 255;
              currentGreen = 255;
              currentBlue = 255;
              LEDState = "on";
              ledstrip.show();
            } else if (header.indexOf("GET /RGB/r255g0b125") >= 0) {
              Serial.println("Set color to Pink");
              ledstrip.setLedColorData(0, 255, 0, 125);
              currentRed = 255;
              currentGreen = 0;
              currentBlue = 125;
              LEDState = "on";
              ledstrip.show();
            }
            else if (header.indexOf("GET /temp") >= 0) {
              Serial.println("GET TEMP");

            get_temp_to_http();

            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server by wojtron</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 26  
            client.println("<p>GPIO 26 - State " + String(digitalRead(output26) == HIGH ? "on" : "off") + "</p>");
            if (digitalRead(output26) == LOW) {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // Display LED state
            client.println("<p>LEDState: " + LEDState + "</p>");
            client.println("<p><a href=\"/LEDs/brightness/on\"><button class=\"button\">Bright 100</button></a></p>");
            client.println("<p><a href=\"/LEDs/brightness/30\"><button class=\"button\">Bright 30</button></a></p>");
            client.println("<p><a href=\"/LEDs/brightness/off\"><button class=\"button button2\">Bright 0</button></a></p>");
            
            // Display color control buttons
            client.println("<p><a href=\"/RGB/r255g0b0\"><button class=\"button\">Red</button></a><a href=\"/RGBall/red\"><button class=\"button\">Red all</button></a></p>");
            client.println("<p><a href=\"/RGB/r0g255b0\"><button class=\"button\">Green</button></a><a href=\"/RGBall/green\"><button class=\"button\">Green all</button></a></p>");
            client.println("<p><a href=\"/RGB/r0g0b255\"><button class=\"button\">Blue</button></a><a href=\"/RGBall/blue\"><button class=\"button\">Blue all</button></a></p>");
            client.println("<p><a href=\"/RGB/r255g255b255\"><button class=\"button\">White</button></a></p>");
            client.println("<p><a href=\"/RGB/r255g0b125\"><button class=\"button\">Pink</button></a><a href=\"/RGBall/pink\"><button class=\"button\">Pink all</button></a></p>");

            client.println("<p></p> <p></p>");
            
          
            client.println("<p><a href=\"/temp\"><button class=\"button\">Get temp</button></a></p>");
            /*
            if (temperatureC =! 0) {
            //client.println("<p>TEMP IS " temperatureC " C </p>");

            client.println("<p>TEMP IS: " + String(temperatureC) + "</p>");
            }
          */
            
            client.println("</body></html>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
        currentTime = millis(); // Update time on each client action
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}




void get_temp_to_http(void){
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);


  Serial.print(temperatureC);
  Serial.println("ºC");
}

float get_temp_to_http_ext(void){
  sensors.requestTemperatures(); 
  float temperatureC2 = sensors.getTempCByIndex(0);


  return temperatureC2;
}

/*
float get_temp_as_float(void){

}
*/
