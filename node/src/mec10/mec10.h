/* 
 *  MEC-10 soil probe
 *  Federico Brunella
 */

#ifndef mec10_h
#define mec10_h

#include <SoftwareSerial.h>
#include <Arduino.h>

class mec10 {    
  public:  

    int RO, RE, DE, DI;
    float soilTemp;
    float soilVWC;
    float soilEC;

    SoftwareSerial myPort;
    
    mec10(int _RO, int _RE, int _DE, int _DI);

    void readData();
    void initialize();
    float getSoilTemp();
    float getSoilVWC();
    float getSoilEC();
    
       
  private:

    //const byte T_VWC_EC[] = {0x01, 0x04, 0x00, 0x00, 0x00, 0x03, 0xB0, 0x0B};    
    byte values[11];
    byte t_H = values[3];
    byte t_L = values[4];
    byte vwc_H = values[5];
    byte vwc_L = values[6];
    byte ec_H = values[7];
    byte ec_L = values[8];
    int start = 0;

    unsigned int calc_crc16 (unsigned char *snd, unsigned char num);
    
};

#endif
