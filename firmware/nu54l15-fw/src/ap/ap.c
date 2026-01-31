#include "ap.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ap, LOG_LEVEL_DBG);




void apInit(void)
{  
  systemInit();
}

void apMain(void)
{
  systemMain();
}

