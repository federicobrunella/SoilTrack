#include "Arduino.h"
#include "LoRa_E32.h"
#include <WiFi.h>
#include <HTTPClient.h>


// ---------------------------LORA Settings----------------------------------------------------
#define RX 17
#define TX 16

#define AUX 4
#define M0 21
#define M1 22

LoRa_E32 e32ttl(RX, TX, &Serial2, AUX, M0, M1, UART_BPS_RATE_9600, SERIAL_8N1);

void postData(String payload);
void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);
// -------------------------------------------------------------------------------------------

// --------------------------Wi-Fi settings---------------------------------------------------
const char* ssid = "TIM-86917939";
const char* password = "9fme7kQsuTuemUIKq4TE2QEz";
//const char* ssid = "federico";
//const char* password = "federico";
//Your Domain name with URL path or IP address with path
const char* serverName = "http://www.iotprojects.it/users/demo2/RXpost.php";

#define LED 23
// -------------------------------------------------------------------------------------------

// --------------------------Data Struct------------------------------------------------------
struct Payload {
  String device_ID = "node_001";
  byte device_ADDL = 0x00;
  byte device_ADDH = 0x00;
  float soilTemp = 0;
  float soilVWC = 0;
  float soilEC = 0;
  float battVolt = 0;
  float battLvl = 0;
};
// -------------------------------------------------------------------------------------------

void sendACK(Payload payload);

void setup()
{
  //--------------------------------------------------------------------
  Serial.begin(115200);
  delay(1000);
  //--------------------------------------------------------------------
  pinMode(LED, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(LED, HIGH);
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  //--------------------------------------------------------------------

  e32ttl.begin();
  ResponseStructContainer c;
  c = e32ttl.getConfiguration();
  Configuration configuration = *(Configuration*) c.data;
  configuration.ADDL = BROADCAST_ADDRESS;
  configuration.ADDH = BROADCAST_ADDRESS;
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
  //--------------------------------------------------------------------
  Serial.println();
  Serial.println("Start listening!");
}

// The loop function is called in an endless loop
void loop()
{
  //Serial.println(String(sizeof(Payload)));
  if (e32ttl.available()  > 1) {
    ResponseStructContainer rs = e32ttl.receiveMessage(sizeof(Payload));
    struct Payload payload = *(Payload*)rs.data;

    Serial.println(rs.status.getResponseDescription());

    sendACK(payload);

    String payload_str = "{\"payload\":{\"ID\":\"" + String(payload.device_ID) + "\",\"SoilTemp\":\"" + String(payload.soilTemp)
                         + "\",\"SoilVWC\":\"" + String(payload.soilVWC) + "\",\"SoilEC\":\"" + String(payload.soilEC) + "\",\"BattVolt\":\"" + String(payload.battVolt) + "\",\"BattLvl\":\"" + String(payload.battLvl) + "\"}}";
    Serial.println(payload_str);
    //spengo il modulo radio durante la richiesta post
    ResponseStructContainer c;
    c = e32ttl.getConfiguration();
    Configuration configuration = *(Configuration*) c.data;
    e32ttl.setMode(MODE_3_SLEEP);
    e32ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);

    postData(payload_str);

    c = e32ttl.getConfiguration();
    configuration = *(Configuration*) c.data;
    e32ttl.setMode(MODE_0_NORMAL);
    e32ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);

    //ESP.restart();
  }
}

void sendACK(Payload payload) {
  Serial.println("SENDING ACK...");
  ResponseStatus rs = e32ttl.sendFixedMessage(payload.device_ADDH, payload.device_ADDL, 0x04, payload.device_ID);
  Serial.println(rs.getResponseDescription());
  Serial.println("ACK SUCCESFULLY SENT...");
}

void printParameters(struct Configuration configuration) {
  Serial.println("----------------------------------------");

  Serial.print(F("HEAD : "));  Serial.print(configuration.HEAD, BIN); Serial.print(" "); Serial.print(configuration.HEAD, DEC); Serial.print(" "); Serial.println(configuration.HEAD, HEX);
  Serial.println(F(" "));
  Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, DEC);
  Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, DEC);
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

void postData(String payload) {
  Serial.println("-------------------------------------------------------------------------------------------------------------");
  Serial.println("Richiesta POST inviata:");
  Serial.println(payload);

  //Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Your Domain name with URL path or IP address with path
    http.begin(serverName);

    // If you need an HTTP request with a content type: application/json, use the following:
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(payload);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.println("-------------------------------------------------------------------------------------------------------------");


    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
    //provo la riconnessione alla rete
    //da pensare un comportamento per i led per identificare gli eventuali problemi
  }
}
