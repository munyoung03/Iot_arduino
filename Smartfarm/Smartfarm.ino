//라이브러리 선언부
#include "WiFisp.h"
#include "DHT.h"
#include "SoftwareSerial.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//-----------------------------------------------------------------

//define 선언부
//핀 설정
#define DHTPIN 4 //DHT 핀
#define SOILHUMIPIN A0 //토양 습도 핀
#define PINPUMP 13 //워터 펌프 핀
#define PINJODO A15 //조도 센서 핀
#define PINLED 9 //led 핀
#define PINRX 3 //Wifi RX 핀
#define PINTX 2 //Wifi TX 핀
#define PINFAN 8 //쿨링 팬 핀

//기타 define 선언부
#define PINLCDADDRESS 0x27 //LCD 주소값
#define WIFINAME dgsw_embd_1 //wifi이름
#define WIFIPASSWORD dgswdgsw //wifi 비번
#define SERVERHOST munyoung.kro.kr //BaseURL

//-----------------------------------------------------------------

//센서, 모듈 설정 선언 부
LiquidCrystal_I2C lcd(PINLCDADDRESS, 16, 2); //lcd 설정
DHT dht(DHTPIN, DHT22); //dht 센서 설정

//센서 값을 저장하거나 카운트를 할때 쓰는 전역 변수
unsigned long serverCount = 0; //서버 보낼 때 카운트

float temperature, humidity; //온도 습도 값을 저장 하는 변수
int soilHumi;
int illuminanceCount //조도 센서 값을 저장 하는 변수

int serverTimerCount; //서버 카운트
int fanTimerCount; //펜 카운트

int status = WL_IDLE_STATUS; //연결 확인 유무
WiFiEspClient client; //클라이언트 값 보낼 때 쓰는 변수

//-----------------------------------------------------------------

//setup
void void setup()
{
  //센서, 모듈 setup 부분
  Wire.begin(); //Wite 시작
  Serial.begin(9600); //Serial 시작
  dht.begin(); //dht 시작
  lcd.backlight(); //lcd 백라이트 켜기
  startDisplay(); //lcd 인트로 화면(함수)

  //서버 통신
  //WiFi.init(&Serial1) //wifi 설정 초기화
  //connectWifi(); //wifi 연결

  //pinMode 설정
  pinMode(PINFAN, OUTPUT); //쿨링 팬 핀 설정
  pinMode(PINPUMP, OUTPUT); //워터 펌프 핀 설정
  pinMode(PINLED, OUTPUT); //led 핀 설정

  //센서, 모듈 초기 설정
  digitalWrite(PINPUMP, LOW);

  //타이머
  TCCR1A = 0x00;
    TCCR1B = 0x0D;
    TCCR1C = 0x00;

    TCCR3A = 0x00;
    TCCR3B = 0x0C;
    TCCR3C = 0x00;

    TCCR2A = 0x02;
    TCCR2B = 0x07;

    OCR1A = 1500;
    OCR3A = 6250;
    OCR2A = 125;

    TIMSK1 = 0x02;
    TIMSK3 = 0x02;
    TIMSK2 = 0x02;
}

//-----------------------------------------------------------------

//loop
void loop()
{
  //토양 습도
  soilHumi = analogRead(SOILHUMIPIN); //토양 습도 값 받아오기
  soilHumiValue = map(soilHumi, 550, 0, 0, 100); //아날로그 데이터 변환

  //조도 센서
  illuminanceCount = getCds(); //조도 센서 값 받아오기

  //온도 습도
  temperature = dht.readTemperature(); //온도 받아오기
  humidity = dht.readHumidity(); //습도 받아오기

  //lcd 출력
  printHT(); //온도 습도 출력

}

//-----------------------------------------------------------------

//서버 통신 부분
void sendService(float humi, float temp, int soil_humi){

  if(client.connect(SERVERHOST, 3000)){
      Serial.println("Connected to Server");
  }

}


//-----------------------------------------------------------------

//와이파이 연결
void connectWifi(){
  if(WiFi.status() == WL_NO_SHIELD){
      Serial.println("WiFi shield not present");

      while(true);
  }

  while(status != WL_CONNECTED){

    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(WIFINAME);
    status = WiFi.begin(WIFINAME, WIFIPASSWORD);
      
      Serial.println("You're connected to the network");
      printWifiStatus();
      Serial.println();
      Serial.println("Starting connection to server...");
  }
}

//-----------------------------------------------------------------

//status 출력
void printWifiStatus(){
  Serial.print("WIFINAME: ");
  Serial.println(WiFi.SSID());
  
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}

//-----------------------------------------------------------------

//ISR(Wifi 통신)
ISR(TIMER1_COMPA_vect)
{
  timer1_count++;
  if (timer1_count == 3125) {
    timer1_count = 0;
    if (!client.connected())
    {
      connectWifi();
    }

    sendData(humi, temp, soil_humi_value);
  }
}

//-----------------------------------------------------------------

//ISR(쿨링 팬)
ISR(TIMER3_COMPA_vect)
{
  //Serial.println("timer3");
  timer3_count++;
  if (timer3_count > 13)
    timer3_count = 0;
  if (temp >= 25)
  {
    digitalWrite(8, LOW);
  }
  else
  {
    if (timer3_count > 3)
    {
      digitalWrite(8, HIGH);
    }
    if (timer3_count <= 3)
    {
      digitalWrite(8, LOW);
    }
  }
}

//-----------------------------------------------------------------

int timer2_count = 0;
int timer2_toggle = 0;

//ISR(모터 펌프)
ISR(TIMER2_COMPA_vect)
{

  if (soil_humi_value < 0) {

    timer2_count++;

    if (timer2_toggle == 0) {
      timer2_toggle = 1;
      
      if (timer2_count == 125 * 3)
      {
        timer2_count = 0;
        digitalWrite(PINPUMP, HIGH);
      }
      
    } else {
        timer2_toggle = 0;
        Serial.println("toggle : 2");

        if (timer2_count == 125 * 5)
        {
          timer2_count = 0;
          digitalWrite(PINPUMP, LOW);
        }

      }
    
  }else {
    timer2_count = 0;
  }

}

//-----------------------------------------------------------------

//lcd 함수
//인트로 text 출력
void startDisplay() {
  lcd.init();

  lcd.begin(16, 2);
  lcd.setCursor(1, 0);
  lcd.print("SmartFarm");
  lcd.setCursor(9, 1);
  lcd.print("ver 1.0");
  delay(1000);
  lcd.clear();
}

//-----------------------------------------------------------------

//lcd 함수
//현재 온도 현재 습도 LCD출력
void printHT() {
  lcd.setCursor(0, 0);
  lcd.print("Tem: ");
  lcd.print(temp);
  lcd.setCursor(0, 1);
  lcd.print("Humi: ");
  lcd.print(humi);
  delay(1000);
  lcd.clear();
}

//-----------------------------------------------------------------