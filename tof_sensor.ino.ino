#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

// Safe LED pins you confirmed working
#define LED_RED     2
#define LED_GREEN   3

// Safe I2C pins for ESP32-C3 SuperMini
#define SDA_PIN     8
#define SCL_PIN     9

// Distance threshold
#define THRESHOLD_CM 10

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== ESP32-C3 STARTED ===");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  // Start with GREEN ON
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, HIGH);

  Serial.println("Starting I2C...");
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(200);

  Serial.println("Initializing VL53L0X...");
  if (!lox.begin()) {
    Serial.println("❌ ERROR: Sensor not detected!");
    Serial.println("Check wiring:");
    Serial.println("  VIN → 5V");
    Serial.println("  GND → GND");
    Serial.println("  SDA → GPIO 8");
    Serial.println("  SCL → GPIO 9");
    Serial.println("  Library → Adafruit_VL53L0X");

    // Flash RED forever → error state
    while (true) {
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, LOW);
      delay(300);
      digitalWrite(LED_RED, LOW);
      delay(200);
    }
  }

  Serial.println("✔️ VL53L0X READY");
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {
    int dist = measure.RangeMilliMeter / 10;
    Serial.print("Distance: ");
    Serial.print(dist);
    Serial.println(" cm");

    if (dist < THRESHOLD_CM) {
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, LOW);
    } else {
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, HIGH);
    }

  } else {
    Serial.println("Out of range");
  }

  delay(70);
}
