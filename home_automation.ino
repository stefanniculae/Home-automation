#include <WiFi.h>
#include <SimpleDHT.h>
#include <string>

const char* ssid = "VL_28";
const char* password =  "tenimtenim";
WiFiServer server(80);
String header;
String light_state = "off";
String fan_state = "off";
const int LIGHT_PIN = 22;
const int FAN_PIN = 23;
const int TEMP_PIN = 4;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
unsigned long current_time_sensor = millis();
unsigned long previous_time_sensor = 0;
const long timeoutTime = 2000;
WiFiClient client;
SimpleDHT11 dht11;
int temperature_in_the_room = 0;

void setup() {

  Serial.begin(115200);

  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);

  wifi_connect();
}

void loop() {
  temperature_in_the_room = sensor_check(); 
  if(temperature_in_the_room >= 26){
    digitalWrite(FAN_PIN, HIGH);
  }
  else{
    digitalWrite(FAN_PIN, LOW);
  } 
  client = server.available();  

  if (client) {                             
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client found.");         
    String currentLine = "";               
    while (client.connected() && currentTime - previousTime <= timeoutTime) { 
      currentTime = millis();
      if (client.available()) {             
        char c = client.read();             
        Serial.write(c);                    
        header += c;
        if (c == '\n') {            
          if (currentLine.length() == 0) {
            if (header.indexOf("GET /22/on") >= 0) {
              Serial.println("GPIO 22 On");
              light_state = "on";
              digitalWrite(LIGHT_PIN, HIGH);
            } else if (header.indexOf("GET /22/off") >= 0) {
              Serial.println("GPIO 22 Off");
              light_state = "off";
              digitalWrite(LIGHT_PIN, LOW);
            }

            html_page();
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

void wifi_connect() {
  Serial.print("Connecting to Network:");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected. The device can be found at IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void html_page() {
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  // CSS to style the on/off buttons
  client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
  client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
  client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
  client.println(".button2 {background-color: #555555;}</style></head>");
  client.println("<body><h1>ESP32 Web Server</h1>");
  client.println("<p>GPIO 22 - Currently " + light_state + "</p>");
  if (light_state == "off") {
    client.println("<p><a href=\"/22/on\"><button class=\"button\">ON</button></a></p>");
  } else {
    client.println("<p><a href=\"/22/off\"><button class=\"button button2\">OFF</button></a></p>");
  }

  if(temperature_in_the_room >= 26){
    fan_state = "ON";
  }
  else{
    fan_state = "OFF";
  }

  client.println("<p>Fan is: " + fan_state + "</p>");
  
  client.println("</body></html>");
}

int sensor_check(){
  current_time_sensor = millis();
  byte t=0;
  byte h=0;
  if(current_time_sensor - previous_time_sensor >= 2000){
    previous_time_sensor = current_time_sensor;
    int err=dht11.read(TEMP_PIN, &t, &h, NULL);
    Serial.println(t);
    return t;
  }
  return temperature_in_the_room;
}
