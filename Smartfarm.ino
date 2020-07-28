#include "WiFiEsp.h"
#include "DHT.h" 
#include "SoftwareSerial.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//SoftwareSerial Serial1(3,2); // RX, TX 

LiquidCrystal_I2C lcd(0x27, 16, 2); //lcd 주소값 설정

int pin = 4; //dht 핀
int soil_humi = A0; //토양 습도 핀
int soil_humi_value;  //토양 습도 값
DHT dht(4, DHT22); //dht 내장 함수

unsigned long count = 0; //서버 보낼 때 카운트

float temp, humi; //온도 습도
int timer1_count; //서버 카운트
int timer3_count; //펜 카운트
int light; //조도

char ssid[] = "dgsw_embd_1";      // Wifi 이름

char pass[] = "dgswdgsw";        // Wifi 비번

int status = WL_IDLE_STATUS;     // 연결 확인 유뮤 변수

char server[] = "munyoung.kro.kr"; //서버 호스트

WiFiEspClient client; //클라이언트 값 보낼 때 쓰는 변수

//--------------------------------------------------------------

//setup
void setup()

{ 
  Wire.begin(); //Wire 시작
  Serial.begin(9600); //Serial 시작
  dht.begin(); //dht 시작

  startDisplay(); //디스플레이 시작(함수)

  //-----------------------------------
  //서버 통신

  //WiFi.init(&Serial1);
  
  //connectWifi();
  
  //-----------------------------------
  //타이머
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
  
  //-----------------------------------
  //pinMode
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(12, OUTPUT);
}

//loop
void loop()
{
  soil_humi_value = analogRead(soil_humi); //토양 습도 값 받아오기
  soil_humi_value = map(soil_humi_value, 550, 0, 0, 100); //아날로그 데이터 변환
  
  temp = dht.readTemperature(); //온도 받아오기
  humi = dht.readHumidity(); //습도 받아오기
  
  light = analogRead(A1); //밝기 받아오기
  
  //시리얼 출력
  Serial.println(temp); 
  Serial.print("jodo : ");
  Serial.println(light);
  
  //현재 온도 현재 습도 LCD출력
  printHT();

  //모터 드라이버 HIGH
  digitalWrite(12, HIGH);
  //delay(1000);

}

//-------------------------------------------------------------------
//데이터 보내기
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

//와이파이 연결
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

//wifi
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

//ISR
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

//ISR
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

//----------------------------------------------
//토양 습도에 따라 모터 펌프 작동시키는 함수
void working_pump(){
   delay(5000);
   digitalWrite(12, LOW);
}


//----------------------------------------------
//lcd 함수

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

//현재 온도 현재 습도 LCD출력
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