#include <Arduino.h>
#include "SHT31.h"
#include "Smoke_sensor.h"
#include "EspNow.h"
#include "data.h"

u8_t masterMAC[] = {0xE0, 0x5A, 0x1B, 0x6C, 0x58, 0xD0};  // E0:5A:1B:6C:58:D0
u8_t slaveID = 1;  // Slave 1, Slave 2, ...

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Gui data den Master: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAILED");
}

void setup() 
{ 
  Serial.begin(115200);
  Init_Sht31();
  initSmokeSensor();

  initEspNow();
  addPeer(masterMAC);
  onDataSentCallback = OnDataSent; 
}

void loop()
{
  struct_message myData;
  myData.slaveID = slaveID;
  printDataSht31(myData);
  printDataSmokeSensor(myData);
  sendData(&myData, masterMAC);

  Serial.println("Dang gui data den Master...");
  delay(5000); 
}

