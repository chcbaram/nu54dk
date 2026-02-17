if (CONFIG_SOC_NRF54L05_CPUAPP)
  board_runner_args(jlink "--device=nRF54L05_M33" "--speed=4000")
elseif (CONFIG_SOC_NRF54L05_CPUFLPR)
  board_runner_args(jlink "--device=nRF54L05_RV32" "--speed=4000")
endif()

if(CONFIG_BOARD_NU54L05_NRF54L05_NS)
  set(TFM_PUBLIC_KEY_FORMAT "full")
endif()

if(CONFIG_TFM_FLASH_MERGED_BINARY)
  set_property(TARGET runners_yaml_props_target PROPERTY hex_file "${CMAKE_BINARY_DIR}/tfm_merged.hex")
endif()

include(${ZEPHYR_BASE}/boards/common/nrfutil.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)