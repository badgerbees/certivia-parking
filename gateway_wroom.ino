#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// =============================
// PACKET STRUCT
// =============================
typedef struct {
  int distance;
  bool occupied;
} ParkingPacket;

ParkingPacket incomingPacket;

// =============================
// NEW CALLBACK FORMAT (required)
// =============================
void onReceive(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  memcpy(&incomingPacket, incomingData, sizeof(incomingPacket));

  Serial.printf("ESP-NOW Received → dist=%d occupied=%d\n",
                incomingPacket.distance,
                incomingPacket.occupied);

  // Forward to Raspberry Pi
  Serial1.printf("%d,%d\n",
                 incomingPacket.distance,
                 incomingPacket.occupied ? 1 : 0);
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 16, 17); // set UART pins if needed

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  esp_now_register_recv_cb(onReceive);

  Serial.println("ESP-NOW Gateway Ready. Forwarding to Raspberry Pi...");
}

void loop() {}
