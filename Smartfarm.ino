#include "WiFiEsp.h"
#include "DHT.h" 


#ifndef HAVE_HWSERIAL1

#endif

#include "SoftwareSerial.h"

//SoftwareSerial Serial1(3,2); // RX, TX 

int pin = 4;

int light;
int soil_humi = A0;
int soil_humi_value;  

DHT dht(4, DHT22);  

float temp, humi;
int i;

char ssid[] = "dgsw_embd_1";            // your network SSID (name)

char pass[] = "dgswdgsw";        // your network password

int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "munyoung.kro.kr";

WiFiEspClient client;


void setup()

{

  Serial.begin(9600);

  dht.begin();

  ///WiFi.init(&Serial1);

  //connectWifi();
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(5, OUTPUT);

}


void sendData(float humi, float temp, int soil_humi_value){
  
  if (client.connect(server, 3000)) {

    Serial.println("Connected to server");
    
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

void connectWifi(){
   if (WiFi.status() == WL_NO_SHIELD) {

    Serial.println("WiFi shield not present");

    while (true);

  }

  while ( status != WL_CONNECTED) {

    Serial.print("Attempting to connect to WPA SSID: ");

    Serial.println(ssid);

    status = WiFi.begin(ssid, pass);

  }

  Serial.println("You're connected to the network");

  printWifiStatus();

  Serial.println();

  Serial.println("Starting connection to server...");

  
}
void loop()

{
  
  soil_humi_value = analogRead(soil_humi);
  soil_humi_value = map(soil_humi_value,550,0,0,100); 
  temp = dht.readTemperature();
  humi = dht.readHumidity();
  light = analogRead(A1);
  Serial.println(temp);
  Serial.print("jodo : ");
  Serial.println(light);
    
  if(temp >= 23)
  {
   
    digitalWrite(8,LOW);
    digitalWrite(7,LOW);
  }
  else
  {
    digitalWrite(8,HIGH);
    digitalWrite(7,HIGH);
    delay(1000);
    digitalWrite(8,LOW);
    digitalWrite(7,LOW);
    delay(300);
  }
        
  //if (!client.connected()) {
    //connectWifi();
    //}

  //sendData(humi, temp, soil_humi_value); 

  //delay(1000);

}




void printWifiStatus()

{


  Serial.print("SSID: ");

  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();

  Serial.print("IP Address: ");

  Serial.println(ip);

  long rssi = WiFi.RSSI();

  Serial.print("Signal strength (RSSI):");

  Serial.print(rssi);

  Serial.println(" dBm");

}
