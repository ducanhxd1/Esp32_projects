// Espnow.cpp
#include "Espnow.h"

void (*onDataSentCallback)(const uint8_t *mac_addr, esp_now_send_status_t status) = nullptr;
void (*onDataRecvCallback)(const uint8_t *mac_addr, const uint8_t *incomingData, int len) = nullptr;


void masterOnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (onDataSentCallback) onDataSentCallback(mac_addr, status);
}

void masterOnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingDataRaw, int len) {
    if (onDataRecvCallback) onDataRecvCallback(mac_addr, incomingDataRaw, len);
}

void initEspNow() {
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Loi khoi tao ESP-NOW");
        return;
    }
    esp_now_register_send_cb(masterOnDataSent);   
    esp_now_register_recv_cb(masterOnDataRecv);   
    Serial.println("ESP-NOW khoi tao thanh cong");
}

void addPeer(uint8_t *peerMac) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, peerMac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Khong add duoc peer");
    }
}

void sendData(struct_message *data, uint8_t *peerMac) {
    esp_now_send(peerMac, (uint8_t *)data, sizeof(struct_message));
}
