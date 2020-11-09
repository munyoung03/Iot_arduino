/*
 
 - project: 스마트팜
 - 담당자: 양준혁, 이문영
 - 최종 작성일: 2020.08.26
 - 오류 발생 시 보드 확인 요망

*/

//라이브러리 선언부
#include "WiFiEsp.h" //WiFi 관련 라이브러리
#include "DHT.h" //온습도 센서 관련 라이브러리

#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
#endif

#include <Wire.h>
#include <LiquidCrystal_I2C.h> //LCD 관련 라이브러리
#include <ArduinoJson.h> //Json파싱 라이브러리

//-----------------------------------------------------------------

//define 선언부
//핀 설정
#define DHTPIN 4 //DHT 핀
#define SOILHUMIPIN A0 //토양 습도 핀
#define PINPUMP 13 //워터 펌프 핀
#define PINJODO A15 //조도 센서 핀
#define PINLED 9 //led 핀
#define PINFAN 8 //쿨링 팬 핀

//기타 define 선언부
#define PINLCDADDRESS 0x27 //LCD 주소값

//-----------------------------------------------------------------

//센서, 모듈 설정 선언 부
LiquidCrystal_I2C lcd(PINLCDADDRESS, 16, 2); //lcd 설정
DHT dht(DHTPIN, DHT22); //dht 센서 설정

//센서 값을 저장하거나 카운트를 할때 쓰는 전역 변수

float temperature, humidity; //온도 습도 값을 저장 하는 변수
int soilHumi;
int soilHumiValue;
int illuminanceCount; //조도 센서 값을 저장 하는 변수

int fanTimerCount; //펜 카운트

//-----------------------------------------------------------------

//setup
void setup()
{
  //센서, 모듈 setup 부분
  Wire.begin(); //Wite 시작
  Serial.begin(9600); //Serial 시작
  dht.begin(); //dht 시작
  lcd.backlight(); //lcd 백라이트 켜기
  startDisplay(); //lcd 인트로 화면(함수)

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

//ISR(쿨링 팬)
ISR(TIMER3_COMPA_vect)
{
  //Serial.println("timer3");
  fanTimerCount++;
  if (fanTimerCount > 13)
    fanTimerCount = 0;
  if (temperature >= 25)
  {
    digitalWrite(8, LOW);
  }
  else
  {
    if (fanTimerCount > 3)
    {
      digitalWrite(8, HIGH);
    }
    if (fanTimerCount <= 3)
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

  if (soilHumiValue < 0) {

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
//조도 센서 계산 값 반환 함수
int getCds() {
  return analogRead(PINJODO);
}

//-----------------------------------------------------------------

//lcd 함수
//인트로 text 출력
void startDisplay() {
  lcd.init();

  lcd.begin(16, 2);
  lcd.setCursor(1, 0); //커서 setting
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
  lcd.setCursor(0, 0); //커서 setting
  lcd.print("Tem: "); 
  lcd.print(temperature);
  lcd.setCursor(0, 1);
  lcd.print("Humi: ");
  lcd.print(humidity);
  delay(1000); 
  lcd.clear();
}

//-----------------------------------------------------------------
