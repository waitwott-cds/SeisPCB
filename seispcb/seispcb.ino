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

// bytes used to save space
byte accelLvl(float val) {
  if (val < 0.05) return 0;
  else if (val < 0.2) return 1;
  else if (val < 0.4) return 2;
  else if (val < 0.7) return 3;
  else return 4;
}

// now this returns label from lvl
const char* accelLabel(byte lvl) {
  static const char* labels[] = {"MNML", "WEAK", "MDRT", "STRG", "SEVR"};
  return labels[lvl];
}

bool blinkState = false;
unsigned long lastBlinkTime = 0;

void alertBlink(float maxG, float abX, float abY, float abZ) {
  static unsigned long lastToggle = 0;
  const unsigned long interval = 200;

  byte lvlMax = accelLvl(maxG);
  byte lvlX = accelLvl(abX);
  byte lvlY = accelLvl(abY);
  byte lvlZ = accelLvl(abZ);

  if (millis() - lastToggle >= interval) {
    lastToggle = millis();
    blinkState = !blinkState;

    display.fillRect(0, 50, 130, 12, BLACK);

    if (lvlMax >= 2 && blinkState) display.fillRect(0, 50, 25, 12, WHITE);
    if (lvlX   >= 2 && blinkState) display.fillRect(35, 50, 25, 12, WHITE);
    if (lvlY   >= 2 && blinkState) display.fillRect(65, 50, 25, 12, WHITE);
    if (lvlZ   >= 2 && blinkState) display.fillRect(95, 50, 25, 12, WHITE);

    display.display();
  }
}

void loop() {
  sensors_event_t event;
  accel.getEvent(&event);

  float X = event.acceleration.x - calX;
  float Y = event.acceleration.y - calY;
  float Z = event.acceleration.z - calZ;

  float abX = fabs(X);
  float abY = fabs(Y);
  float abZ = fabs(Z);
  float maxG = max(abX, max(abY, abZ));

  byte lvlMax = accelLvl(maxG);
  byte lvlX   = accelLvl(abX);
  byte lvlY   = accelLvl(abY);
  byte lvlZ   = accelLvl(abZ);

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
  display.display();

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 500) {
    lastUpdate = millis();
    display.fillRect(0, 30, 130, 40, BLACK);
    display.setTextColor(WHITE);
    display.setTextSize(1);

    display.setCursor(0, 37);  display.println(accelLabel(lvlMax));
    display.setCursor(35, 37); display.println(accelLabel(lvlX));
    display.setCursor(65, 37); display.println(accelLabel(lvlY));
    display.setCursor(95, 37); display.println(accelLabel(lvlZ));

    display.display();
  }

  alertBlink(maxG, abX, abY, abZ);
}



