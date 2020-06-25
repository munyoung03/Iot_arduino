#include "WiFiEsp.h"
#include <DHT11.h>  

// Emulate Serial1 on pins 6/7 if not present

#ifndef HAVE_HWSERIAL1

#endif

#include "SoftwareSerial.h"

SoftwareSerial Serial1(3,2); // RX, TX 

int pin = 4;
int soil_humi = A0;
  
int soil_humi_value;  

DHT11 dht11(pin);

float temp, humi;
int i;

char ssid[] = "dgsw_embd_1";            // your network SSID (name)

char pass[] = "dgswdgsw";        // your network password

int status = WL_IDLE_STATUS;     // the Wifi radio's status



char server[] = "munyoung.kro.kr";



// Initialize the Ethernet client object

WiFiEspClient client;



void setup()

{

  // initialize serial for debugging

  Serial.begin(9600);

  // initialize serial for ESP module

  Serial1.begin(9600);

  // initialize ESP module

  WiFi.init(&Serial1);

  // check for the presence of the shield

  if(i = dht11.read(humi, temp) ==0){
    soil_humi_value = analogRead(soil_humi);
    soil_humi_value = map(soil_humi_value,550,0,0,100); 
    Serial.println(temp);
    Serial.println(soil_humi_value);
    connectWifi(humi, temp, soil_humi_value);
  }
  else{
    Serial.println("실패");
  }
}


void connectWifi(float humi, float temp, int soil_humi_value){
   if (WiFi.status() == WL_NO_SHIELD) {

    Serial.println("WiFi shield not present");
  
    // don't continue

    while (true);

  }

  // attempt to connect to WiFi network

  while ( status != WL_CONNECTED) {

    Serial.print("Attempting to connect to WPA SSID: ");

    Serial.println(ssid);

    // Connect to WPA/WPA2 network

    status = WiFi.begin(ssid, pass);

  }

  // you're connected now, so print out the data

  Serial.println("You're connected to the network");

  printWifiStatus();

  Serial.println();

  Serial.println("Starting connection to server...");

  // if you get a connection, report back via serial

  if (client.connect(server, 3000)) {

    Serial.println("Connected to server");

    // Make a HTTP request

    client.print("GET /update?temp=");
    client.print(temp);
    client.print("&humi=");
    client.print(humi);
    client.print("&soilhumi=");
    client.print(soil_humi_value);  
    client.print("&id=1");
    client.println(" HTTP/1.1");
    client.println("Host: 54.236.26.160/");

    client.println("Connection: close");

    client.println();

  }
}
void loop()

{
  
  // if there are incoming bytes available

  // from the server, read them and print them

  while (client.available()) {

    char c = client.read();

    Serial.write(c);

  }



  // if the server's disconnected, stop the client

  if (!client.connected()) {

    Serial.println();

    Serial.println("Disconnecting from server...");

    client.stop();



    // do nothing forevermore

    while (true);

  }

}




void printWifiStatus()

{

  // print the SSID of the network you're attached to

  Serial.print("SSID: ");

  Serial.println(WiFi.SSID());



  // print your WiFi shield's IP address

  IPAddress ip = WiFi.localIP();

  Serial.print("IP Address: ");

  Serial.println(ip);



  // print the received signal strength

  long rssi = WiFi.RSSI();

  Serial.print("Signal strength (RSSI):");

  Serial.print(rssi);

  Serial.println(" dBm");

}
