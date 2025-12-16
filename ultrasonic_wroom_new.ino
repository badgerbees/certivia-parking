#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// =============================
// CONFIGURATION
// =============================
#define SENSOR_ID "A1"  // Change this for each sensor: A1, A2, B1, etc

// =============================
// PIN DEFINITIONS
// =============================
#define TRIG 3
#define ECHO 2
#define LED_RED 5
#define LED_GREEN 8

// =============================
// PACKET STRUCT
// =============================
typedef struct {
  char sensor_id[4];  // "A1", "B3", etc
  int distance;
  bool occupied;
} ParkingPacket;

ParkingPacket packet;

// =============================
// GATEWAY MAC ADDRESS (FIXED!)
// =============================
uint8_t gatewayAddress[] = {0xEC, 0x64, 0xC9, 0x85, 0x15, 0xD0};

// =============================
// NEW CALLBACK FORMAT (IDF v5.5)
// =============================
void onSent(const esp_now_send_info_t *info, esp_now_send_status_t status) {
  Serial.printf("Packet → %s\n", status == ESP_NOW_SEND_SUCCESS ? "Delivered" : "FAILED");
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ ESP-NOW Init Failed");
    return;
  }

  esp_now_register_send_cb(onSent);

  // Register Peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("❌ Failed to add peer");
    return;
  }

  Serial.println("✔ ESP-NOW Sensor Ready");
}

void loop() {

  // Trigger ultrasonic
  digitalWrite(TRIG, LOW); delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);
  long distance = duration * 0.034 / 2;

  Serial.printf("Distance: %ld cm\n", distance);

  bool occupied = (distance > 0 && distance < 150);

  digitalWrite(LED_RED,   occupied);
  digitalWrite(LED_GREEN, !occupied);

  // Pack data with sensor_id
  strcpy(packet.sensor_id, SENSOR_ID);
  packet.distance = distance;
  packet.occupied = occupied;

  esp_now_send(gatewayAddress, (uint8_t*)&packet, sizeof(packet));

  delay(300);
}
