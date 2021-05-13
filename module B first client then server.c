/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include "BLEDevice.h"
#include "BLEScan.h"
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <iostream>
#include <string>
#include <algorithm>
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10  


RTC_DATA_ATTR unsigned long entry;
RTC_DATA_ATTR const int batpin = 34;
RTC_DATA_ATTR float batValue;
RTC_DATA_ATTR char BatteryServer[4];
RTC_DATA_ATTR char TemperatureServer[6];


  
#define SERVICE_UUID_SERVER       ((uint16_t)0x1701) //define velesniam serveriui
#define CHARACTERISTIC_UUID_Temperature ((uint16_t)0x2A6E) //gauta temperatura
#define CHARACTERISTIC_UUID_BatteryServer ((uint16_t)0x180F) //gautas serverio baterijos lygis
#define CHARACTERISTIC_UUID_BatteryRelay ((uint16_t)0x1801) //gautas tarpinio  baterijos lygis
#define CHARACTERISTIC_UUID_Sleep ((uint16_t)0x2345) //miegui

// The remote service we wish to connect to.
RTC_DATA_ATTR static BLEUUID serviceUUID(((uint16_t)0x1700));
// The characteristic of the remote service we are interested in.
RTC_DATA_ATTR static BLEUUID    charUUID(((uint16_t)0x2A6E)); //temperaturos lygis
RTC_DATA_ATTR static BLEUUID    charUUID1(((uint16_t)0x180F)); //jutiklio baterijos lygis
RTC_DATA_ATTR static BLEUUID    charUUID2(((uint16_t)0x1234)); //jutiklio miegui

RTC_DATA_ATTR static boolean doConnect = false;
RTC_DATA_ATTR static boolean connected = false;
RTC_DATA_ATTR static boolean doScan = false;
RTC_DATA_ATTR static BLERemoteCharacteristic* pRemoteCharacteristic_temp;
RTC_DATA_ATTR static BLERemoteCharacteristic* pRemoteCharacteristic_jutikliobat;
RTC_DATA_ATTR static BLERemoteCharacteristic* pRemoteCharacteristic_jutikliomiegui;

RTC_DATA_ATTR static BLEAdvertisedDevice* myDevice;

RTC_DATA_ATTR static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
   for (int i = 0;i<length; i++){
   Serial.print(pData[i]);
   Serial.print(" ");
   Serial.println();}
   }


RTC_DATA_ATTR class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

RTC_DATA_ATTR bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");
   


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic_temp = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic_temp == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our temperature characteristic");


   
    // Read the value of the characteristic.
    if(pRemoteCharacteristic_temp->canRead()) {
      std::string value = pRemoteCharacteristic_temp->readValue();
      Serial.print("The characteristic temperature value was: ");
      Serial.println(value.c_str());
      std::copy(value.begin(), value.end(), TemperatureServer);
      TemperatureServer[value.size()] = '\0';
    
          }
      // obtain a reference to battery level characteristic
      pRemoteCharacteristic_jutikliobat = pRemoteService->getCharacteristic(charUUID1);
    if (pRemoteCharacteristic_jutikliobat == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our battery characteristic");
   
    // Read the value of the characteristic.
    if(pRemoteCharacteristic_jutikliobat->canRead()) {
      std::string value1 = pRemoteCharacteristic_jutikliobat->readValue();
      Serial.print("The characteristic battery value was: ");
      Serial.println(value1.c_str());
      std::copy(value1.begin(), value1.end(), BatteryServer);
      BatteryServer[value1.size()] = '\0';    
      }
      else Serial.print ("Cannot read the characteristic");

      pRemoteCharacteristic_jutikliomiegui = pRemoteService->getCharacteristic(charUUID2);
    if (pRemoteCharacteristic_jutikliomiegui == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our sensor sleep characteristic");
   
    // Read the value of the characteristic.
    if(pRemoteCharacteristic_jutikliomiegui->canWrite()) {
      Serial.println("Sending instruction to go to sleep");
      pRemoteCharacteristic_jutikliomiegui->writeValue((byte)0x01);
    }
      
   /* if(pRemoteCharacteristic_temp->canNotify())
      pRemoteCharacteristic_temp->registerForNotify(notifyCallback);
    if(pRemoteCharacteristic_jutikliobat->canNotify())
      pRemoteCharacteristic_jutikliobat->registerForNotify(notifyCallback); */

    connected = true;
    return true;
    pClient->disconnect();
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
RTC_DATA_ATTR class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks
     


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("Relay");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

connectToServer();
Serial.println("Got the data, disconnecting from server.");


 // charakteristikos
  Serial.println("Creating BLE server!");

  BLEServer *pServer = BLEDevice::createServer(); // create the BLE server 
  BLEService *pService = pServer->createService(SERVICE_UUID_SERVER); //add service to server
  
  BLECharacteristic *Characteristic_Temperatura = pService->createCharacteristic( //add characteristic to server
                                         CHARACTERISTIC_UUID_Temperature,
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_READ
                                       );

   BLECharacteristic *Characteristic_BatteryServer = pService->createCharacteristic( //add characteristic to server
                                         CHARACTERISTIC_UUID_BatteryServer,
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_READ
                                       );

   BLECharacteristic *Characteristic_BatteryRelay = pService->createCharacteristic( //add characteristic to server
                                         CHARACTERISTIC_UUID_BatteryRelay,
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_READ
                                       );
   
   BLECharacteristic *Characteristic_Sleep = pService->createCharacteristic( //add characteristic to server
                                         CHARACTERISTIC_UUID_Sleep,
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_READ
                                       );                                    
                                      
  Characteristic_Temperatura->addDescriptor(new BLE2902());
  Characteristic_BatteryServer->addDescriptor (new BLE2902());
  Characteristic_BatteryRelay->addDescriptor (new BLE2902());
  Characteristic_Sleep->addDescriptor (new BLE2902());
  
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID_SERVER);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

            Characteristic_Temperatura->setValue(TemperatureServer); //set characteristic value 
          //  Characteristic_Temperatura->notify(); // send value
            Serial.println("Serverio ismatuota temperatura");
            Serial.println(TemperatureServer);
            Characteristic_BatteryServer->setValue(BatteryServer); //set characteristic value 
         //   Characteristic_BatteryServer->notify(); // send value
            Serial.println("Serverio baterijos lygis");
            Serial.println(BatteryServer);
            float BatteryRelay; float batRelay; char batRelayString[3];
            BatteryRelay=analogRead(batpin);
            Serial.println(batRelayString);
            batRelay=(BatteryRelay*(3.3/4095)*2.73);
            dtostrf(batRelay, 3, 1, batRelayString);//float_val, min_width, digits_after_decimal, char_buffer
            Serial.println("Tarpinio mazgo baterijos lygis");
            Serial.println(batRelayString);
            Characteristic_BatteryRelay->setValue(batRelayString); //set characteristic value 
         //   Characteristic_BatteryRelay->notify(); // send value
            while (1){
         std::string sleepflag = Characteristic_Sleep->getValue();
         if (sleepflag[0]!=0){
            esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
            Serial.println("Sensor is going to sleep");
            esp_deep_sleep_start();
            }
            }
            
   } // End of setup.


// This is the Arduino main loop function.
void loop(){
} // End of loop