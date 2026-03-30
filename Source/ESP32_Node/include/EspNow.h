#ifndef _ESP_NOW_H_
#define _ESP_NOW_H_

#include <WiFi.h>
#include <esp_now.h>
#include "data.h"

void getMACaddr();

void initEspNow();
void addPeer(uint8_t *peerMac);
void sendData(struct_message *data, uint8_t *peerMac);
void sendReply(const char* message, uint8_t *peerMac);


extern void (*onDataSentCallback)(const uint8_t *mac_addr, esp_now_send_status_t status);
extern void (*onDataRecvCallback)(const uint8_t *mac_addr, const uint8_t *incomingData, int len);

#endif

