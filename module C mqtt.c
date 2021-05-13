/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include "BLEDevice.h"
#include "BLEScan.h"

#include <WiFi.h>
#include <PubSubClient.h>
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10  
#define ssid          "Ka sakei?"
#define password      "********"
#define SERVER          "io.adafruit.com"
#define SERVERPORT      1883
#define MQTT_USERNAME   "SJuociunas"
#define MQTT_KEY        "***********"
#define USERNAME          "SJuociunas/"
#define PREAMBLE          "feeds/"
#define T_TEMPERATURE      "temperatura"
#define T_SERVERIS        "serverio-baterijos-lygis"
#define T_JUTIKLIO BAT    "jutiklio-lygis"
#define T_TARPINIO BAT    "tarpinio-lygis"
#define T_CLIENTSTATUS    "clientStatus"
#define T_COMMAND         "command"
RTC_DATA_ATTR char BatterySensor[4];
RTC_DATA_ATTR char TemperatureSensor[6];
RTC_DATA_ATTR char BatteryRelay[4];
RTC_DATA_ATTR WiFiClient WiFiClient;
RTC_DATA_ATTR PubSubClient client(WiFiClient);
RTC_DATA_ATTR unsigned long entry;
RTC_DATA_ATTR const int batpin = 36;
RTC_DATA_ATTR float batValue;
  
// The remote service we wish to connect to.
RTC_DATA_ATTR static BLEUUID serviceUUID(((uint16_t)0x1701));
// The characteristic of the remote service we are interested in.
RTC_DATA_ATTR static BLEUUID    charUUID(((uint16_t)0x2A6E)); //temperaturos lygis
RTC_DATA_ATTR static BLEUUID    charUUID1(((uint16_t)0x180F)); //jutiklio baterijos lygis
RTC_DATA_ATTR static BLEUUID    charUUID2(((uint16_t)0x1801)); //tarpinio mazgo baterijos lygis
RTC_DATA_ATTR static BLEUUID    charUUID3(((uint16_t)0x2345)); //sleep tarpinis


RTC_DATA_ATTR static boolean doConnect = false;
RTC_DATA_ATTR static boolean connected = false;
RTC_DATA_ATTR static boolean doScan = false;
RTC_DATA_ATTR static BLERemoteCharacteristic* pRemoteCharacteristic_SensorTemperature;
RTC_DATA_ATTR static BLERemoteCharacteristic* pRemoteCharacteristic_SensorBattery;
RTC_DATA_ATTR static BLERemoteCharacteristic* pRemoteCharacteristic_RelayBattery;
RTC_DATA_ATTR static BLERemoteCharacteristic* pRemoteCharacteristic_Sleep;

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
    pRemoteCharacteristic_SensorTemperature = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic_SensorTemperature == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our temperature characteristic");


   
    // Read the value of the characteristic.
    pRemoteCharacteristic_SensorTemperature = pRemoteService->getCharacteristic(charUUID);
    if(pRemoteCharacteristic_SensorTemperature->canRead()) {
      std::string value = pRemoteCharacteristic_SensorTemperature->readValue();
      Serial.print("The characteristic temperature value was: ");
      Serial.println(value.c_str());
      std::copy(value.begin(), value.end(), TemperatureSensor);
      TemperatureSensor[value.size()] = '\0';
     /* const char temperature [5];
      temperature=value.c_str();
      Serial.println("Temp:");
      Serial.println(temperature);*/
      }
      // obtain a reference to battery level characteristic
      pRemoteCharacteristic_SensorBattery = pRemoteService->getCharacteristic(charUUID1);
    if (pRemoteCharacteristic_SensorBattery == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our server battery characteristic");
   
    // Read the value of the server battery characteristic.
    if(pRemoteCharacteristic_SensorBattery->canRead()) {
      std::string value1 = pRemoteCharacteristic_SensorBattery->readValue();
      Serial.print("The characteristic server battery value was: ");
      Serial.println(value1.c_str());
       std::copy(value1.begin(), value1.end(), BatterySensor);
      BatterySensor[value1.size()] = '\0';

      // obtain a reference to relay node battery level characteristic
      pRemoteCharacteristic_RelayBattery = pRemoteService->getCharacteristic(charUUID2);
    if (pRemoteCharacteristic_RelayBattery == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our relay battery characteristic");
    
    // Read the value of the relay battery characteristic.
    if(pRemoteCharacteristic_RelayBattery->canRead()) {
      std::string value2 = pRemoteCharacteristic_RelayBattery->readValue();
      Serial.print("The characteristic relay battery value was: ");
      Serial.println(value2.c_str());
      std::copy(value2.begin(), value2.end(), BatteryRelay);
      BatteryRelay[value2.size()] = '\0';
      
// obtain a reference to sleep characteristic
      pRemoteCharacteristic_Sleep = pRemoteService->getCharacteristic(charUUID3);
        if (pRemoteCharacteristic_Sleep == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our sleep characteristic");
   
    // write to sleep characteristic.
      
      if(pRemoteCharacteristic_Sleep->canWrite()) {
      Serial.println("Sending instruction to go to sleep");
      pRemoteCharacteristic_Sleep->writeValue((byte)0x01);
    }

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.printDiag(Serial);

  yield();
  
  if (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("", MQTT_USERNAME, MQTT_KEY)) {
      Serial.println("connected");}
      else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
    }
    }
  //  std::string value = pRemoteCharacteristic_ServerTemperature->readValue();
  //  std::string value1 = pRemoteCharacteristic_ServerBattery->readValue();
  //  std::string value2 = pRemoteCharacteristic_RelayBattery->readValue();
    
   
    // create MQTT object
    
      client.setServer(SERVER, SERVERPORT);

    batValue=analogRead(batpin);
    Serial.println(batValue);
    char batServerString[4];  
    float batServer=(batValue*(3.3/4095)*2.73);
    dtostrf(batServer, 3, 1, batServerString);//float_val, min_width, digits_after_decimal, char_buffer    
    Serial.println("Temperature value is ");
    Serial.print(TemperatureSensor);
    Serial.print("°C");
    Serial.println("");
    Serial.println("Sensor battery value is ");
    Serial.print(BatterySensor);
    Serial.print(" V");
    Serial.println("");
    Serial.println("Relay battery value is ");
    Serial.print(BatteryRelay);
    Serial.print(" V");
    Serial.println("");
    Serial.println("Server battery level is ");
    Serial.print(batServerString);
    Serial.print(" V");
    client.publish(USERNAME PREAMBLE "temperatura",TemperatureSensor);
    client.publish(USERNAME PREAMBLE "serverio-baterijos-lygis",batServerString);
    client.publish(USERNAME PREAMBLE "jutiklio-lygis",BatterySensor);
    client.publish(USERNAME PREAMBLE "tarpinio-lygis",BatteryRelay);
    
    client.loop();
     
    }
      else Serial.print ("Cannot read the characteristic");
    
    connected = true;
    return true;

} //connect to server
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
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("Serveris");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 2 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(2, false);

connectToServer();
Serial.println("Data uploaded to io.adafruit.com server");
esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
esp_deep_sleep_start();

   

} // End of setup.


// This is the Arduino main loop function.
void loop(){
} // End of loop
