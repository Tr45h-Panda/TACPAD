#include "BLE_Driver.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEHIDDevice.h>
#include <BLESecurity.h>
#include <HIDTypes.h>
#include <HIDKeyboardTypes.h>
#include "BLE_Data_Service.h"

constexpr const char* DEVICE_NAME = "ESP32S3 TACPAD";

bool isBleConnected = false;
bool sent = false;

BLEHIDDevice* hid = nullptr;
BLECharacteristic* input = nullptr;


static const uint8_t REPORT_MAP[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)

    0x85, 0x01,        // Report ID (1)

    0x05, 0x07,        // Usage Page (Keyboard)
    0x19, 0xE0,        // Usage Minimum (224)
    0x29, 0xE7,        // Usage Maximum (231)
    0x15, 0x00,        // Logical Minimum (0)
    0x25, 0x01,        // Logical Maximum (1)
    0x75, 0x01,        // Report Size (1)
    0x95, 0x08,        // Report Count (8)
    0x81, 0x02,        // Input (Data, Variable, Absolute)

    0x95, 0x01,        // Report Count (1)
    0x75, 0x08,        // Report Size (8)
    0x81, 0x01,        // Input (Constant)

    0x95, 0x06,        // Report Count (6)
    0x75, 0x08,        // Report Size (8)
    0x15, 0x00,        // Logical Minimum (0)
    0x25, 0x65,        // Logical Maximum (101)
    0x05, 0x07,        // Usage Page (Keyboard)
    0x19, 0x00,        // Usage Minimum (0)
    0x29, 0x65,        // Usage Maximum (101)
    0x81, 0x00,        // Input (Data, Array)

    0x05, 0x08,        // Usage Page (LEDs)
    0x19, 0x01,        // Usage Minimum (1)
    0x29, 0x05,        // Usage Maximum (5)
    0x95, 0x05,        // Report Count (5)
    0x75, 0x01,        // Report Size (1)
    0x91, 0x02,        // Output (Data, Variable, Absolute)

    0x95, 0x01,        // Report Count (1)
    0x75, 0x03,        // Report Size (3)
    0x91, 0x01,        // Output (Constant)

    0xC0               // End Collection
};
struct InputReport {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t pressedKeys[6];
};

static const InputReport NO_KEY_PRESSED = {
    0,
    0,
    {0,0,0,0,0,0}
};

class BleKeyboardCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* server) override {
        isBleConnected = true;
        printf("BLE client connected\r\n");
    }
    void onDisconnect(BLEServer* server) override {
        isBleConnected = false;
        printf("BLE client disconnected\r\n");
    }
};

static void sendReport(const InputReport& report) {
    
    if (!input) return;

    input->setValue((uint8_t*)&report, sizeof(report));
    input->notify();
}

void bluetoothTask(void*) {
    printf("bluetoothTask: started\r\n");

    BLEDevice::init(DEVICE_NAME);
    printf("checkpoint: BLEDevice::init done\r\n");

    BLEServer* server = BLEDevice::createServer();
    server->setCallbacks(new BleKeyboardCallbacks());
    printf("checkpoint: server created\r\n");

    hid = new BLEHIDDevice(server);
    input = hid->inputReport(1);
    hid->manufacturer()->setValue("TACPAD");
    hid->pnp(0x02, 0xE502, 0xA111, 0x0210);
    hid->hidInfo(0x00, 0x01);
    hid->reportMap((uint8_t*)REPORT_MAP, sizeof(REPORT_MAP));
    printf("checkpoint: HID configured\r\n");

    hid->startServices();
    printf("checkpoint: HID services started\r\n");

    hid->setBatteryLevel(100);

    printf("checkpoint: before BLE_DataService_Init\r\n");
    BLE_DataService_Init(server);
    printf("checkpoint: after BLE_DataService_Init\r\n");

    BLESecurity* security = new BLESecurity();
    security->setAuthenticationMode(ESP_LE_AUTH_BOND);
    printf("checkpoint: security configured\r\n");

    BLEAdvertising* advertising = server->getAdvertising();
    advertising->setAppearance(HID_KEYBOARD);
    advertising->addServiceUUID(hid->hidService()->getUUID());
    printf("checkpoint: advertising configured\r\n");

    advertising->start();
    printf("checkpoint: advertising started\r\n");

    printf("BLE keyboard advertising\r\n");

    vTaskDelete(nullptr);
}

#define KEY_QUEUE_LEN   8
#define KEY_SEQ_MAXLEN  16

static QueueHandle_t key_queue;

struct KeySeqMsg {
  char text[KEY_SEQ_MAXLEN];
};

static void KeySequenceTask(void *parameter)
{
  KeySeqMsg msg;
  while (1) {
    if (xQueueReceive(key_queue, &msg, portMAX_DELAY) == pdTRUE) {
      sendKeySequence(msg.text);   // blocking is fine here — this task has nothing else to do
    }
  }
}

void BLE_Queue_Init(void)
{
  key_queue = xQueueCreate(KEY_QUEUE_LEN, sizeof(KeySeqMsg));
  xTaskCreate(KeySequenceTask, "KeySeqTask", 4096, NULL, 1, NULL);
}

void Queue_KeySequence(const char* sequence)
{
  KeySeqMsg msg;
  strlcpy(msg.text, sequence, sizeof(msg.text));
  if (xQueueSend(key_queue, &msg, 0) != pdTRUE) {
    printf("BLE : key sequence queue full, dropped '%s'\r\n", sequence);
  }
}

void sendKeySequence(const char* sequence) 
{

    if (!isBleConnected || !input)
    {
        return;
    }

    constexpr uint8_t MOD_CTRL = 0x01;

    constexpr uint8_t KEY_RIGHT = 0x4F;
    constexpr uint8_t KEY_LEFT  = 0x50;
    constexpr uint8_t KEY_DOWN  = 0x51;
    constexpr uint8_t KEY_UP    = 0x52;

    InputReport ctrlHeld = {
        MOD_CTRL,
        0,
        {0,0,0,0,0,0}
    };
  
    sendReport(ctrlHeld);
    delay(75);
    for (int i = 0; sequence[i] != '\0'; i++) {

        uint8_t keycode = 0;

        switch (sequence[i]) {

            case 'R':
                keycode = KEY_RIGHT;
                break;

            case 'L':
                keycode = KEY_LEFT;
                break;

            case 'U':
                keycode = KEY_UP;
                break;

            case 'D':
                keycode = KEY_DOWN;
                break;

            default:
                continue;
        }
        InputReport press = {
            MOD_CTRL,
            0,
            {keycode, 0, 0, 0, 0, 0}
        };  

        sendReport(press);
        delay(40);
        sendReport(ctrlHeld);
        delay(40);
    }

    sendReport(NO_KEY_PRESSED);
    delay(50);
}