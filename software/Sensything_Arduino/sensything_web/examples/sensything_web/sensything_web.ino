/*
 * Sensything web interface 
 */
 
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

#include "index.h"  //Web page header file
#include <SPI.h>
#include "Protocentral_ADS1220.h"

#define PGA          1                 // Programmable Gain = 1
#define VREF         2.048            // Internal reference of 2.048V
#define VFSR         VREF/PGA
#define FULL_SCALE   (((long int)1<<23)-1)

#define ADS1220_CS_PIN    4
#define ADS1220_DRDY_PIN  34

Protocentral_ADS1220 pc_ads1220;
int32_t adc_data;

WebServer server(80);

const char* ssid = "protocentral";
const char* password = "open1234";

void handleRoot() {
 String adcValue = MAIN_page; //Read HTML contents
 server.send(200, "text/html", adcValue); //Send web page
 }
 
void handleADC() {
  adc_data=pc_ads1220.Read_SingleShot_SingleEnded_WaitForData(MUX_SE_CH0);
  String adcValue = String(convertToMilliV(adc_data));
  server.send(200, "text/plane", adcValue); //Send ADC value only to client ajax request
 }
 float convertToMilliV(int32_t i32data)
 {
    return (float)((i32data*VFSR*1000)/FULL_SCALE);
 }


void setup(void){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Loading sensything web-interface...");

  pc_ads1220.begin(ADS1220_CS_PIN,ADS1220_DRDY_PIN);
  pc_ads1220.set_data_rate(DR_330SPS);
  pc_ads1220.set_pga_gain(PGA_GAIN_1);
  pc_ads1220.set_conv_mode_single_shot(); //Set Single shot mode

//Sensything access point
  WiFi.mode(WIFI_AP); //Access Point mode
  WiFi.softAP(ssid, password);

  WiFi.mode(WIFI_STA); //Connect to your wifi
  WiFi.begin(ssid, password);

  Serial.println("Connecting to ");
  Serial.print(ssid);

  while(WiFi.waitForConnectResult() != WL_CONNECTED){      
  Serial.print(".");
  }
    
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  

 
  server.on("/", handleRoot);      //This is display page
  server.on("/readADC", handleADC);//To get update of Sensything ADC Value 
  server.begin();                  //Start server
  Serial.println("HTTP server started");
 }

void loop(void){
  server.handleClient();
  delay(1);
 }
