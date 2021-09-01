
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Wire.h> 
#include <Adafruit_GFX.h> 
#include "Adafruit_LEDBackpack.h" 
#include <SparkFun_APDS9960.h>
#include "index.h"  //Web page header file
#define LM73_ADDR 0x4D


#define SEND_DELAY 2000

SparkFun_APDS9960 apds = SparkFun_APDS9960();
int isr_flag = 0;
int analog_value = 0;
int outputValue = 0;  
double temp=0;   
Adafruit_8x16minimatrix matrix = Adafruit_8x16minimatrix(); 
WebServer server(80);

//Enter your SSID and PASSWORD
const char* ssid = "";
const char* password = "";
const int analogInPin = 36; 
//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}
float readTemperature() {
  Wire1.beginTransmission(LM73_ADDR);
  Wire1.write(0x00); // Temperature Data Register
  Wire1.endTransmission();
 
  uint8_t count = Wire1.requestFrom(LM73_ADDR, 2);
  float temp = 0.0;
  if (count == 2) {
    byte buff[2];
    buff[0] = Wire1.read();
    buff[1] = Wire1.read();
    temp += (int)(buff[0]<<1);
    if (buff[1]&0b10000000) temp += 1.0;
    if (buff[1]&0b01000000) temp += 0.5;
    if (buff[1]&0b00100000) temp += 0.25;
    if (buff[0]&0b10000000) temp *= -1.0;
  }
  return temp;
} 
void handleADC() {
 int a = analogRead(analogInPin);
 temp = readTemperature();
 String adcValue = "LDR = "+String(a)+ " , Temp =" + String(temp) + " C";
 



 
 server.send(200, "text/plane", adcValue); //Send ADC value only to client ajax request
}

//===============================================================
// Setup
//===============================================================

void setup(void){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  analogReadResolution(10);
  Wire1.begin(4, 5);
  matrix.begin(0x70);
  matrix.setRotation(1); 
  matrix.setTextSize(1); 
  matrix.setTextWrap(false); 
  matrix.setTextColor(LED_ON);
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
  
  // Start running the APDS-9960 gesture sensor engine
  if ( apds.enableGestureSensor(true) ) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }
/*
//ESP32 As access point
  WiFi.mode(WIFI_AP); //Access Point mode
  WiFi.softAP(ssid, password);
*/
//ESP32 connects to your wifi -----------------------------------
  WiFi.mode(WIFI_STA); //Connectto your wifi
  WiFi.begin(ssid, password);

  Serial.println("Connecting to ");
  Serial.print(ssid);

  //Wait for WiFi to connect
  while(WiFi.waitForConnectResult() != WL_CONNECTED){      
      Serial.print(".");
    }
    
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
//----------------------------------------------------------------
 
  server.on("/", handleRoot);      //This is display page
  server.on("/readADC", handleADC);//To get update of ADC Value only
 
  server.begin();                  //Start server
  Serial.println("HTTP server started");
}
void matrixTemp(){
  temp = readTemperature();
  matrix.clear();
  matrix.setCursor(3,0);
  String response = String(temp);
  Serial.println(response);
  matrix.print(response); 
  matrix.writeDisplay();
}
void matrixLDR(){
  int a = analogRead(analogInPin);
  outputValue = map(a, 0, 1023, 0, 99);  
  matrix.clear();
  matrix.setCursor(3,0);
  String response = String(outputValue);
  Serial.println(response);
  matrix.print(response); 
  matrix.writeDisplay();
}

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void loop(void){
  int a = analogRead(analogInPin);
  temp = readTemperature();
  outputValue = map(a, 0, 1023, 0, 99);  
  Serial.print("sensor = ");     Serial.print(a);  
  Serial.print("\t output = ");  Serial.println(outputValue); 
  Serial.print("temp = "); Serial.println(temp);
  //Display value on LED Matrix


  delay(SEND_DELAY);

    matrixTemp();
      if(apds.readGesture() == DIR_LEFT){
         Serial.println("LEFT");
        matrixLDR();
      }
      else{
        matrixTemp();
      }
    
  
 

  server.handleClient();
  delay(1);
}
