
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

#include "index.h"  //Web page header file
#include "SparkFun_BNO080_Arduino_Library.h"
#include <Wire.h>

String dataString;

BNO080 myIMU;

WebServer server(80);

//Enter your SSID and PASSWORD
const char* ssid = "protocentral";
const char* password = "open1234";

void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}
 
void handleQwiic() {
  
  if (myIMU.dataAvailable() == true)
  {
      dataString = myIMU.getAccelX() + String(",") 
     + myIMU.getAccelY() + String(",") 
     + myIMU.getAccelZ() + String(",");
  }
  
     server.send(200, "text/plane", dataString); //Send ADC value only to client ajax request
     Serial.println(dataString); //Store x on SD card
     Serial.print("\t");
  }
  

void setup(void){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  Wire.begin(21, 22, 400000);
  myIMU.begin();
  Wire.setClock(400000); //Increase I2C data rate to 400kHz
  myIMU.enableAccelerometer(50); //Send data update every 50ms

  Serial.println(F("Accelerometer enabled"));

  

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
  server.on("/readQwiic", handleQwiic);//To get update of ADC Value only
  server.begin();                  //Start server
  Serial.println("HTTP server started");
}


void loop(void){
  server.handleClient();
  delay(1);
}
