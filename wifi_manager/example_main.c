#include "wifi_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    /* Supply your network SSID and password. */
    wifi_manager_init("MyNetworkSSID", "MyNetworkPassword");
    wifi_manager_start();

    /* Wait until connected. */
    while (!wifi_manager_is_connected()) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    /* WiFi is now connected – proceed with the rest of your application. */
}
