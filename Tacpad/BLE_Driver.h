#ifndef BLE_DRIVER_H
#define BLE_DRIVER_H

#include <Arduino.h>

extern bool isBleConnected;
extern bool sent;

void bluetoothTask(void* parameter);
void sendKeySequence(const char* sequence);
void BLE_Queue_Init(void);
void Queue_KeySequence(const char* sequence);

#endif