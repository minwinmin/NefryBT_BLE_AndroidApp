/*  
 *   server，1つの中にserviceが複数追加されていく感じ．このプログラムを参考に作ってみよう．severは絶対一つつくる．
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
   And has a characteristic of: beb5483e-36e1-4688-b7f5-ea07361b26a8

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/
#include <Nefry.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t value = 0;

BLECharacteristic *pCharacteristic2;
int j = 0;
int red,green,blue;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
//serviceを複数作るときは複数のUUIDが必要
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

//2つ目UUIDを設定
#define SERVICE_UUID2 "275cc101-1f61-4854-af72-57917900e866" 
#define CHARACTERISTIC_UUID2 "e64bbdc5-196d-4198-ad89-71cdf05264d1" 



class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

//classがどういう機能をもつかを示す感じ
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic2) {
      std::string j = pCharacteristic2->getValue();
         
         randomSeed(analogRead(A0));
         
         if (j.length() > 0) {
         red=random(255);          //random関数は0-255の数値をランダムに返します。
         green=random(255);
         blue=random(255);
         Nefry.setLed(red,green,blue);
         delay(1000);
         Nefry.println("On");   
         }else{
         Nefry.println("Off");
         }
    }
};
MyCallbacks myCallbacks;//名前を端的にわかりやすくした



void setup() {
  randomSeed(analogRead(A0));
  
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("MyESP32");

  // Create the BLE Server
  BLEServer *pServer = new BLEServer();
  pServer->setCallbacks(new MyServerCallbacks());//severにclassを渡す

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

    // Start the service
  pService->start();
///////////////////////////////////////////////////////////////////////////////////////////
   // Create the BLE Service
  BLEService *pService2 = pServer->createService(SERVICE_UUID2);
   //serviceのどういうことをやるかをcharacteristic(特性)を指定する，READ，WRITEなど 
  BLECharacteristic *pCharacteristic2 = pService2->createCharacteristic(
                                         CHARACTERISTIC_UUID2,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic2->setCallbacks(&myCallbacks);//特性を指定したclassにもどす．機能の追加には＆をつける．classはpythonみたいな感じ．読み込んだ値なども格納されている．
  pCharacteristic2->setValue("Hello");

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml

  // Start the service
  pService2->start();


  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {

  if (deviceConnected) {
    Serial.printf("*** NOTIFY: %d ***\n", value);
    pCharacteristic->setValue(&value, 1);
    pCharacteristic->notify();
    //pCharacteristic->indicate();
    value++;
  }
  delay(2000);
}
