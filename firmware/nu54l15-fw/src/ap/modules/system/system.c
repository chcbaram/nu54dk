#include "system.h"



#define lock(x)      k_mutex_lock(&x, K_FOREVER);
#define unLock(x)    k_mutex_unlock(&x);


static K_MUTEX_DEFINE(mutex_ready);


void updateSD(void);



bool systemInit(void)
{  
  return true;
}

bool systemIsReady(void)
{
  lock(mutex_ready);
  unLock(mutex_ready);
  return true;
}

void systemMain(void)
{
  bool init_ret = true;
  uint32_t pre_time;

  lock(mutex_ready);

  init_ret &= moduleInit();

  logBoot(false);
  logPrintf("[%s] Thread Started : System\n", init_ret ? "OK":"E_" );
  unLock(mutex_ready);

  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 500)
    {
      pre_time = millis();
      ledToggle(_DEF_LED1);
    }
    
    updateSD();

    delay(5);
  }
}

void updateSD(void)
{
  sd_state_t sd_state;


  sd_state = sdUpdate();
  if (sd_state == SDCARD_CONNECTED)
  {
    logPrintf("\nSDCARD_CONNECTED\n");
  }
  if (sd_state == SDCARD_DISCONNECTED)
  {
    logPrintf("\nSDCARD_DISCONNECTED\n");
  }
}