#include <SPI.h>
#include<ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100_PulseOximeter.h"

// blynk setup
#include <BlynkSimpleEsp8266.h>



char authToken[] = "j8jxdpEsUmFlYaHLWwhe02Dbxe7Ama-K";
char* ssid = "ebot";
char* pass = "13091993";

// mpu6050 setup
#include <MPU6050_tockn.h>

MPU6050 mpu6050(Wire);


// sim900a


#include "SIM900.h"
#include <SoftwareSerial.h>

SoftwareSerial SIM900(0, 2);


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
#define REPORTING_PERIOD_MS 1000
PulseOximeter pox;



const int Led_canh_bao = 13;

int x_1, y_1, z_1;
int x_2, y_2, z_2;

int check_time = 0;

const unsigned char bitmap [] PROGMEM =
{
  0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x18, 0x00, 0x0f, 0xe0, 0x7f, 0x00, 0x3f, 0xf9, 0xff, 0xc0,
  0x7f, 0xf9, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xe0, 0x7f, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf0,
  0xff, 0xf7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0x7f, 0xdb, 0xff, 0xe0,
  0x7f, 0x9b, 0xff, 0xe0, 0x00, 0x3b, 0xc0, 0x00, 0x3f, 0xf9, 0x9f, 0xc0, 0x3f, 0xfd, 0xbf, 0xc0,

  0x1f, 0xfd, 0xbf, 0x80, 0x0f, 0xfd, 0x7f, 0x00, 0x07, 0xfe, 0x7e, 0x00, 0x03, 0xfe, 0xfc, 0x00,
  0x01, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x3f, 0xc0, 0x00,
  0x00, 0x0f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


void onBeatDetected()
{
  Serial.println("Beat!");
  display.drawBitmap( 80, 20, bitmap, 28, 28, 1);
  display.display();
}



// thiet lap so dien thoai canh bao
String sdt = "+84345898643";
// goi canh bao
void SIM900Call(String sdt) {
  SIM900.print(F("ATD"));
  SIM900.print(sdt);
  SIM900.println(F(";\r\n"));
}

// gui tin nhan

void SIM900SMS(String sdt, String content) {
  SIM900.println("AT+CMGS=\"" + sdt + "\"");     // Lenh gui tin nhan
  delay(3000);                                     // Cho ky tu '>' phan hoi ve
  SIM900.print(content);                           // Gui noi dung
  SIM900.print((char)26);                          // Gui Ctrl+Z hay 26 de ket thuc noi dung tin nhan va gui tin di
  delay(5000);                                     // delay 5s
  Serial.print("Đã gủi tin nhắn đến số: ");
  Serial.println(sdt);
  Serial.print("Nội dung tin nhắn: ");
  Serial.println(content);
}

void setup() {
  //SIM900.begin(9600);
  Serial.begin(9600);
  SIM900.begin(9600);
  // sim900 connect
  SIM900.println("AT");
  delay(200);

  pinMode(Led_canh_bao, OUTPUT);
  digitalWrite(Led_canh_bao, LOW);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000); // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 0);

  display.println("Initializing pulse oximeter..");
  display.display();
  Serial.print("Initializing pulse oximeter..");
  Blynk.begin(authToken, ssid, pass);
  if (!pox.begin()) {
    Serial.println("FAILED");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(0, 0);
    display.println("FAILED");
    display.display();
    for (;;);
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(0, 0);
    display.println("SUCCESS");
    display.display();
    Serial.println("SUCCESS");
  }

  mpu6050.begin();
  x_1 = 0;
  y_1 = 0;
  z_1 = 0;
   pox.setOnBeatDetectedCallback(onBeatDetected);

}

void loop() {
  pox.update();
  Serial.print("Heart BPM:");
  Serial.print(pox.getHeartRate());
  Serial.print("-----");
  Serial.print("Oxygen Percent:");
  Serial.print(pox.getSpO2());
  Serial.println("\n");

  Blynk.virtualWrite(V5, pox.getHeartRate());
  Blynk.virtualWrite(V6, pox.getSpO2());


  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(1);
  display.setCursor(32, 0);
  display.println("VXMHUST");

  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 16);
  display.println("Heart Bpm");
  display.setCursor(0, 26);
  display.println(pox.getHeartRate());
  display.setCursor(0, 40);
  display.println("Spo2");
  display.setCursor(0, 50);
  display.println(pox.getSpO2());
  display.display();
  x_2 = x_1;
  y_2 = y_1;
  z_2 = z_1;
  mpu6050.update();
  int x = mpu6050.getAngleX();
  int y = mpu6050.getAngleY();
  int z = mpu6050.getAngleZ();
  Serial.print("angleX : ");
  Serial.print(x);
  Serial.print("\tangleY : ");
  Serial.print(y);
  Serial.print("\tangleZ : ");
  Serial.println(z);

  x_1 = x;
  y_1 = y;
  z_1 = z;
  int dentaX = x_2 - x_1;
  int dentaY = y_2 - y_1;
  int dentaZ = z_2 - z_1;
  if ((abs(dentaX) <= 2) && (abs(dentaY) <= 2) && (abs(dentaZ) <= 2) && (pox.getHeartRate() > 120)) {
    check_time++;
  }
  if (check_time == 15) {
    check_time = 0;
    digitalWrite(Led_canh_bao, HIGH);
    SIM900SMS(sdt, "Canh bao ");
    delay(1000);
    SIM900Call(sdt);
    delay(5000);
    digitalWrite(Led_canh_bao, LOW);
    delay(1000);
    pox.shutdown();
    pox.begin();
    if (SIM900.available() > 0) {
      Serial.write(SIM900.read());
    }
  }
}
