#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

typedef struct sensor_packet {
  int value;
} sensor_packet;

void onDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  sensor_packet pkt;
  memcpy(&pkt, incomingData, sizeof(pkt));

  Serial.print("Packet from: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", info->src_addr[i]);
    if (i < 5) Serial.print(":");
  }

  Serial.print(" | Value = ");
  Serial.println(pkt.value);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  WiFi.mode(WIFI_STA);

  uint8_t mac[6];
  esp_wifi_get_mac(WIFI_IF_STA, mac);

  Serial.printf("Receiver MAC (STA): %02X:%02X:%02X:%02X:%02X:%02X\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
}

void loop() {}
