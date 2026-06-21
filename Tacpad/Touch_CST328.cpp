// Touch_CST328.cpp
#include "Touch_CST328.h"

// Width/height here = the panel's native resolution (240x320), not your
// LVGL rotation — used by the controller for coordinate scaling only.
CSE_CST328 tsPanel = CSE_CST328(240, 320, &Wire1, TP_RST_PIN, TP_INT_PIN);

void Touch_Init(void) {
  Wire1.begin(TP_SDA_PIN, TP_SCL_PIN);   // second I2C bus, separate from QMI8658/RTC

  if (!tsPanel.begin()) {
    printf("CST328 : touch controller not found\r\n");
    return;
  }
  tsPanel.setRotation(1);   // try 0/1/2/3 if X/Y comes out swapped or mirrored
  printf("CST328 : touch controller initialized\r\n");
}

bool Touch_Get_XY(uint16_t *x, uint16_t *y) {
  if (tsPanel.isTouched(0)) {
    CSE_TouchPoint p = tsPanel.getPoint(0);
    *x = p.x;
    *y = p.y;
    return true;
  }
  return false;
}