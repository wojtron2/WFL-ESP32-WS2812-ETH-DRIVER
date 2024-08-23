#include "Freenove_WS2812_Lib_for_ESP32.h"
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Time.h>

#define LEDS_COUNT 1
#define LEDS_PIN 27
#define CHANNEL 0

Freenove_ESP32_WS2812 ledstrip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

byte mac[] = { 0xFE, 0xFE, 0xBE, 0xFE, 0xDE, 0xED };
IPAddress ip(192, 168, 2, 177);

EthernetServer server(80);
String header;

// Zmienna do przechowywania aktualnego koloru LED
int currentRed = 0;
int currentGreen = 0;
int currentBlue = 0;

const int output26 = 26;
const int output25 = 25;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

void setup() {
  pinMode(output26, OUTPUT);
  pinMode(output25, OUTPUT);
  digitalWrite(output26, LOW);
  digitalWrite(output25, LOW);
  Ethernet.init(5);

  ledstrip.begin();
  ledstrip.setBrightness(100);
  ledstrip.setLedColorData(0, 0, 0, 0);
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
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";
    
    while (client.connected()) {
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
            
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              digitalWrite(output26, LOW);
            } else if (header.indexOf("GET /LEDs/on") >= 0) {
              Serial.println("LED on");
              ledstrip.setLedColorData(0, 255, 0, 125);
              currentRed = 255;
              currentGreen = 0;
              currentBlue = 125;
              ledstrip.show();
            } else if (header.indexOf("GET /LEDs/off") >= 0) {
              Serial.println("LED off");
              ledstrip.setLedColorData(0, 0, 0, 0);
              currentRed = 0;
              currentGreen = 0;
              currentBlue = 0;
              ledstrip.show();
            } else if (header.indexOf("GET /RGB/") >= 0) {
              int red, green, blue;
              parseRGB(header, red, green, blue);
              //Serial.printf("Setting LED to R:%d G:%d B:%d\n", red, green, blue);
              ledstrip.setLedColorData(0, red, green, blue);
              currentRed = red;
              currentGreen = green;
              currentBlue = blue;
              ledstrip.show();
            }

            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            client.println("<body><h1>ESP32 Web Server by wojtron</h1>");
            client.println("<p>GPIO 26 - State " + String(digitalRead(output26) == HIGH ? "on" : "off") + "</p>");
            client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            
            client.println("<p>LEDState " + String(currentRed) + "," + String(currentGreen) + "," + String(currentBlue) + "</p>");
            client.println("<p><a href=\"/LEDs/on\"><button class=\"button button2\">OFF</button></a></p>");
            client.println("<p><a href=\"/LEDs/off\"><button class=\"button\">ON</button></a></p>");

            client.println("<p><a href=\"/RGB/r255g0b0\"><button class=\"button\">Red</button></a></p>");
            client.println("<p><a href=\"/RGB/r0g255b0\"><button class=\"button\">Green</button></a></p>");
            client.println("<p><a href=\"/RGB/r0g0b255\"><button class=\"button\">Blue</button></a></p>");
            
            client.println("</body></html>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void parseRGB(String header, int &red, int &green, int &blue) {
  int rIndex = header.indexOf('r');
  int gIndex = header.indexOf('g');
  int bIndex = header.indexOf('b');
  
  if (rIndex > 0 && gIndex > 0 && bIndex > 0) {
    String rStr = header.substring(rIndex + 1, gIndex);  
    String gStr = header.substring(gIndex + 1, bIndex);  
    String bStr = header.substring(bIndex + 1);   
    
    red = rStr.toInt();
    green = gStr.toInt();
    blue = bStr.toInt();
  } else {
    red = 0;
    green = 0;
    blue = 0;
  }
}
