#include <Arduino.h>

// =============================
// PIN DEFINITIONS
// =============================
#define TRIG 3
#define ECHO 2
#define LED_RED 5
#define LED_GREEN 8

// =============================
// ULTRASONIC FUNCTION
// =============================
long getDistanceCM() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000); // timeout 30ms
  long distance = duration * 0.034 / 2;       // convert to cm

  return distance;
}

void setup() {
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  Serial.begin(115200);
}

void loop() {
  long distance = getDistanceCM();
  Serial.print("Distance: ");
  Serial.println(distance);

  // =============================
  // NORMAL: GREEN ON
  // OBJECT DETECTED: RED ON
  // =============================
  if (distance > 0 && distance < 100) {  
    // Detected < 100 cm -> turn RED
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
  } 
  else {
    // No object -> remain GREEN
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
  }

  delay(100);
}
