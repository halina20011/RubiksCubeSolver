#ifndef WIFI
#define WIFI

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"

#include "protocol_examples_common.h"

#include <esp_http_server.h>

#define WS_SEND(req, size, format, ...) do{\
    char data[size] = {};\
    snprintf(data, size, format, __VA_ARGS__);\
    wsSend(req, data);\
}while(0)

static void wsAsyncSend(void *arg);
esp_err_t wsSend(int fd, httpd_handle_t handle, char *message);
esp_err_t wsSendReq(httpd_req_t *req, char *message);

void wifiInit();

#endif
