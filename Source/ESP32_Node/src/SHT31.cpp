#include "SHT31.h"

// I2C Pins for SHT31 (default: GPIO 21 = SDA, GPIO 22 = SCL)
const uint8_t I2C_SDA = 21;
const uint8_t I2C_SCL = 22;

Adafruit_SHT31 sht31 = Adafruit_SHT31();

void Init_Sht31()
{
  // ---- Initialize I2C for SHT31 ----
  Wire.begin(I2C_SDA, I2C_SCL);

  // Initialize SHT31 Temperature/Humidity Sensor
  if (!sht31.begin(0x44))
  {
    Serial.println("[ERROR] SHT31 sensor not found!");
  }
  else
  {
    Serial.println("[OK] SHT31 initialized");
  }
}

void printDataSht31(struct_message *data)
{
  // ---- Read SHT31 Temperature & Humidity ----
  float temp = sht31.readTemperature();
  float humidity = sht31.readHumidity();

  if (!isnan(temp) && !isnan(humidity))
  {
    Serial.print("[SHT31] Temp: ");
    Serial.print(temp, 1);
    Serial.print("°C | Humidity: ");
    Serial.print(humidity, 1);
    Serial.println("%");
    data->temp = temp;
    data->humi = humidity;
  }
  else
  {
    Serial.println("[ERROR] Failed to read SHT31");
    data->temp = -99.0;
    data->humi = -99.0;
  }
}

