#include "WiFiEsp.h"
#include "DHT.h" 
#ifndef HAVE_HWSERIAL1
#endif
#include "SoftwareSerial.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//SoftwareSerial Serial1(3,2); // RX, TX 

LiquidCrystal_I2C lcd(0x27, 16, 2); //lcd 주소값 설정
#define DS3231_I2C_ADDRESS 104

int pin = 4;
int soil_humi = A0;
int soil_humi_value;  
DHT dht(4, DHT22);

unsigned long count = 0;
float temp, humi;
int timer1_count;
int timer3_count;
int i;
int light;

char ssid[] = "dgsw_embd_1";            // your network SSID (name)

char pass[] = "dgswdgsw";        // your network password

int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "munyoung.kro.kr";

WiFiEspClient client;

void setup()

{ 
  Wire.begin();
  Serial.begin(9600);
  dht.begin();

  startDisplay();
  
  //WiFi.init(&Serial1);
  
  //connectWifi();
  
  TCCR1A = 0x00;
  TCCR1B = 0x0D;
  TCCR1C = 0x00;

  TCCR3A = 0x00;
  TCCR3B = 0x0C;
  TCCR3C = 0x00;

  OCR1A = 1500;
  OCR3A = 6250;
  
  TIMSK1 = 0x02;
  TIMSK3 = 0x02;
  
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(12, OUTPUT);
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
  printHT();
  digitalWrite(12, HIGH);
  //delay(1000);

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

//토양 습도에 따라 모터 펌프 작동시키는 함수
void working_pump(){
   delay(5000);
   digitalWrite(12, LOW);
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

ISR(TIMER1_COMPA_vect)
{
  timer1_count++;
  Serial.println(count);
  if(timer1_count == 3125){
    timer1_count = 0;
    if (!client.connected()) 
    {
      connectWifi();
    }

    sendData(humi, temp, soil_humi_value); 
  }
}

ISR(TIMER3_COMPA_vect)
{
  timer3_count++;
  if(timer3_count > 13)
    timer3_count = 0;
  if(temp >= 25)
  {
    digitalWrite(8,LOW);
  }
  else
  {
    if(timer3_count > 3)
    { 
      digitalWrite(8,HIGH);
    }
    if(timer3_count <= 3)
    { 
      digitalWrite(8,LOW);
    }
  }
}

//인트로 text 출력
void startDisplay(){
  lcd.init();
  lcd.backlight();
  
  lcd.begin(16, 2);
  lcd.setCursor(1,0);
  lcd.print("SmartFarm");
  lcd.setCursor(9, 1);
  lcd.print("ver 1.0");
  delay(1000);
  lcd.clear();
}

//현재 온도 현재 습도 출력
void printHT(){
  lcd.setCursor(0,0);
  lcd.print("Tem: ");
  lcd.print(temp);
  lcd.setCursor(0,1);
  lcd.print("Humi: ");
  lcd.print(humi);
  delay(1000);
  lcd.clear();
}