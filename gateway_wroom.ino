#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// =============================
// PACKET STRUCT (from nodes)
// =============================
typedef struct {
  int distance;
  bool occupied;
} ParkingPacket;

ParkingPacket incomingPacket;

// =============================
// ON RECEIVE CALLBACK
// =============================
void onReceive(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingPacket, incomingData, sizeof(incomingPacket));

  Serial.printf("ESP-NOW Received → dist=%d occupied=%d\n",
                incomingPacket.distance,
                incomingPacket.occupied);

  // ====================================
  // FORWARD TO Raspberry Pi Zero 2W
  // Format: distance,occupied\n
  // ====================================
  Serial1.printf("%d,%d\n",
                 incomingPacket.distance,
                 incomingPacket.occupied ? 1 : 0);
}

void setup() {
  Serial.begin(115200);   // Debug (USB)
  Serial1.begin(115200);  // UART to Raspberry Pi (TX/RX)

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  esp_now_register_recv_cb(onReceive);

  Serial.println("ESP-NOW Gateway Ready. Forwarding to Raspberry Pi...");
}

void loop() {
  // Nothing needed — everything is event-driven
}
