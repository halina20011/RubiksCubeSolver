#ifndef WIFI
#define WIFI

#include "esp_netif.h" // include tcp/ip stack
#include "protocol_examples_common.h" // for
#include "esp_log.h"
#include "esp_netif.h"
#include <nvs_flash.h>
#include <esp_event.h>

#include "lwip/sockets.h"
#include <lwip/netdb.h>
#include "lwip/sys.h"

#define KEEPALIVE_IDLE 1
#define KEEPALIVE_INTERVAL 1
#define KEEPALIVE_COUNT 1

#define PORT 6969

#define socketSend(sock, size, format, ...) do{\
    char data[size] = {};\
    snprintf(data, size, format, __VA_ARGS__);\
    socketTransmit(sock, data);\
}while(0)

void socketTransmitAll(char *data);

uint8_t socketReceive(const int sock, char *buffer, const int bufferSize);
uint8_t socketTransmit(int sock, char *data);

void tcpServerTask(void *pvParameters);

void wifiInit();

#endif
