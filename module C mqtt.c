/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <OneWire.h>
#include <DallasTemperature.h>


// GPIO where the DS18B20 is connected to
RTC_DATA_ATTR const int oneWireBus = 4;     
// battery is connected to GPIO 34 
RTC_DATA_ATTR const int batpin = 34;

// variable for storing the battery value
RTC_DATA_ATTR float batValue;
RTC_DATA_ATTR float batVolts;

RTC_DATA_ATTR unsigned long entry;
RTC_DATA_ATTR float temperatureC;
RTC_DATA_ATTR char valueStr[5];
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID       ((uint16_t)0x1700)
#define CHARACTERISTIC_UUID_T ((uint16_t)0x2A6E) //ismatuota temperatura
#define CHARACTERISTIC_UUID_Battery ((uint16_t)0x180F) //baterijos lygis
#define CHARACTERISTIC_UUID_Sleep ((uint16_t)0x1234) //charakteristika miegui

 void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  Serial.begin(115200);
  
  // Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

sensors.begin();// Start the DS18B20 sensor
 delay(2000);
    Serial.println("Measure temperature");
   sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);

    
 batValue= analogRead(batpin); // matuojamas baterijos lygis
  
  Serial.println("Starting BLE work!");

  BLEDevice::init("Jutiklis");  //create and name the BLE device
  
  BLEServer *pServer = BLEDevice::createServer(); // create the BLE server 
  BLEService *pService = pServer->createService(SERVICE_UUID); //add service to server
  
  BLECharacteristic *Characteristic_T = pService->createCharacteristic( //add characteristic to server
                                         CHARACTERISTIC_UUID_T,
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_READ
                                       );

   BLECharacteristic *Characteristic_Battery = pService->createCharacteristic( //add characteristic to server
                                         CHARACTERISTIC_UUID_Battery,
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_READ
                                       );
   BLECharacteristic *Characteristic_Sleep = pService->createCharacteristic( //add characteristic to server
                                         CHARACTERISTIC_UUID_Sleep,
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_READ
                                         );

  Characteristic_T->addDescriptor(new BLE2902());
  Characteristic_Battery->addDescriptor (new BLE2902());
  Characteristic_Sleep->addDescriptor (new BLE2902());
  
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
   
            
            // change value to char
            char txString[5]; 
            char BatterylevelString[3];
            dtostrf(temperatureC, 2, 1, txString); // float_val, min_width, digits_after_decimal, char_buffer
            delay (5);
            Characteristic_T->setValue(txString); //set characteristic value 
           
            batValue= analogRead(batpin);
            
            batVolts=(batValue*(3.3/4095)*2.73); //LSB * 9 / 3,3 V
            Serial.println("Battery level ");
            Serial.println(batVolts);
            Serial.print(" V");
            dtostrf(batVolts, 4, 2, BatterylevelString); // float_val, min_width, digits_after_decimal, char_buffer
            Characteristic_Battery->setValue(BatterylevelString);
            delay (5);
         
       
         while (1){
         std::string sleepflag = Characteristic_Sleep->getValue();
         if (sleepflag[0]!=0){
            esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
            Serial.println("Sensor is going to sleep");
            esp_deep_sleep_start();
            }
            } 
      
  
  }


void loop() {}