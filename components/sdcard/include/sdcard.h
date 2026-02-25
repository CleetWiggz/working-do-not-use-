#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Initialise the SD‑card (SPI mode) and mount it at /sdcard. */
esp_err_t sdcard_init(void);

#ifdef __cplusplus
}
#endif
