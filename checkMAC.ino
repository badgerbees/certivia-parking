#include <WiFi.h>
#include <esp_wifi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  uint8_t baseMac[6];
  esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  Serial.printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
}

void loop() {
}
