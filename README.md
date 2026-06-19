# TACPAD
A functioning tacpad that declares a waveshare ESP32-S3-Touch-LCD-2.8 as a HID keyboard to send stratagem sequences.

Arduino Setup:
Add URL in Preferences/Additional Boards:
https://espressif.github.io/arduino-esp32/package_esp32_index.json

Board Manager:
Search esp32
Install ESP32 by Espressif Systems 3.3.10

Sketch/Include Library/Load from zip:
https://github.com/CIRCUITSTATE/CSE_Touch
https://github.com/CIRCUITSTATE/CSE_CST328

Modify lv_config.h:
#define LV_USE_FS_STDIO 1
#if LV_USE_FS_STDIO
    #define LV_FS_STDIO_LETTER     'S'        // access SD card files as "S:/..."
    #define LV_FS_STDIO_PATH       "/sdcard"  // must match SD_MMC.begin()'s mount point
    #define LV_FS_STDIO_CACHE_SIZE 1024
#endif
