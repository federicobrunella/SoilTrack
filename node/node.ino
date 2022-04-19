/*
*/

#include "src/battery/battery.h"
#include "src/mec10/mec10.h"

#include <SoftwareSerial.h>
#include <Arduino.h>
#include "LoRa_E32.h"
#include "esp_sleep.h"
#include "src/mec10/mec10.h";


//--------- MEC-10 -----------
#define RO 33
#define RE 25
#define DE 26
#define DI 27

mec10 sensor(RO, RE, DE, DI);
//----------------------------



//-------- POWER PIN ---------
#define BATT_LVL_EN 12
#define BATT_LVL 36
#define SHIELD_PWR_EN 32

battery BL(BATT_LVL, 1.7, 20);
//----------------------------

//---------- SLEEP -----------
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 21600      /* Time ESP32 will go to sleep (in seconds) */
unsigned long long timetosleep = TIME_TO_SLEEP * uS_TO_S_FACTOR;
//----------------------------

//------- LORA MODULE --------
#define RXD_LORA 17 //17
#define TXD_LORA 16
#define AUX 4
#define M0 21
#define M1 22

LoRa_E32 e32ttl(RXD_LORA, TXD_LORA, &Serial2, AUX, M0, M1, UART_BPS_RATE_9600, SERIAL_8N1);
void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);

struct Payload {
  String device_ID = "node_001";
  byte device_ADDL = 0x04;
  byte device_ADDH = 0x00;
  float soilTemp = 0;
  float soilVWC = 0;
  float soilEC = 0;
  float battVolt = 0;
  float battLvl = 0;
} payload;

#define ACK_TIMEOUT 10000  //millis
#define TX_ATTEMPTS 10
//----------------------------

void en_batt_lvl();
void dis_batt_lvl();

void enable_shield_pwr();
void disable_shield_pwr();

void send_over_LORA();

void setup() {
  Serial.begin(115200);

  pinMode(SHIELD_PWR_EN, OUTPUT);
  pinMode(BATT_LVL_EN, OUTPUT);

  // ------------LORA SETUP---------------
  e32ttl.begin();
  ResponseStructContainer c;
  c = e32ttl.getConfiguration();
  Configuration configuration = *(Configuration*) c.data;
  configuration.ADDL = 0x04;
  configuration.ADDH = 0x00;
  configuration.CHAN = 0x04;
  configuration.OPTION.transmissionPower = POWER_20;
  configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;
  configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
  configuration.OPTION.fec = FEC_1_ON;
  configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  configuration.SPED.uartBaudRate = UART_BPS_9600;
  configuration.SPED.uartParity = MODE_00_8N1;
  configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
  e32ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  printParameters(configuration);
  // -------------------------------------

  enable_shield_pwr();
  en_batt_lvl();
  sensor.initialize();

  // ESEGUE LETTURE
  sensor.readData();
  Serial.println(" --------------------------------");
  Serial.print("Temp (C°): ");
  Serial.println(sensor.getSoilTemp());
  payload.soilTemp = sensor.getSoilTemp();

  Serial.print("VWC (%): ");
  Serial.println(sensor.getSoilVWC());
  payload.soilVWC = sensor.getSoilVWC();

  Serial.print("EC (us/m): ");
  Serial.println(sensor.getSoilEC());
  payload.soilEC = sensor.getSoilEC();

  Serial.print("battery: ");
  payload.battVolt = BL.getBatteryVolts();
  payload.battLvl = BL.getBatteryChargeLevel();  
  Serial.println(payload.battVolt);
  Serial.println(analogRead(34));

  Serial.println(" --------------------------------");

  // TRASMETTE LE LETTURE
  send_over_LORA();

  // ENTRA IN DEEPSEEP
  disable_shield_pwr();
  dis_batt_lvl();

  esp_sleep_enable_timer_wakeup(1ULL * 60 * 60 * 1000 * 1000);
  //esp_sleep_enable_timer_wakeup(timetosleep);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  Serial.println("Going to sleep now");
  delay(500);
  Serial.flush();
  esp_deep_sleep_start();
}

