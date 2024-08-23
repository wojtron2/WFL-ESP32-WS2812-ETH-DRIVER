#include "Freenove_WS2812_Lib_for_ESP32.h"
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Time.h>

#define LEDS_COUNT 1
#define LEDS_PIN 27
#define CHANNEL 0

Freenove_ESP32_WS2812 ledstrip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xFE, 0xFE, 0xBE, 0xFE, 0xDE, 0xED
};
IPAddress ip(192, 168, 2, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output26State = "off";
String LEDState = "off";

// Assign output variables to GPIO pins
const int output26 = 26;
const int output25 = 25;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  pinMode(output26, OUTPUT);
  pinMode(output25, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output26, LOW);
  digitalWrite(output25, LOW);
  Ethernet.init(5);  //ESP32

  ledstrip.begin();
  ledstrip.setBrightness(100);

  ledstrip.setLedColorData(0, 0, 0, 0);
  ledstrip.show();

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Ethernet WebServer Example");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "on";
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              digitalWrite(output26, LOW);
            } else if (header.indexOf("GET /RGB/") >= 0) {  // Sprawdzenie żądania RGB
              Serial.println("Ustawianie koloru LED");
              int r, g, b;
              parseRGB(header, r, g, b);  // Parsowanie wartości RGB
              ledstrip.setLedColorData(0, r, g, b);
              ledstrip.show();
              // Nie zmieniaj stanu LEDState tutaj, bo kolor może być dowolny
            } else if (header.indexOf("GET /LEDs/on") >= 0) {
              Serial.println("LED on");
              ledstrip.setLedColorData(0, 255, 255, 255);  // Przykładowo ustawiamy na biały
              ledstrip.show();
              LEDState = "on";
            } else if (header.indexOf("GET /LEDs/off") >= 0) {
              Serial.println("LED off");
              ledstrip.setLedColorData(0, 0, 0, 0);
              ledstrip.show();
              LEDState = "off";
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server by wojtron</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 26  
            client.println("<p>GPIO 26 - State " + output26State + "</p>");
            // If the output26State is off, it displays the ON button       
            if (output26State=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for LED  
            client.println("<p>LEDState " + LEDState + "</p>");
            // If the LEDState is off, it displays the OFF button       
            if (LEDState=="off") {
              client.println("<p><a href=\"/LEDs/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/LEDs/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // Provide a link for setting RGB color
            client.println("<p>Set RGB Color:</p>");
            client.println("<p><a href=\"/RGB/r255g0b0\"><button class=\"button\">Red</button></a>");
            client.println("<a href=\"/RGB/r0g255b0\"><button class=\"button\">Green</button></a>");
            client.println("<a href=\"/RGB/r0g0b255\"><button class=\"button\">Blue</button></a></p>");

            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

// Funkcja do parsowania wartości RGB z żądania
void parseRGB(String header, int &red, int &green, int &blue) {
  int rIndex = header.indexOf('r');
  int gIndex = header.indexOf('g');
  int bIndex = header.indexOf('b');
  
  if (rIndex > 0 && gIndex > 0 && bIndex > 0) {
    String rStr = header.substring(rIndex + 1, gIndex);  // Poprawka do wyciągania wartości czerwonej
    String gStr = header.substring(gIndex + 1, bIndex);  // Poprawka do wyciągania wartości zielonej
    String bStr = header.substring(bIndex + 1);          // Wyciągnij wartość niebieską aż do końca
    
    // Konwersja wartości na liczby całkowite
    red = rStr.toInt();
    green = gStr.toInt();
    blue = bStr.toInt();
  } else {
    // Domyślne wartości, jeśli coś poszło nie tak
    red = 0;
    green = 0;
    blue = 0;
  }
}
