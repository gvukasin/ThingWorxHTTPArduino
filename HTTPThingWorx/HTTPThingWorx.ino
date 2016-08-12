// This script is a combination of code from George and the ThingWorxWifi Library, modified to use the WiFi101 library 
// instead of the WiFi library that the ThingWorxWifi Library relies on

#include <SPI.h>
#include <WiFi101.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <WiFiSSLClient.h>


// sensor information
#define sensorCount 4  //How many values you will be pushing to ThingWorx

double sensorValues[sensorCount];
char *nameArray[] = {"Photocell","Potentiometer","Temperature","Button"};
int sensorPins[sensorCount] = {0,1,2,8};
String pinType[sensorCount] = {"a","a","a","d"};


//wifi connection information
char ssid[] = "Tufts_Wireless"; //name of your WiFi network
char pass[] = "";   //password for WiFi network
int keyIndex = 0;  // your network key Index number (needed only for WEP)
       
byte mac[6];
int status = WL_IDLE_STATUS;

WiFiClient client; // initialize WiFi client library
IPAddress myIP; //IP address of remote Edge device

//Initialize ThingWorx server and app key
char server[] = "52.202.159.58"; //ThingWorx server (do not include http://)
int port=80; //port number for your server, 80 is default
char appKey[] = "0aae8b5f-871a-4973-bdf2-199fabb27dcb";   //ThingWorx Application Key
char thingName[] = "GenericEdge";                     //Name of your Thing in ThingWorx
char serviceName[] = "UpdateValues";   //Name of your Service (blank for now)


// Setup Loop 
void setup() {
  //setup connection
  Serial.begin(9600);
   while (!Serial);
  Serial.println("Connecting...");
  status = WiFi.begin(ssid);
  Serial.println(WiFi.status());

  if ( status != WL_CONNECTED) { 
    Serial.println("Couldn't get a wifi connection");
    while(true);
  } 
  else {
    Serial.println("Connected to wifi");
    Serial.println("\nStarting connection...");

      if (client.connect(server, port)) {
        Serial.println("connected");
        delay(2000);
        client.write("arduino\r\n"); // why does ThingWorxWiFi send this?
        Serial.println("sent");
        delay(2000);
        Serial.println(client.available());
        while(client.available()){
           char c = client.read();
           port += c;
        }
        client.stop();
        Serial.println("Port is:" );
        Serial.print(port);
      if(client.connect(server,port)){
        Serial.print(" assigned port and connected");
      }    
    } else Serial.print("cant connect");
  }
   //setup pin modes
  for(int i=0;i<sensorCount;i++) {
    if(pinType[i]=="d"){
      pinMode(sensorPins[i],INPUT);
      Serial.println("Digital"+String(i));
    } else if(pinType[i]=="a") {
      Serial.println("Analog"+String(i));
    } else {
      Serial.println("Have incorrect pin type for sensor " + String(i));
    }
  }
}


// Program Loop
void loop() {
  for(int i=0;i<sensorCount;i++) {
    if(pinType[i]=="d"){
      sensorValues[i] = digitalRead(sensorPins[i]);
      Serial.println("d"+String(i) + "=" + digitalRead(sensorPins[i]));
    } else if(pinType[i] == "a") {
      sensorValues[i] = analogRead(sensorPins[i]);
      Serial.println("a"+String(i) + "=" + analogRead(sensorPins[i]));
    } else {
      Serial.println("Have incorrect pin type for sensor " + String(i));
    }
  }

  UpdateValues(sensorValues,client,server,port,appKey,thingName,serviceName,nameArray,sensorCount);
  delay(1000);
}



// Update Values Function taken from "ThingWorxWiFi.cpp" in ThingWorxWiFi library and modified
void UpdateValues(double propValues[], WiFiClient &client, char server[], int port, char aKey[], char thing[], char service[], char* sensNames[], int sensCount)
{
 if (client.connect(server, 80)) {
//    // send HTTP PUT request to send sensor values one by one to ThingWorx:
//    for(i=0; i<sensCount; i++){
//      String putRequest = "PUT /Thingworx/Things/" + thing + "/Properties/" + sensNames[i] +"?method=put&appKey=" + aKey + "&value=" + String(sensorValue[i]);
//      client.println(putRequest);
//      client.println(" HTTP/1.1");
//      client.print("Host: " + server + ":" + String(port));
//      client.println();
//    }
    
      
    // send the HTTP POST request for services to send all sensor values at once:
    String postString = "POST /Thingworx/Things/" + String(thing) + "/Services/" + String(service) + "?appKey=" + aKey + "&method=post&x-thingworx-session=true<";
    for(int i=0;i<sensCount;i++){
      postString = postString + "&" + sensNames[i] + "=" + propValues[i];
    }
    postString = postString + ">";
    client.println(postString);
    Serial.println(postString);
    
    client.println(" HTTP/1.1");
    client.print("Host: " + String(server) + ":" + String(port));
    client.println();

//// send HTTP GET request to receive values from ThingWorx:
//    String getRequest = "GET /Thingworx/Things/" + String(thing) + "/Properties?method=get&appKey=" + aKey;
//    client.println(getRequest);
//    client.println(" HTTP/1.1");
//    client.print("Host: " + String(server) + ":" + String(port));
//    client.println();
    
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
  
  client.stop();
  } else {
    // if you couldn't make a connection:
    client.stop();
  }
}


