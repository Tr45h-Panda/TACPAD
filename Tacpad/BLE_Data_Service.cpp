#include "BLE_Data_Service.h"
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID  "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_UUID_RX  "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_UUID_TX  "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

static BLECharacteristic* txChar = nullptr;
static QueueHandle_t rx_queue;

struct RxMsg { char text[BLE_RX_MAXLEN]; };

class RxCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    String value = characteristic->getValue();
    RxMsg msg;
    size_t len = value.length();
    if (len >= BLE_RX_MAXLEN) len = BLE_RX_MAXLEN - 1;
    memcpy(msg.text, value.c_str(), len);
    msg.text[len] = '\0';
    xQueueSend(rx_queue, &msg, 0);
    printf("BLE Data : received '%s'\r\n", msg.text);
}
};

void BLE_DataService_Init(BLEServer* server)
{
  rx_queue = xQueueCreate(4, sizeof(RxMsg));

  BLEService* service = server->createService(SERVICE_UUID);

  txChar = service->createCharacteristic(CHAR_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  txChar->addDescriptor(new BLE2902());

  BLECharacteristic* rxChar = service->createCharacteristic(CHAR_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  rxChar->setCallbacks(new RxCallbacks());

  service->start();
  printf("BLE Data : custom RX/TX service started\r\n");
}

void BLE_SendData(const char* text)
{
  if (!txChar) return;
  txChar->setValue((uint8_t*)text, strlen(text));
  txChar->notify();
}

bool BLE_PollReceived(char* outBuf, size_t outBufLen)
{
  RxMsg msg;
  if (xQueueReceive(rx_queue, &msg, 0) == pdTRUE) {
    strlcpy(outBuf, msg.text, outBufLen);
    return true;
  }
  return false;
}