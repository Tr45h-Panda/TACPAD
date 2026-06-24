/*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html  */

#include "Display_ST7789.h"
#include "Audio_PCM5101.h"
#include "LVGL_Driver.h"
#include "PWR_Key.h"
#include "SD_Card.h"
#include "LVGL_Example.h"
#include "Touch_CST328.h"
#include "BAT_Driver.h"
#include "Wireless.h"
#include "BLE_Driver.h"
#include "Stratagem_Data.h"
#include "I2C_Driver.h"
#include "BLE_Data_Service.h"
#include <esp_system.h>


void DriverTask(void *parameter) {

  while(1){
    PWR_Loop();
    BAT_Get_Volts();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
void Driver_Loop() {
  xTaskCreatePinnedToCore(
    DriverTask,           
    "DriverTask",         
    4096,                 
    NULL,                 
    3,                    
    NULL,                 
    0                     
  );  
}

// calls BLE_Driver.cpp to initialize bluetooth connection
void BLE_Loop() {
  Serial.begin(115200);
  xTaskCreate(
        bluetoothTask,
        "BLE",
        8192,
        nullptr,
        1,
        nullptr
  );
}

void setup()
{
  esp_reset_reason_t reason = esp_reset_reason();
  printf("Reset reason code: %d\r\n", (int)reason);
  
  Flash_test();
  Button_Init();
  PWR_Init();
  BAT_Init();

  I2C_Init();
  Touch_Init();

  BLE_Loop();
  BLE_Queue_Init();
  Backlight_Init();

  SD_Init();
  LCD_Init();
  Lvgl_Init();

  Splash_Show();

  Stratagem_LoadCSV("/Stratagems.csv");
  Stratagem_LoadAllLoadouts("/loadout.txt");
  Stratagem_SelectLoadout(0);
  Audio_Init();

  Lvgl_Example1();

  Splash_Hide();

  BLE_Loop();
  BLE_Queue_Init();
  Driver_Loop();

  
}

void loop()
{
  Lvgl_Loop();

  char loadoutMsg[BLE_RX_MAXLEN];
  if (BLE_PollReceived(loadoutMsg, sizeof(loadoutMsg))) {
    printf("Got loadout from PC: %s\r\n", loadoutMsg);
    Stratagem_ParseLoadoutString(loadoutMsg);
    Automatic_grid_refresh();
  }

  vTaskDelay(pdMS_TO_TICKS(5));
}
