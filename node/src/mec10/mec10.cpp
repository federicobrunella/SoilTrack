#include "mec10.h"

mec10::mec10(int _RO, int _RE, int _DE, int _DI)
{
    RO = _RO;
    RE = _RE;
    DE = _DE;
    DI = _DI;
}

float mec10::getSoilTemp()
{
    soilTemp=int(t_H << 8 | t_L) / 100.0;
    return float(soilTemp);
}
float mec10::getSoilVWC()
{
    soilVWC=int(vwc_H << 8 | vwc_L) / 100.0;
    return float(soilVWC);
}
float mec10::getSoilEC()
{
    soilEC=int(ec_H << 8 | ec_L) / 1.0;
    return float(soilEC);
}

void mec10::initialize()
{
    pinMode(RE, OUTPUT);
    pinMode(DE, OUTPUT);
    digitalWrite(DE,HIGH);
    digitalWrite(RE,HIGH);
    pinMode(DI, OUTPUT); // TX
    pinMode(RO, OUTPUT); // RX

    myPort.begin(9600, SWSERIAL_8N1, RO, DI, false);
    if (!myPort) { // If the object did not initialize, then its configuration is invalid
        Serial.println("Invalid SoftwareSerial pin configuration, check config");
        while (1) { // Don't continue with invalid configuration
        delay (1000);
        }
    }

    readData();
    delay(1000);
    readData();
    delay(1000);

}

void mec10::readData()
{
    const byte T_VWC_EC[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x03, 0xB0, 0x0B};    
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(100);

    if (myPort.write(T_VWC_EC, sizeof(T_VWC_EC))) {

        digitalWrite(DE, LOW);
        digitalWrite(RE, LOW);
        delay(100);
        for (byte i = 0; i <= 10; i++) {
        values[i] = myPort.read();
        //Serial.print(i); Serial.print("..."); Serial.println(values[i],HEX);
        }
        //Serial.println("-------------------------");
    }

    //Serial.print(values[0], HEX); Serial.print("-");
    //Serial.print(values[1], HEX); Serial.print("-");
    //Serial.print(values[2], HEX); Serial.print("-");
    t_H = values[3];  /*Serial.print(t_H, HEX); Serial.print("-");*/
    t_L = values[4];  /*Serial.print(t_L, HEX); Serial.print("-");*/
    vwc_H = values[5]; /*Serial.print(vwc_H, HEX); Serial.print("-");*/
    vwc_L = values[6]; /*Serial.print(vwc_L, HEX); Serial.print("-");*/
    ec_H = values[7]; /*Serial.print(ec_H, HEX); Serial.print("-");*/
    ec_L = values[8]; /*Serial.print(ec_L, HEX); Serial.print("-");*/
    //Serial.print(values[9], HEX); Serial.print("-");
    //Serial.println(values[10], HEX);
    byte crc16_return = calc_crc16(values, 11);
    //Serial.print("crc calcolato: "); Serial.println(crc16_return);
    if (crc16_return != 0 && start != 0) {
        ESP.restart();
    }
}

unsigned int mec10::calc_crc16 (unsigned char *snd, unsigned char num)
{
    unsigned char i, j;
    unsigned int c, crc = 0xFFFF;
    for (i = 0; i < num; i ++) {
        c = snd[i] & 0x00FF;
        crc ^= c;
        for (j = 0; j < 8; j ++) {
        if (crc & 0x0001) {
            crc >>= 1;
            crc ^= 0xA001;
        }
        else {
            crc >>= 1;
        }
        }
    }
    return (crc);
}
    
