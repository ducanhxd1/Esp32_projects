#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_sleep.h>
#include "BM22S2021-1.h"
#include "Adafruit_SHT31.h"

// ============ SENSOR CONFIGURATION ============
// Smoke Sensor (BM22S2021-1) - UART pins
const uint8_t SMOKE_RX_PIN = 16;
const uint8_t SMOKE_TX_PIN = 17;
const uint8_t SMOKE_STATUS_PIN = 4;  // Status pin for smoke detection interrupt

// I2C Pins for SHT31 (default: GPIO 21 = SDA, GPIO 22 = SCL)
const uint8_t I2C_SDA = 21;
const uint8_t I2C_SCL = 22;

// Deep Sleep Configuration
const uint64_t SLEEP_DURATION_US = 2 * 60 * 1000000;  // 2 hours in microseconds
#define WAKEUP_PIN  SMOKE_STATUS_PIN

// ============ STRUCTURE FOR DATA TRANSMISSION ============
typedef struct SensorData {
  uint32_t nodeID;
  uint16_t smokeValue;
  uint8_t smokeStatus;  // 0 = no smoke, 1 = smoke detected
  float temperature;
  float humidity;
  uint8_t sht31Status;  // 0 = OK, 1 = Error
  uint32_t timestamp;
} SensorData;

// ============ GLOBAL VARIABLES ============
BM22S2021_1 smokeSensor(SMOKE_STATUS_PIN, SMOKE_RX_PIN, SMOKE_TX_PIN);
Adafruit_SHT31 sht31 = Adafruit_SHT31();

uint8_t masterMAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // Update with master MAC
uint8_t nodeID = 1;  // Node identifier, change for each node

volatile bool smokeAlarmTriggered = false;

// ============ ESP-NOW CALLBACKS ============
void onSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("[ESP-NOW] Data sent successfully");
  } else {
    Serial.println("[ESP-NOW] Send failed");
  }
}

void onReceive(const uint8_t * mac, const uint8_t *incomingData, int len) {
  Serial.print("[ESP-NOW] Received ");
  Serial.print(len);
  Serial.println(" bytes");
}

// ============ INTERRUPT HANDLER FOR SMOKE ============
void IRAM_ATTR smokeInterrupt() {
  smokeAlarmTriggered = true;
}

void readAndSendData();
void sendData(SensorData &data);
// ============ SETUP FUNCTION ============
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== ESP32 Sensor Node Starting ===");
  
  // ---- Initialize GPIO ----
  pinMode(SMOKE_STATUS_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SMOKE_STATUS_PIN), smokeInterrupt, RISING);
  
  // ---- Initialize I2C for SHT31 ----
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Initialize SHT31 Temperature/Humidity Sensor
  if (!sht31.begin(0x44)) {
    Serial.println("[ERROR] SHT31 sensor not found!");
  } else {
    Serial.println("[OK] SHT31 initialized");
  }
  
  // ---- Initialize Smoke Sensor (UART) ----
  smokeSensor.begin();
  delay(500);
  
  uint8_t smokeStatus = smokeSensor.getSTATUS();
  Serial.print("[INFO] Smoke sensor status: 0x");
  Serial.println(smokeStatus, HEX);
  
  // ---- Initialize WiFi & ESP-NOW ----
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] ESP-NOW init failed!");
    return;
  }
  Serial.println("[OK] ESP-NOW initialized");
  
  // Register callbacks
  esp_now_register_send_cb(onSend);
  esp_now_register_recv_cb(onReceive);
  
  // Add master peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, masterMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[WARNING] Failed to add master peer");
  } else {
    Serial.println("[OK] Master peer added");
  }
  
  Serial.println("=== Setup Complete ===\n");
  
  // Perform first sensor reading immediately
  readAndSendData();
}

// ============ READ ALL SENSORS ============
void readAndSendData() {
  Serial.println("[SENSOR] Reading sensors...");
  
  SensorData data = {};
  data.nodeID = nodeID;
  data.timestamp = millis();
  
  // ---- Read Smoke Sensor (Priority: HIGH) ----
  // Request smoke sensor data package
  uint8_t smokeBuffer[41] = {0};
  if (smokeSensor.requestInfoPackage(smokeBuffer) == 0) {
    if (smokeSensor.isInfoAvailable()) {
      smokeSensor.readInfoPackage(smokeBuffer);
      
      // Extract smoke value (raw ADC)
      data.smokeValue = (smokeBuffer[1] << 8) | smokeBuffer[2];
      
      // Check status pin for immediate smoke detection
      data.smokeStatus = digitalRead(SMOKE_STATUS_PIN);
      
      Serial.print("[SMOKE] Value: ");
      Serial.print(data.smokeValue);
      Serial.print(" | Status Pin: ");
      Serial.println(data.smokeStatus);
    } else {
      Serial.println("[ERROR] Smoke sensor data not available");
      data.smokeValue = 0;
      data.smokeStatus = 0;
    }
  } else {
    Serial.println("[ERROR] Failed to read smoke sensor");
    data.smokeValue = 0;
    data.smokeStatus = 0;
  }
  
  // ---- Read SHT31 Temperature & Humidity ----
  float temp = sht31.readTemperature();
  float humidity = sht31.readHumidity();
  
  if (!isnan(temp) && !isnan(humidity)) {
    data.temperature = temp;
    data.humidity = humidity;
    data.sht31Status = 0;
    
    Serial.print("[SHT31] Temp: ");
    Serial.print(temp, 1);
    Serial.print("°C | Humidity: ");
    Serial.print(humidity, 1);
    Serial.println("%");
  } else {
    Serial.println("[ERROR] Failed to read SHT31");
    data.temperature = -99.0;
    data.humidity = -99.0;
    data.sht31Status = 1;
  }
  
  // ---- Send data via ESP-NOW ----
  sendData(data);
}

// ============ SEND DATA VIA ESP-NOW ============
void sendData(SensorData &data) {
  Serial.println("[TRANSMIT] Sending data via ESP-NOW...");
  
  esp_err_t result = esp_now_send(masterMAC, (uint8_t *)&data, sizeof(data));
  
  if (result == ESP_OK) {
    Serial.println("[OK] Data sent");
  } else {
    Serial.print("[ERROR] Send failed: ");
    Serial.println(result);
  }
  
  delay(100);  // Allow time for ESP-NOW to complete
}

// ============ ENTER DEEP SLEEP ============
void enterDeepSleep() {
  Serial.println("[SLEEP] Entering deep sleep...");
  Serial.println("Wake-up triggers:");
  Serial.println("  - Timer: 2 hours");
  Serial.println("  - Smoke status pin HIGH");
  
  delay(100);  // Flush serial
  
  // Configure wake-ups
  // 1. Timer wake-up (2 hours)
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION_US);
  
  // 2. External wake-up (smoke status pin HIGH)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, HIGH);
  
  // Enter deep sleep
  esp_deep_sleep_start();
}

// ============ MAIN LOOP ============
void loop() {
  // Check if smoke alarm was triggered
  if (smokeAlarmTriggered) {
    Serial.println("\n!!! SMOKE DETECTED !!!\n");
    readAndSendData();
    smokeAlarmTriggered = false;
    
    // Optional: Don't sleep immediately if smoke is detected
    // Let the master handle the alarm
    delay(5000);  // Wait 5 seconds before trying to sleep
  }
  
  // Enter deep sleep after normal reading
  // (Adjust this logic based on your requirements)
  enterDeepSleep();
}
