#include "sdcard.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat_sdspi.h"
#include <stdio.h>

#define SD_CARD_CS_PIN    GPIO_NUM_23   // <-- CHANGE 2
#define SD_CARD_MOSI_PIN  GPIO_NUM_24   // <-- CHANGE 3
#define SD_CARD_MISO_PIN  GPIO_NUM_4    // <-- CHANGE 4
#define SD_CARD_SCK_PIN   GPIO_NUM_5    // <-- CHANGE 5

static sdmmc_card_t *card;
static const char mount_point[] = "/sdcard";

esp_err_t sdcard_init(void)
{
    esp_vfs_fat_sdmmc_config_t mount_cfg = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();      // uses SPI2 (GP‑SPI)
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_CARD_MOSI_PIN,
        .miso_io_num = SD_CARD_MISO_PIN,
        .sclk_io_num = SD_CARD_SCK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA));

    sdspi_device_config_t slot_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_cfg.gpio_cs = SD_CARD_CS_PIN;
    slot_cfg.host_id = host.slot;

    esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_point, &host,
                                           &slot_cfg, &mount_cfg, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            printf("Failed to mount filesystem.\n");
        } else {
            printf("SD init error: %s\n", esp_err_to_name(ret));
        }
        return ret;
    }

    printf("SD Card mounted at %s\n", mount_point);
    return ESP_OK;
}
