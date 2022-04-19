#include "mec10.h"

#define SHIELD_PWR_EN 32


//RS485 pin definition
#define RO 33
#define RE 25
#define DE 26
#define DI 27

mec10 sensor(RO, RE, DE, DI);

void setup() {
  Serial.begin(115200);
  pinMode(SHIELD_PWR_EN, OUTPUT);
  digitalWrite(SHIELD_PWR_EN, HIGH);

  sensor.initialize();
}

void loop() {
  sensor.readData();
  Serial.println(" --------------------------------");
  Serial.print("Temp (C°): ");
  Serial.println(sensor.getSoilTemp());

  Serial.print("VWC (%): ");
  Serial.println(sensor.getSoilVWC());

  Serial.print("EC (us/m): ");
  Serial.println(sensor.getSoilEC());
  Serial.println(" --------------------------------");

  delay(4000);
}
