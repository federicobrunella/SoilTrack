/* 
 *  Ion-Li battery charge
 */
  
#ifndef battery_h
#define battery_h

#include "Arduino.h"

#define DEF_PIN 4
#define DEF_CONV_FACTOR 1.7
#define DEF_READS 20

/*
 * Ion-Li battery charge
 * Calculates charge level of an Ion-Li battery
 */
class battery {    
  public:  
    
    /*
    * Constructor
    * @param addressPin, ADC pin number where the voltage divider is connected to
    */
    battery(int addressPin);
    
    /*
    * Constructor
    * @param addressPin, ADC pin number where the voltage divider is connected to
    * @param convFactor, Convertion factor for analog read units to volts
    */
    battery(int addressPin, double convFactor);
    
    /*
    * Constructor
    * @param addressPin, ADC pin number where the voltage divider is connected to
    * @param convFactor, Convertion factor for analog read units to volts
    * @param reads, Number of reads of analog pin to calculate an average value
    */
    battery(int addressPin, double convFactor, int reads);
    /*
    * Constructor
    */
    battery();    

    /*
     * Get the battery charge level (0-100)
     * @return The calculated battery charge level
     */
    int getBatteryChargeLevel();
    double getBatteryVolts();
    int getAnalogPin();
    int pinRead();
    double getConvFactor();
       
  private:

    int    _addressPin;               //!< ADC pin used, default is GPIO34 - ADC1_6
    int    _reads;                    //Number of reads of ADC pin to calculate an average value
    double _convFactor;               //!< Convertion factor to translate analog units to volts
    double _vs[101];                 //Array with voltage - charge definitions
    
    void   _initVoltsArray();
    int    _getChargeLevel(double volts);
    int    _analogRead(int pinNumber);
    double _analogReadToVolts(int readValue);
    
};

#endif