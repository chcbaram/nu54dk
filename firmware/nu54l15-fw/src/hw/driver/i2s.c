#include "i2s.h"



#ifdef _USE_HW_I2S
#include "cli.h"
#include <zephyr/drivers/i2s.h>


#define I2S_MAX_CH      1

#define I2S_SAMPLERATE_HZ       16000
#define I2S_BUF_MS              (4)
#define I2S_BUF_CNT             16
#define I2S_BUF_FRAME_LEN       ((48000 * 2 * I2S_BUF_MS) / 1000)  // 48Khz, Stereo, 4ms



typedef struct
{
  const struct device *h_i2s;  
} i2s_hw_t;

typedef struct
{
  bool is_open;  
  struct i2s_config i2s_cfg;
  i2s_hw_t *p_hw;
} i2s_t;


static void i2sThread(void *arg1, void *arg2, void *arg3);

static bool is_init = false;

K_THREAD_STACK_DEFINE(i2s_stack_area, _HW_DEF_RTOS_THREAD_MEM_I2S);
static struct k_thread i2s_thread_data;
static i2s_t i2s_tbl[I2S_MAX_CH];  

K_MEM_SLAB_DEFINE(tx_mem_slab, I2S_BUF_FRAME_LEN * 2, I2S_BUF_CNT, 32);

static i2s_hw_t i2s_hw_tbl[I2S_MAX_CH] = 
  {
    {DEVICE_DT_GET(DT_NODELABEL(i2s20))},    
  };






bool i2sInit(void)
{
  bool ret = true;


  for (int i=0; i<I2S_MAX_CH; i++)
  {
    i2s_tbl[i].is_open = false;
    i2s_tbl[i].p_hw    = (i2s_hw_t *)&i2s_hw_tbl[i];

    if (!device_is_ready(i2s_tbl[i].p_hw->h_i2s))
    {
      logPrintf("[E_] I2S device not ready\n");
      ret = false;
    }
  }

  if (ret)
  {
    int i2s_ret;

    /* Configure I2S stream */
    i2s_tbl[0].i2s_cfg.word_size      = 16U;
    i2s_tbl[0].i2s_cfg.channels       = 2U;
    i2s_tbl[0].i2s_cfg.format         = I2S_FMT_DATA_FORMAT_I2S;
    i2s_tbl[0].i2s_cfg.frame_clk_freq = 44100;
    i2s_tbl[0].i2s_cfg.block_size     = I2S_BUF_FRAME_LEN * 2;    
    i2s_tbl[0].i2s_cfg.timeout        = 1000;

    /* Configure the Transmit port as Master */
    i2s_tbl[0].i2s_cfg.options  = I2S_OPT_FRAME_CLK_MASTER | I2S_OPT_BIT_CLK_MASTER;

    i2s_tbl[0].i2s_cfg.mem_slab = &tx_mem_slab;
    i2s_ret                     = i2s_configure(i2s_tbl[0].p_hw->h_i2s, I2S_DIR_TX, &i2s_tbl[0].i2s_cfg);
    if (i2s_ret < 0)
    {
      logPrintf("[E_] Failed to configure I2S stream\n");
      ret = false;
    }
  }

  int i2s_ret;

  i2s_ret = i2s_trigger(i2s_tbl[0].p_hw->h_i2s, I2S_DIR_TX, I2S_TRIGGER_START);
  if (i2s_ret < 0)
  {
    logPrintf("Could not trigger I2S tx\n");    
  }

  int16_t data = 0;
  for (int i=0; i<32; i++)
  {
    int16_t buf[32];

    for (int j=0; j<32; j++)
    {
      buf[j] = (data += 10) % 1000;
    }

    i2s_ret = i2s_buf_write(i2s_tbl[0].p_hw->h_i2s, buf, sizeof(buf));
    logPrintf("tx %d, %d\n", i, i2s_ret);
  }

  k_thread_create(&i2s_thread_data, i2s_stack_area,
                  K_THREAD_STACK_SIZEOF(i2s_stack_area),
                  i2sThread,
                  NULL, NULL, NULL,
                  _HW_DEF_RTOS_THREAD_PRI_I2S, 0, K_NO_WAIT);

  is_init = ret;
  
  logPrintf("[%s] i2sInit()\n", ret ? "OK" : "E_");  

  return ret;
}

void i2sThread(void *arg1, void *arg2, void *arg3)
{
  logPrintf("[  ] i2sThread()\n");

  while (1)
  {
    delay(100);
  }
}


#endif