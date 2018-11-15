#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
//#include "bt.h"				//use this for esp-idf versions below 3.0  else use esp_bt.h
#include "esp_bt.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "driver/sdmmc_host.h"

#include "esp_log.h"
#include "sensything_ble.h"
#include "sensything_ads1220.h"
#include "ws2812.h"
#include "sensything_adc.h"
#include "sensything_qwiic.h"
#include "sensything_bmp180.h"

#define TAG "Sensything:"
#define SENSYTHING_MDNS_ENABLED    FALSE
#define SENSYTHING_MDNS_NAME       "heartypatch"
#define BUF_SIZE  1000

extern xSemaphoreHandle print_mux;
char uart_data[50];
const int uart_num = UART_NUM_1;
uint8_t* db;

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT_SENSYTHING = BIT0;

uint8_t* db;
unsigned int global_heartRate ; 


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT_SENSYTHING);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT_SENSYTHING);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void sensything_wifi_init(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
          .sta = {
              .ssid = CONFIG_WIFI_SSID,
              .password = CONFIG_WIFI_PASSWORD,
          },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

void app_main(void)
{

    // Initialize NVS.
  esp_err_t ret;
  ret = nvs_flash_init();
  ESP_ERROR_CHECK( ret );

  sensything_ads1220start();
	vTaskDelay(2000/ portTICK_PERIOD_MS);		//give sometime for max to settle

#ifdef CONFIG_RGB_LED_ENABLE
  ws2812_init(WS2812_PIN);
  vTaskDelay(200/ portTICK_PERIOD_MS);
#endif

  sensything_adc_start();
  vTaskDelay(200/ portTICK_PERIOD_MS);

  init_qwiic();
  vTaskDelay(200/ portTICK_PERIOD_MS);

#ifdef CONFIG_BLE_MODE_ENABLE
  sensything_ble_init();		
#endif

#ifdef CONFIG_WIFIMODE_ENABLE					//configure the ssid/password under makemenuconfig/heartypatch configuration. 
  sensything_wifi_init();
  xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT_SENSYTHING,false, true, portMAX_DELAY);
  /*as of now, functionalities based on wifi is not added*/
#endif


}
