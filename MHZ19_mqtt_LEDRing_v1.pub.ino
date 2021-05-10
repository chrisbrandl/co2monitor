#include <Arduino.h>
#include "MHZ19.h"                                        
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#define RX_PIN 0         //MH-Z19 RX-PIN                                         
#define TX_PIN 2         //MH-Z19 TX-PIN                                   
#define SCL_PIN 5         //OLED SCL-PIN                                         
#define SDA_PIN 4         //OLED SDA-PIN                                   
#define BAUDRATE 9600    //Terminal Baudrate
#define SCHWELLE 1500 //Schwellwert wenn die LED von Gelb auf Rot wechseln soll.
#define RICHTWERT 1000 //Schwellwert wenn die LED von Grün auf Gelb wechseln soll.
#define OLED_RESET -1  // GPIO0 , -1 not used

//Klassen definierung
MHZ19 myMHZ19;                               
SoftwareSerial mySerial(RX_PIN, TX_PIN); 
unsigned long getDataTimer = 0;
int CO2 = 0; 
int8_t Temp = 0;
int CO2MaxPerDay = 0;
int CO2MaxOverall = 0;
int CO2AvgPerDay = 0;
int CO2AvgOverall = 0;
int readCounter = 0;
unsigned long getSendingTimer = 0;

// Wifi setup
#define WLAN_SSID  "******" 
#define WLAN_PASS  "******" 

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
WiFiClient client; 
#define MQTT_NAME "MQTTNAME" 
#include <PubSubClient.h>
const char* MQTT_BROKER = "******";
PubSubClient mqttClient(client);
long lastMsg = 0;
char msg[50];
int value = 0;

// Define NTP Client to get time
#include "NTPClient.h"
#include "WiFiUdp.h"
const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

#include <Adafruit_NeoPixel.h>
#define PIN D5
#define NUMPIXELS 12
#define DELAYVAL 100 // Time (in milliseconds) to pause between pixels
#define BRIGHTNESS 10 // change brightness here – 255 for full brightness
int R = 253;
int G = 255;
int B = 56;
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// OLED Display 128x64 
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

void setup(){
  Serial.begin(115200);
  delay(500);
  Serial.println("Start");
    //LED PIN Mode festlegen
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    pixels.setPixelColor(i, pixels.Color(R,G,B)); 
    pixels.setBrightness(BRIGHTNESS);
    pixels.show();   // Send the updated pixel colors to the hardware.
  }
    
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr //0x3D (128x64), 0x3C (for the 64x48)
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Connecting...");
  display.display();    
  delay(500);

  //Serielle Intialisierung
  mySerial.begin(BAUDRATE);                   
  myMHZ19.begin(mySerial);
  myMHZ19.autoCalibration(); // Turn auto calibration ON (OFF autoCalibration(false))

  delay(1000);

    /* Alternative manual calibration
    myMHZ19.autoCalibration(false);     // make sure auto calibration is off
    Serial.print("ABC Status: "); myMHZ19.getABC() ? Serial.println("ON") :  Serial.println("OFF");  // now print it's status
    Serial.println("Waiting 20 minutes to stabalise...");
   // if you don't need to wait (it's already been this amount of time), remove the 2 lines 
    timeElapse = 12e5;   //  20 minutes in milliseconds
    delay(timeElapse);    //  wait this duration
    Serial.println("Calibrating..");
    myMHZ19.calibrate();    // Take a reading which be used as the zero point for 400 ppm 
    */
    
  connectToWiFi(); 

  mqttClient.setServer(MQTT_BROKER, 1883);
  //mqttClient.setCallback(callback);
  while (!mqttClient.connected()) {
    Serial.print("MQTT ");
    if (mqttClient.connect(MQTT_NAME)) {
      Serial.println("connected");  
    } else {
      Serial.print("failed with state ");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
  //timeClient.begin();
  delay(1000);
}

void loop() {

  //Prüfung ob 2 Sekunden um sind.
  if (millis() - getDataTimer >= 2000){
    CO2 = myMHZ19.getCO2();
    readCounter++;
    CO2AvgOverall = (CO2AvgOverall + CO2) / readCounter;
    if (CO2 > CO2MaxPerDay) {
      CO2MaxPerDay = CO2;
    }
    if (CO2 > CO2MaxOverall) {
      CO2MaxOverall = CO2;
    }
    if (CO2 > SCHWELLE) { 
        R = 255;
        G = 0;
        B = 0;
    } else if (CO2 > RICHTWERT) { 
        R = 255;
        G = 127;
        B = 80;
    } else if (CO2 == 0) {
        R = 255;
        G = 255;
        B = 0;
    } else {
        R = 0;
        G = 255;
        B = 0;
    }
    //Ausgabe auf den Seriellen Monitor
    Serial.print("CO2 (ppm): ");
    Serial.println(CO2);

    Temp = myMHZ19.getTemperature();                     
    // Request Temperature (as Celsius)
    Serial.print("Temperature (C): ");                  
    Serial.println(Temp);                               

    getDataTimer = millis();
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    //display.setFont(&FreeSans12pt7b);
    display.setCursor(0,0);
    display.print("CO2: ");
    display.println(CO2);
    display.setCursor(0,17);
    display.print("Max: ");
    display.println(CO2MaxOverall);
    display.setCursor(0,34);
    display.print("Temp: ");
    display.println(Temp);
    display.display();  

    for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
      pixels.setPixelColor(i, pixels.Color(R,G,B)); 
      pixels.setBrightness(BRIGHTNESS);
      pixels.show();   // Send the updated pixel colors to the hardware.
    }
  }
  if (millis() - getSendingTimer >= 60000) {
    int str_len = String(CO2).length() + 1; 
    // Prepare the character array (the buffer) 
    char char_array[str_len];
    String(CO2).toCharArray(char_array, str_len);
    if(! mqttClient.connected()) {
      if (mqttClient.connect("coronx")) {
        Serial.println("connected");  
      } else {
        Serial.print("failed with state ");
        Serial.print(mqttClient.state());
        delay(2000);
      }
    } 
    mqttClient.publish("data/co2", char_array);
    Serial.print("Publish (CO2) to mqtt = ");
    Serial.println(char_array);

    int str_len2 = String(Temp).length() + 1; 
    char char_array2[str_len2];
    String(Temp).toCharArray(char_array2, str_len2);
    mqttClient.publish("data/temp", char_array2);
    Serial.print("Publish (Temp) to mqtt = ");
    Serial.println(char_array2);

    getSendingTimer = millis();
  }
  delay(1000);  
}

void connectToWiFi() { 
  // Connect to WiFi access point. 
  delay(10); 
  Serial.print(F("Connecting to ")); 
  Serial.println(WLAN_SSID); 
  WiFi.begin(WLAN_SSID, WLAN_PASS); 
  int timeoutCounter = 0;
  while ( WiFi.status() != WL_CONNECTED && timeoutCounter <= 12) {
    //sometimes it is not connecting properly
    timeoutCounter++;
    Serial.print(".");
    delay(500);
  }
  if ( WiFi.status() != WL_CONNECTED ) {
    Serial.println(F("NOT connected!")); 
    //Serial.println("Restarting ESP as WiFi can not connect"); 
    //ESP.restart();
  } else {
    Serial.println(F("WiFi connected!")); 
  }
} 
