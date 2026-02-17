#include "i2s.h"



#ifdef _USE_HW_I2S
#include "cli.h"
#include <zephyr/drivers/i2s.h>
#include <zephyr/sys/iterable_sections.h>


#define I2S_MAX_CH      1

#define I2S_SAMPLERATE_HZ       16000
#define I2S_BUF_MS              (4)
#define I2S_BUF_CNT             16
#define I2S_BUF_FRAME_LEN       ((48000 * 2 * I2S_BUF_MS) / 1000)  // 48Khz, Stereo, 4ms


#define SAMPLE_NO 64

static int16_t data[SAMPLE_NO] = {
	  3211,   6392,   9511,  12539,  15446,  18204,  20787,  23169,
	 25329,  27244,  28897,  30272,  31356,  32137,  32609,  32767,
	 32609,  32137,  31356,  30272,  28897,  27244,  25329,  23169,
	 20787,  18204,  15446,  12539,   9511,   6392,   3211,      0,
	 -3212,  -6393,  -9512, -12540, -15447, -18205, -20788, -23170,
	-25330, -27245, -28898, -30273, -31357, -32138, -32610, -32767,
	-32610, -32138, -31357, -30273, -28898, -27245, -25330, -23170,
	-20788, -18205, -15447, -12540,  -9512,  -6393,  -3212,     -1,
};

static void fill_buf(int16_t *tx_block, int att)
{
	int r_idx;

	for (int i = 0; i < SAMPLE_NO; i++) {
		/* Left channel is sine wave */
		tx_block[2 * i] = data[i] / (1 << att);
		/* Right channel is same sine wave, shifted by 90 degrees */
		r_idx = (i + (ARRAY_SIZE(data) / 4)) % ARRAY_SIZE(data);
		tx_block[2 * i + 1] = data[r_idx] / (1 << att);
	}
}

#define NUM_BLOCKS 20
#define BLOCK_SIZE (2 * sizeof(data))

#ifdef CONFIG_NOCACHE_MEMORY
	#define MEM_SLAB_CACHE_ATTR __nocache
  d
#else
	#define MEM_SLAB_CACHE_ATTR
#endif /* CONFIG_NOCACHE_MEMORY */

static char MEM_SLAB_CACHE_ATTR __aligned(WB_UP(32))
	_k_mem_slab_buf_tx_0_mem_slab[(NUM_BLOCKS) * WB_UP(BLOCK_SIZE)];

static STRUCT_SECTION_ITERABLE(k_mem_slab, tx_0_mem_slab) =
	Z_MEM_SLAB_INITIALIZER(tx_0_mem_slab, _k_mem_slab_buf_tx_0_mem_slab,
				WB_UP(BLOCK_SIZE), NUM_BLOCKS);


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

// static char __nocache __aligned(WB_UP(32)) _k_mem_slab_buf_tx_0_mem_slab[(I2S_BUF_CNT) * WB_UP(I2S_BUF_FRAME_LEN)];

// static STRUCT_SECTION_ITERABLE(k_mem_slab, tx_mem_slab) = Z_MEM_SLAB_INITIALIZER(tx_mem_slab, _k_mem_slab_buf_tx_0_mem_slab, WB_UP(I2S_BUF_FRAME_LEN), I2S_BUF_CNT);


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
    i2s_tbl[0].i2s_cfg.block_size     = BLOCK_SIZE;
    i2s_tbl[0].i2s_cfg.timeout        = 1000;

    /* Configure the Transmit port as Master */
    i2s_tbl[0].i2s_cfg.options  = I2S_OPT_FRAME_CLK_MASTER | I2S_OPT_BIT_CLK_MASTER;

    i2s_tbl[0].i2s_cfg.mem_slab = &tx_mem_slab;
    // i2s_tbl[0].i2s_cfg.mem_slab = &tx_0_mem_slab;

    

    i2s_ret                     = i2s_configure(i2s_tbl[0].p_hw->h_i2s, I2S_DIR_TX, &i2s_tbl[0].i2s_cfg);
    if (i2s_ret < 0)
    {
      logPrintf("[E_] Failed to configure I2S stream\n");
      ret = false;
    }
  }


	// void *tx_block[NUM_BLOCKS];	
	// int i2s_ret;
	// uint32_t tx_idx;
    
	// /* Prepare all TX blocks */
  // for (tx_idx = 0; tx_idx < NUM_BLOCKS; tx_idx++)
  // {
  //   i2s_ret = k_mem_slab_alloc(&tx_0_mem_slab, &tx_block[tx_idx],
  //                          K_FOREVER);
  //   if (i2s_ret < 0)
  //   {
  //     logPrintf("Failed to allocate TX block\n");
  //     return ret;
  //   }
  //   fill_buf((uint16_t *)tx_block[tx_idx], tx_idx % 3);
  // }

  // tx_idx = 0;
  // /* Send first block */
  // i2s_ret = i2s_write(i2s_tbl[0].p_hw->h_i2s, tx_block[tx_idx++], BLOCK_SIZE);
  // if (i2s_ret < 0)
  // {
  //   logPrintf("Could not write TX buffer %d\n", tx_idx);
  //   return i2s_ret;
  // }
  // /* Trigger the I2S transmission */
  // i2s_ret = i2s_trigger(i2s_tbl[0].p_hw->h_i2s, I2S_DIR_TX, I2S_TRIGGER_START);
  // if (i2s_ret < 0)
  // {
  //   printf("Could not trigger I2S tx\n");
  //   return i2s_ret;
  // }

  // for (; tx_idx < NUM_BLOCKS;)
  // {
  //   i2s_ret = i2s_write(i2s_tbl[0].p_hw->h_i2s, tx_block[tx_idx++], BLOCK_SIZE);
  //   if (i2s_ret < 0)
  //   {
  //     logPrintf("[E_] Could not write TX buffer %d, %d\n", tx_idx, i2s_ret);
  //     return i2s_ret;
  //   }
  // }
  // /* Drain TX queue */
  // i2s_ret = i2s_trigger(i2s_tbl[0].p_hw->h_i2s, I2S_DIR_TX, I2S_TRIGGER_DRAIN);
  // if (i2s_ret < 0)
  // {
  //   logPrintf("[E_] Could not trigger I2S tx\n");
  //   return i2s_ret;
  // }
  // logPrintf("[  ] All I2S blocks written\n");


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
    // delay(10);
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