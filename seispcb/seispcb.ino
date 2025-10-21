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

String getAccelIntensity(float val) {

      if (val < 0.05) return "MNML";
      else if (val < 0.2) return "WEAK";
      else if (val < 0.4) return "MDRT";
      else if (val < 0.7) return "STRG";
      else return "SEVR";
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
  display.setCursor(0, 20); display.println("MaxG");
  display.setCursor(40, 20); display.println("X");
  display.setCursor(70, 20); display.println("Y");
  display.setCursor(100, 20); display.println("Z");
  display.drawLine(30, 20, 30, 55, SSD1306_WHITE);
  display.display();

  // update intensity every 3s
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 3000) {
    lastUpdate = millis();
    display.fillRect(0, 30, 130, 40, BLACK);
    display.setTextColor(WHITE);
    
    float maxG = max(fabs(X), max(fabs(Y), fabs(Z)));
    float abX = fabs(X);
    float abY = fabs(Y);
    float abZ = fabs(Z);

    display.setTextSize(1);
    display.setCursor(0, 37);
    display.println(getAccelIntensity(maxG));
    display.setCursor(35, 37);
    display.println(getAccelIntensity(abX));
    display.setCursor(65, 37);
    display.println(getAccelIntensity(abY));
    display.setCursor(95, 37);
    display.println(getAccelIntensity(abZ));

    
    display.display();
  }
  delay(20);
}
