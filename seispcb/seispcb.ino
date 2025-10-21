#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

bool calibrated = false;
float calX = 0, calY = 0, calZ = 0;

void selfCalibrate() {
  if (calibrated) return;
  calibrated = true;
  Serial.println("calibrating...");
  const int numSamples = 20;
  float xSum = 0, ySum = 0, zSum = 0;
  
  for (int i = 0; i < numSamples; i++) {
    sensors_event_t event;
    accel.getEvent(&event);
    xSum += event.acceleration.x;
    ySum += event.acceleration.y;
    zSum += event.acceleration.z;
    delay(50);
  }
  
  calX = xSum / numSamples;
  calY = ySum / numSamples;
  calZ = zSum / numSamples;
  
  Serial.println("calibration done!!");
  Serial.print("offsets: x="); Serial.print(calX, 4);
  Serial.print(" y="); Serial.print(calY, 4);
  Serial.print(" z="); Serial.println(calZ, 4);
}

void setup() {
  Serial.begin(115200);
  Serial.println("setup...");
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("ssd1306 fail :("));
    while (1);
  }
  
  display.clearDisplay();
  display.display();
  
  if (!accel.begin()) {
    Serial.println("ADXL345 not found, how do you even forget that?");
    while (1);
  }
  
  accel.setRange(ADXL345_RANGE_16_G);
  delay(500);
  selfCalibrate();
  Serial.println("Finished setup");
}

void loop() {
  sensors_event_t event;
  accel.getEvent(&event);
  
  // apply calibration
  float X = event.acceleration.x - calX;
  float Y = event.acceleration.y - calY;
  float Z = event.acceleration.z - calZ;
  
  Serial.print(X); Serial.print(" ");
  Serial.print(Y); Serial.print(" ");
  Serial.println(Z);
  
  // update accel
  display.fillRect(1, 8, 125, 10, BLACK);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1, 8); display.println(X, 3);
  display.setCursor(45, 8); display.println(Y, 3);
  display.setCursor(90, 8); display.println(Z, 3);
  display.setCursor(10, 20); display.println("MaxG");
  display.drawLine(40, 20, 40, 55, SSD1306_WHITE);
  display.display();
  
  // update intensity every 3s
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 3000) {
    lastUpdate = millis();
    display.fillRect(4, 30, 110, 40, BLACK);
    display.setTextColor(WHITE);
    
    float maxG = max(fabs(X), max(fabs(Y), fabs(Z)));
    display.setCursor(10, 37);
    display.setTextSize(1);
    
    if (maxG < 0.05) display.println("MNML");
    else if (maxG < 0.2) display.println("WEAK");
    else if (maxG < 0.4) display.println("MDRT");
    else if (maxG < 0.7) display.println("STRG");
    else display.println("SEVR");
    
    display.display();
  }
  
  delay(20);
}
