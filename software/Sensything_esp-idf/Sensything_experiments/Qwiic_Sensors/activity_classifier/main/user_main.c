
#include <libesphttpd/esp.h>
#include "libesphttpd/httpd.h"
#include "io.h"
#include "libesphttpd/httpdespfs.h"
#include "cgi.h"
#include "libesphttpd/cgiwifi.h"
#include "libesphttpd/cgiflash.h"
#include "libesphttpd/auth.h"
#include "libesphttpd/espfs.h"
#include "libesphttpd/captdns.h"
#include "libesphttpd/webpages-espfs.h"
#include "libesphttpd/cgiwebsocket.h"
#include "libesphttpd/httpd-freertos.h"
#include "libesphttpd/route.h"
#include "cgi-test.h"

#include "esp_wifi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#ifdef ESP32
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"


#endif
#include "sensything_bno080.h"


#define TAG "user_main"

#define LISTEN_PORT     80u
#define MAX_CONNECTIONS 32u

uint8_t data[10];
int i;

static char connectionMemory[sizeof(RtosConnType) * MAX_CONNECTIONS];
static HttpdFreertosInstance httpdFreertosInstance;

//Function that tells the authentication system what users/passwords live on the system.
//This is disabled in the default build; if you want to try it, enable the authBasic line in
//the builtInUrls below.
int myPassFn(HttpdConnData *connData, int no, char *user, int userLen, char *pass, int passLen) {
	if (no==0) {
		strcpy(user, "admin");
		strcpy(pass, "s3cr3t");
		return 1;
	}
	return 0;
}


//Broadcast the uptime in seconds every second over connected websockets
static void websocketBcast(void *arg) {
	static int ctr=33;
	char buff[128];
    
	while(1) {
		
		bno_classifier * imu_data = get_imudata();

		if(imu_data != NULL){
        data[i] = imu_data->activity_values;
        sprintf(buff, "%d", data[i]);
    	}else {}
		cgiWebsockBroadcast(&httpdFreertosInstance.httpdInstance,
		                    "/websocket/ws.cgi", buff, strlen(buff),
		                    WEBSOCK_FLAG_NONE);
	
		vTaskDelay(400/portTICK_RATE_MS);
	}
}

static void myWebsocketRecv(Websock *ws, char *data, int len, int flags) {
	int i;
	char buff[128];
	sprintf(buff, "You sent: ");
	for (i=0; i<len; i++) buff[i+10]=data[i];
	buff[i+10]=0;
	cgiWebsocketSend(&httpdFreertosInstance.httpdInstance,
	                 ws, buff, strlen(buff), WEBSOCK_FLAG_NONE);
}

static void myWebsocketConnect(Websock *ws) {
	ws->recvCb=myWebsocketRecv;
	cgiWebsocketSend(&httpdFreertosInstance.httpdInstance,
	                 ws, "Hi, Websocket!", 14, WEBSOCK_FLAG_NONE);
}

void myEchoWebsocketRecv(Websock *ws, char *data, int len, int flags) {
	cgiWebsocketSend(&httpdFreertosInstance.httpdInstance,
	                 ws, data, len, flags);
}

void myEchoWebsocketConnect(Websock *ws) {
	ws->recvCb=myEchoWebsocketRecv;
}

#define OTA_FLASH_SIZE_K 1024
#define OTA_TAGNAME "generic"

CgiUploadFlashDef uploadParams={
	.type=CGIFLASH_TYPE_FW,
	.fw1Pos=0x1000,
	.fw2Pos=((OTA_FLASH_SIZE_K*1024)/2)+0x1000,
	.fwSize=((OTA_FLASH_SIZE_K*1024)/2)-0x1000,
	.tagName=OTA_TAGNAME
};


HttpdBuiltInUrl builtInUrls[]={
	ROUTE_CGI_ARG("*", cgiRedirectApClientToHostname, "esp8266.nonet"),
	ROUTE_REDIRECT("/", "/index.tpl"),

	ROUTE_TPL("/led.tpl", tplLed),
	ROUTE_TPL("/index.tpl", tplCounter),
	ROUTE_CGI("/led.cgi", cgiLed),

	ROUTE_REDIRECT("/flash", "/flash/index.html"),
	ROUTE_REDIRECT("/flash/", "/flash/index.html"),
	ROUTE_CGI("/flash/flashinfo.json", cgiGetFlashInfo),
	ROUTE_CGI("/flash/setboot", cgiSetBoot),
	ROUTE_CGI_ARG("/flash/upload", cgiUploadFirmware, &uploadParams),
	ROUTE_CGI_ARG("/flash/erase", cgiEraseFlash, &uploadParams),
	ROUTE_CGI("/flash/reboot", cgiRebootFirmware),

	ROUTE_REDIRECT("/wifi", "/wifi/wifi.tpl"),
	ROUTE_REDIRECT("/wifi/", "/wifi/wifi.tpl"),
	ROUTE_CGI("/wifi/wifiscan.cgi", cgiWiFiScan),
	ROUTE_TPL("/wifi/wifi.tpl", tplWlan),
	ROUTE_CGI("/wifi/connect.cgi", cgiWiFiConnect),
	ROUTE_CGI("/wifi/connstatus.cgi", cgiWiFiConnStatus),
	ROUTE_CGI("/wifi/setmode.cgi", cgiWiFiSetMode),
	ROUTE_CGI("/wifi/startwps.cgi", cgiWiFiStartWps),

	ROUTE_REDIRECT("/websocket", "/websocket/index.html"),
	ROUTE_WS("/websocket/ws.cgi", myWebsocketConnect),
	ROUTE_WS("/websocket/echo.cgi", myEchoWebsocketConnect),

	ROUTE_REDIRECT("/httptest", "/httptest/index.html"),
	ROUTE_REDIRECT("/httptest/", "/httptest/index.html"),
	ROUTE_CGI("/httptest/test.cgi", cgiTestbed),

	ROUTE_FILESYSTEM(),

	ROUTE_END()
};


