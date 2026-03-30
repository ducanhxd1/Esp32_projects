#include <Arduino.h>
#include "data.h"
#include "Espnow.h"

struct_message incomingData;
 uint8_t senderMAC[6] = {0xF0, 0x24, 0xF9, 0xEB, 0x71, 0x34}; // F0:24:F9:EB:71:34 (slave 1) 
// uint8_t senderMAC[6]; 

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingDataRaw, int len) {
    memcpy(&incomingData, incomingDataRaw, sizeof(incomingData));
    memcpy(senderMAC, mac_addr, 6);

    Serial.printf("\n=== SLAVE %d ===\n", incomingData.slaveID);
    Serial.printf("Temp: %.2f °C\n", incomingData.temp);
    Serial.printf("Humi  : %.2f %%\n", incomingData.humi);
    Serial.printf("Smoke A: %d\n", incomingData.smokeValueA);
    Serial.printf("Smoke B: %d\n", incomingData.smokeValueB);
    Serial.printf("MAC  : %02X:%02X:%02X:%02X:%02X:%02X\n", 
                  mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.println("========================\n");


}

void setup() {
    Serial.begin(115200);
    initEspNow();
 
    onDataRecvCallback = OnDataRecv;   //  callback 
    Serial.println("Master nhan data tu nhieu Slave, OK");
}

void loop() {
    delay(100);
}