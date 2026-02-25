/* --------------------------------------------------------------
 * main.c – Demo that uses the sdcard component, two buttons,
 * and an optional UART1 link.
 * ----------------------------------------------------------- */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "sdkconfig.h"

#include "sdcard.h"          // <-- our component
#include "driver/gpio.h"
#include "driver/uart.h"

static const char *TAG = "APP";

/* --------------------------------------------------------------
 * Button definitions (avoid strapping pins)
 * ----------------------------------------------------------- */
#define BUTTON1_PIN GPIO_NUM_14   // <-- CHANGE 7
#define BUTTON2_PIN GPIO_NUM_13   // <-- CHANGE 7

static void init_buttons(void)
{
    gpio_config_t cfg = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON1_PIN) | (1ULL << BUTTON2_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));
}

/* --------------------------------------------------------------
 * Optional UART1 link (C5 ↔ C3). Comment out if you don’t need it.
 * ----------------------------------------------------------- */
#define UART_LINK_NUM   UART_NUM_1
#define UART_LINK_TX    GPIO_NUM_4   // C5 TX → C3 RX
#define UART_LINK_RX    GPIO_NUM_5   // C5 RX → C3 TX

static void init_uart_link(void)
{
    const uart_config_t cfg = {
        .baud_rate  = 115200,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    ESP_ERROR_CHECK(uart_driver_install(UART_LINK_NUM, 1024, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_LINK_NUM, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(UART_LINK_NUM,
                                 UART_LINK_TX, UART_LINK_RX,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

/* --------------------------------------------------------------
 * Demo task – proves everything works together
 * ----------------------------------------------------------- */
static void demo_task(void *arg)
{
    (void) arg;
    int counter = 0;

    while (1) {
        if (gpio_get_level(BUTTON1_PIN) == 0) {
            ESP_LOGI(TAG, "Button 1 pressed");
        }
        if (gpio_get_level(BUTTON2_PIN) == 0) {
            ESP_LOGI(TAG, "Button 2 pressed");
        }

        if (counter % 5 == 0) {
            FILE *f = fopen("/sdcard/hello.txt", "a");
            if (f) {
                fprintf(f, "Hello %d\n", counter);
                fclose(f);
                ESP_LOGI(TAG, "Appended to /sdcard/hello.txt (cnt=%d)", counter);
            } else {
                ESP_LOGE(TAG, "Failed to open /sdcard/hello.txt");
            }
        }

        counter++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* --------------------------------------------------------------
 * app_main – runs once at boot
 * ----------------------------------------------------------- */
void app_main(void)
{
    ESP_LOGI(TAG, "=== ESP32‑C5 / ESP32‑C3 Multi‑Board Demo ===");
    ESP_LOGI(TAG, "CPU frequency = %d MHz", esp_clk_cpu_freq() / 1000000);

    init_buttons();                // configure the two buttons
    ESP_ERROR_CHECK(sdcard_init());   // mount /sdcard
    init_uart_link();              // optional UART bridge

    xTaskCreate(&demo_task, "demo_task", 4096, NULL, 5, NULL);
}