#ifdef ESP32

static EventGroupHandle_t wifi_ap_event_group;
static EventGroupHandle_t wifi_sta_event_group;

const static int CONNECTED_BIT = BIT0;

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        /* Calling this unconditionally would interfere with the WiFi CGI. */
        // esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_sta_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        xEventGroupClearBits(wifi_sta_event_group, CONNECTED_BIT);
                                                                  
        switch(event->event_info.disconnected.reason){
        case WIFI_REASON_ASSOC_LEAVE:
        case WIFI_REASON_AUTH_FAIL:
            break;
        default:
            esp_wifi_connect();
            break;
        }
        break;
    case SYSTEM_EVENT_AP_START:
    {
        tcpip_adapter_ip_info_t ap_ip_info;
        if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ap_ip_info) == 0) {
            ESP_LOGI(TAG, "~~~~~~~~~~~");
            ESP_LOGI(TAG, "IP:" IPSTR, IP2STR(&ap_ip_info.ip));
            ESP_LOGI(TAG, "MASK:" IPSTR, IP2STR(&ap_ip_info.netmask));
            ESP_LOGI(TAG, "GW:" IPSTR, IP2STR(&ap_ip_info.gw));
            ESP_LOGI(TAG, "~~~~~~~~~~~");
        }
    }
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG, "station:" MACSTR" join,AID=%d\n",
                MAC2STR(event->event_info.sta_connected.mac),
                event->event_info.sta_connected.aid);
        xEventGroupSetBits(wifi_ap_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "station:" MACSTR"leave,AID=%d\n",
                MAC2STR(event->event_info.sta_disconnected.mac),
                event->event_info.sta_disconnected.aid);
        xEventGroupClearBits(wifi_ap_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_SCAN_DONE:
        break;
    default:
        break;
    }

    /* Forward event to to the WiFi CGI module */
    cgiWifiEventCb(event);

    return ESP_OK;
}


//Simple task to connect to an access point
void ICACHE_FLASH_ATTR init_wifi(bool modeAP) {
	esp_err_t result;

	result = nvs_flash_init();
	if(   result == ESP_ERR_NVS_NO_FREE_PAGES
	   || result == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_LOGI(TAG, "Erasing NVS");
		nvs_flash_erase();
		result = nvs_flash_init();
	}
	ESP_ERROR_CHECK(result);

	wifi_sta_event_group = xEventGroupCreate();
	wifi_ap_event_group = xEventGroupCreate();

	// Initialise wifi configuration CGI
	result = initCgiWifi();
	ESP_ERROR_CHECK(result);

	ESP_ERROR_CHECK( esp_event_loop_init(wifi_event_handler, NULL) );

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

	//Go to station mode
	esp_wifi_disconnect();

	if(modeAP) {
		ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );

		wifi_config_t ap_config;
		strcpy((char*)(&ap_config.ap.ssid), "esp");
		ap_config.ap.ssid_len = 3;
		ap_config.ap.channel = 1;
		ap_config.ap.authmode = WIFI_AUTH_OPEN;
		ap_config.ap.ssid_hidden = 0;
		ap_config.ap.max_connection = 3;
		ap_config.ap.beacon_interval = 100;

		esp_wifi_set_config(WIFI_IF_AP, &ap_config);
	}
	else {
		esp_wifi_set_mode(WIFI_MODE_STA);

		//Connect to the defined access point.
		wifi_config_t config;
		memset(&config, 0, sizeof(config));
		sprintf((char*)config.sta.ssid, "protocentral");			// @TODO: Changeme
		sprintf((char*)config.sta.password, "open1234"); 	// @TODO: Changeme
		esp_wifi_set_config(WIFI_IF_STA, &config);
		esp_wifi_connect();
	}

	ESP_ERROR_CHECK( esp_wifi_start() );
}
#endif

//Main routine. Initialize stdout, the I/O, filesystem and the webserver and we're done.
#if ESP32
void app_main(void) {
#else
void user_init(void) {
#endif

#ifndef ESP32
	uart_div_modify(0, UART_CLK_FREQ / 115200);
#endif

	start_imu();
    
	ioInit();
	espFsInit((void*)(webpages_espfs_start));

	tcpip_adapter_init();
	httpdFreertosInit(&httpdFreertosInstance,
	                  builtInUrls,
	                  LISTEN_PORT,
	                  connectionMemory,
	                  MAX_CONNECTIONS,
	                  HTTPD_FLAG_NONE);
	httpdFreertosStart(&httpdFreertosInstance);

	init_wifi(true); // Supply false for STA mode

	xTaskCreate(websocketBcast, "wsbcast", 3000, NULL, 3, NULL);
	}