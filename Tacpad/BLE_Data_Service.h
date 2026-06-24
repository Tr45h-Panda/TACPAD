#pragma once
#include <Arduino.h>
#include <BLEServer.h>

#define BLE_RX_MAXLEN 64

void BLE_DataService_Init(BLEServer* server);
void BLE_SendData(const char* text);
bool BLE_PollReceived(char* outBuf, size_t outBufLen);   // true if a new message was waiting