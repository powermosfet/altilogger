#define PIN_ACTION 13 //D7
#define MEASUREMENT_COUNT 200
#define MEASUREMENT_INTERVAL 3000
#define EEPROM_ADDR 0

#include <ESP_EEPROM.h>
#include <EnvironmentCalculations.h>
#include <BME280I2C.h>
#include <Wire.h>

extern "C" {
#include "user_interface.h"
}

void printRamInfo() {
  uint32_t freeRam = system_get_free_heap_size();
  Serial.printf("Free memory: %d", freeRam); Serial.println("");
}

struct Measurement {
  float temp;
  float hum;
  float pres;
  float altitude;
} measurement;

struct Measurement measurements[MEASUREMENT_COUNT];
int measurementIndex = 0;
bool recordActive = false;
int recordStart;

float referencePressure = 1018.6;  
float outdoorTemp = 4.7;
BME280I2C::Settings settings(
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::Mode_Forced,
   BME280::StandbyTime_1000ms,
   BME280::Filter_Off,
   BME280::SpiEnable_False,
   BME280I2C::I2CAddr_0x76
);
BME280I2C bme(settings);
float temp(NAN), hum(NAN), pres(NAN), altitude(NAN);
BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
BME280::PresUnit presUnit(BME280::PresUnit_hPa);
EnvironmentCalculations::AltitudeUnit envAltUnit  =  EnvironmentCalculations::AltitudeUnit_Meters;
EnvironmentCalculations::TempUnit     envTempUnit =  EnvironmentCalculations::TempUnit_Celsius;

void blink(int count) {
  digitalWrite(LED_BUILTIN, LOW);
  delay(150);
  digitalWrite(LED_BUILTIN, HIGH);

  if(count > 1) {
    delay(200);
    blink(count - 1);
  }
}

void prepareRecord() {
  Serial.println("Recording...");
  blink(1);

  bme.read(pres, temp, hum, tempUnit, presUnit);
  referencePressure = pres;
  outdoorTemp = temp;
  recordStart = millis();
  recordActive = true;
}

void report() {
  Serial.println("index,temperature,humidity,pressure,altitude");
  blink(3);
  EEPROM.get(EEPROM_ADDR, measurements);
  for(int i=0; i<MEASUREMENT_COUNT; i++) {
    Serial.printf("%d,%f,%f,%f,%f\n", i, measurements[i].temp, measurements[i].hum, measurements[i].pres, measurements[i].altitude);
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_ACTION, INPUT);
  Serial.begin(115200);
  while(!Serial) delay(100);
  Serial.println("Bleep bloop");
  digitalWrite(LED_BUILTIN, HIGH);
  Wire.begin();
  delay(2000);
  while(!bme.begin())
  {
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }

  EEPROM.begin(MEASUREMENT_COUNT * sizeof(Measurement));

  if(digitalRead(PIN_ACTION) == HIGH) {
    printRamInfo();
    prepareRecord();
  } else {
    report();
  }
}

void loop() {
  if(recordActive) {
    if(millis() > (recordStart + (measurementIndex * MEASUREMENT_INTERVAL))) {
      bme.read(pres, temp, hum, tempUnit, presUnit);
      measurement.temp = temp;
      measurement.hum = hum;
      measurement.pres = pres;
      measurement.altitude = EnvironmentCalculations::Altitude(pres, envAltUnit, referencePressure, outdoorTemp, envTempUnit);
      measurements[measurementIndex] = measurement;
      Serial.printf("[%03d]: %f\r\n", measurementIndex, measurement.altitude);
      measurementIndex++;
    }
    if(measurementIndex == MEASUREMENT_COUNT) {
      boolean commitSuccess;
      for(int i = 0; i < MEASUREMENT_COUNT; i++) {
        EEPROM.put(EEPROM_ADDR + (i * sizeof(Measurement)), measurements[i]);
      }
      commitSuccess = EEPROM.commit();
      Serial.println((commitSuccess) ? "Commit OK" : "Commit failed");
      recordActive = false;
      Serial.println("Done!");
      printRamInfo();
    }
  } else {
    delay(1000);
    blink(5);
  }
}
