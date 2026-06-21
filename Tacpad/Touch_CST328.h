// Touch_CST328.h
#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <CSE_CST328.h>

#define TP_SDA_PIN   1
#define TP_SCL_PIN   3
#define TP_RST_PIN   2
#define TP_INT_PIN   4

void Touch_Init(void);
bool Touch_Get_XY(uint16_t *x, uint16_t *y);