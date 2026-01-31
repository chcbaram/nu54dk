#include "ap.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ap, LOG_LEVEL_DBG);




void apInit(void)
{  
  for (int i = 0; i < 32; i += 2) 
  {
    lcdClearBuffer(black);
    lcdPrintfResize(0, 40 - i, green, 16, "  -- BARAM --");
    lcdDrawRect(0, 0, LCD_WIDTH, LCD_HEIGHT, white);
    lcdUpdateDraw();
    delay(5);
  }  
  delay(500);
  lcdClear(black);

  systemInit();
}

void apMain(void)
{
  systemMain();
}