void loop() {

}


void send_over_LORA() {
  unsigned long currentMillis = millis();
  unsigned long previousMillis = 0;

  bool ack_recived = false;

  for (int j = 0; j < TX_ATTEMPTS; j++) {
    Serial.print("ATTEMPT N°");
    Serial.println(j);
    Serial.println("Sending payload...");
    Serial.println("payload lenght: " + String(sizeof(payload)));
    ResponseStatus rs = e32ttl.sendBroadcastFixedMessage(0x04, &payload, sizeof(Payload));
    Serial.println(rs.getResponseDescription());

    while (currentMillis - previousMillis < ACK_TIMEOUT)
    {
      currentMillis = millis();
      if (e32ttl.available()  > 1) {
        Serial.println("WAITING FOR ACK...");
        ResponseContainer rs = e32ttl.receiveMessage();
        String message = rs.data;

        if (message == payload.device_ID)
        {
          //ACK correctly recived
          Serial.println("ACK RECIVED!");
          ack_recived = true;
          break;
        }
      }
    }

    if (ack_recived == true)
      break;
    previousMillis = currentMillis;
    Serial.println(previousMillis);
  }
}

void en_batt_lvl() {
  digitalWrite(BATT_LVL_EN, HIGH);
}

void dis_batt_lvl() {
  digitalWrite(BATT_LVL_EN, LOW);
}

void enable_shield_pwr() {
  digitalWrite(SHIELD_PWR_EN, HIGH);
}

void disable_shield_pwr() {
  digitalWrite(SHIELD_PWR_EN, LOW);
}
void printParameters(struct Configuration configuration) {
  Serial.println("----------------------------------------");

  Serial.print(F("HEAD : "));  Serial.print(configuration.HEAD, BIN); Serial.print(" "); Serial.print(configuration.HEAD, DEC); Serial.print(" "); Serial.println(configuration.HEAD, HEX);
  Serial.println(F(" "));
  Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, BIN);
  Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, BIN);
  Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, DEC); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
  Serial.println(F(" "));
  Serial.print(F("SpeedParityBit     : "));  Serial.print(configuration.SPED.uartParity, BIN); Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
  Serial.print(F("SpeedUARTDatte  : "));  Serial.print(configuration.SPED.uartBaudRate, BIN); Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRate());
  Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN); Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRate());

  Serial.print(F("OptionTrans        : "));  Serial.print(configuration.OPTION.fixedTransmission, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getFixedTransmissionDescription());
  Serial.print(F("OptionPullup       : "));  Serial.print(configuration.OPTION.ioDriveMode, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getIODroveModeDescription());
  Serial.print(F("OptionWakeup       : "));  Serial.print(configuration.OPTION.wirelessWakeupTime, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getWirelessWakeUPTimeDescription());
  Serial.print(F("OptionFEC          : "));  Serial.print(configuration.OPTION.fec, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getFECDescription());
  Serial.print(F("OptionPower        : "));  Serial.print(configuration.OPTION.transmissionPower, BIN); Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());

  Serial.println("----------------------------------------");

}
void printModuleInformation(struct ModuleInformation moduleInformation) {
  Serial.println("----------------------------------------");
  Serial.print(F("HEAD BIN: "));  Serial.print(moduleInformation.HEAD, BIN); Serial.print(" "); Serial.print(moduleInformation.HEAD, DEC); Serial.print(" "); Serial.println(moduleInformation.HEAD, HEX);

  Serial.print(F("Freq.: "));  Serial.println(moduleInformation.frequency, HEX);
  Serial.print(F("Version  : "));  Serial.println(moduleInformation.version, HEX);
  Serial.print(F("Features : "));  Serial.println(moduleInformation.features, HEX);
  Serial.println("----------------------------------------");

}
