#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// =============================
// PACKET STRUCT (must match sender)
// =============================
typedef struct sensor_packet {
  int state;        // 1 = triggered, 0 = normal
  long distance;    // distance in cm
} sensor_packet;

sensor_packet pkt;

// UART to Raspberry Pi uses Serial1
// TX = GPIO1 (default), RX = GPIO3 (if needed)
HardwareSerial PiSerial(1);

void onDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  memcpy(&pkt, incomingData, sizeof(pkt));

  // =============================
  // PRINT TO SERIAL MONITOR (USB)
  // =============================
  Serial.print("Packet from: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", info->src_addr[i]);
    if (i < 5) Serial.print(":");
  }

  Serial.printf(" | State: %d | Distance: %ld cm\n", pkt.state, pkt.distance);

  // =============================
  // FORWARD TO RASPBERRY PI
  // =============================
  // Format as JSON for easy parsing on Pi
  PiSerial.printf("{\"state\":%d,\"distance\":%ld}\n", pkt.state, pkt.distance);
}

void setup() {
  Serial.begin(115200);   // USB Serial (Monitor)
  delay(300);

  // =============================
  // UART for Raspberry Pi
  // TX = GPIO1, RX = GPIO3
  // =============================
  PiSerial.begin(115200, SERIAL_8N1, 3, 1);  
  // RX=3 (unused), TX=1 → Pi GPIO15 (Rx)

  Serial.println("UART to Raspberry Pi started...");

  // =============================
  // ESP-NOW INIT
  // =============================
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

  Serial.println("ESP32 WROOM Receiver Ready!");
}

void loop() {
  // Nothing needed here
}
