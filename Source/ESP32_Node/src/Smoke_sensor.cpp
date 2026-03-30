#include "Smoke_sensor.h"
#include "data.h"

// Smoke Sensor (BM22S2021-1) - UART pins
const uint8_t SMOKE_RX_PIN = 17;
const uint8_t SMOKE_TX_PIN = 16;
const uint8_t SMOKE_STATUS_PIN = 32; // Status pin for smoke detection interrupt
uint16_t smokeValueA, smokeValueB;

BM22S2021_1 smokeSensor(SMOKE_STATUS_PIN, SMOKE_RX_PIN, SMOKE_TX_PIN);

void initSmokeSensor()
{
    // ---- Initialize GPIO ----
    pinMode(SMOKE_STATUS_PIN, INPUT);

    // ---- Initialize Smoke Sensor (UART) ----
    smokeSensor.begin();
    delay(500);
    uint8_t smokeStatus = smokeSensor.getSTATUS(); // 0 = no smoke, 1 = smoke detected
    Serial.print("[INFO] Smoke sensor status: 0x");
    Serial.println(smokeStatus, HEX);
}

void printDataSmokeSensor(struct_message *data)
{
    // Request smoke sensor data package
    uint8_t smokeBuffer[41] = {0};
    uint16_t smokeValueA, smokeValueB;
    if (smokeSensor.isInfoAvailable() == 1) 
    {
        smokeSensor.readInfoPackage(smokeBuffer);
        smokeValueA = (smokeBuffer[17] << 8 | smokeBuffer[16]);
        Serial.print("Smoke value A is: ");
        Serial.println(smokeValueA, DEC);

        smokeValueB = (smokeBuffer[19] << 8 | smokeBuffer[18]);
        Serial.print("Smoke value B is: ");
        Serial.println(smokeValueB, DEC);

        // ================================================
        data->smokeValueA = smokeValueA;
        data->smokeValueB = smokeValueB;
    } else {
        Serial.println("FAILED !");
        data->smokeValueA = -99;
        data->smokeValueB = -99;
    }
}