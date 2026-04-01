#include <Arduino.h>
#include "data.h"
#include "Espnow.h"
#include <Arduino_JSON.h>
#include <AsyncTCP.h>  
#include <ESPAsyncWebServer.h>


struct_message incomingData;
uint8_t senderMAC[6] = {0xF0, 0x24, 0xF9, 0xEB, 0x71, 0x34}; // F0:24:F9:EB:71:34 (slave 1)

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingDataRaw, int len) {
    memcpy(&incomingData, incomingDataRaw, sizeof(incomingData));
    memcpy(senderMAC, mac_addr, 6);

    incomingData.timestamp = (unsigned long)time(nullptr);  // Unix timestamp (accurate after NTP sync)

    
    Serial.printf("Nhận Slave %d: %.1f°C | %.1f%% | Smoke A:%d | Smoke B:%d\n", 
                  incomingData.slaveID, incomingData.temp, incomingData.humi,
                  incomingData.smokeValueA, incomingData.smokeValueB);
}

// ==================== SETUP ====================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Initialize ESP-NOW
    initEspNow();
    addPeer(senderMAC);
    onDataRecvCallback = OnDataRecv;
    Serial.println("[OK] ESP-NOW Initialized!");
    
    Serial.println("[OK] Web Server Started!");
}

// ==================== LOOP ====================
void loop() {
    delay(100);
}