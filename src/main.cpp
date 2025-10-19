#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  
  // Wait for serial connection
  while (!Serial) {
    delay(10);
  }
  
  Serial.println("\n=== ESP32-S3 Basic Info ===");
  
  // Chip info
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  Serial.printf("Chip: ESP32-S3 Rev %d\n", chip_info.revision);
  Serial.printf("Cores: %d\n", chip_info.cores);
  
  // Memory
  Serial.printf("Free Heap: %d bytes\n", esp_get_free_heap_size());
  Serial.printf("PSRAM Size: %d bytes\n", ESP.getPsramSize());
  
  // Flash
  Serial.printf("Flash Size: %d MB\n", spi_flash_get_chip_size() / (1024 * 1024));
  
  // MAC address
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  Serial.printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", 
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  
  Serial.println("=== COMMS OK ===");
}

void loop() {
  // Blink LED and print heartbeat
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 2000) {
    lastPrint = millis();
    Serial.printf("Heartbeat - Free Heap: %d bytes\n", esp_get_free_heap_size());
  }
  delay(100);
}