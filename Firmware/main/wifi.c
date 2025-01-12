#include "wifi.h"

const static char *TAG = "wifi";

extern uint8_t handleConnection(const int sock);

int activeSocket = 0;

uint8_t socketReceive(const int sock, char *buffer, const int bufferSize){
    int len;
    buffer[0] = 0;

    len = recv(sock, buffer, bufferSize - 1, 0);
    if(len < 0){
        ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        return 1;
    }
    else if (len == 0) {
        ESP_LOGW(TAG, "Connection closed");
        return 1;
    }

    ESP_LOGI(TAG, "received: >%s<", buffer);

    return 0;
}

uint8_t socketTransmit(const int sock, char *data){
    int size = strlen(data);
    ESP_LOGI(TAG, "Sending %d bytes: %s", size, data);

    // send() can return less bytes than supplied length.
    // Walk-around for robust implementation.
    int toWrite = size;
    while(toWrite > 0){
        int written = send(sock, data + (size - toWrite), toWrite, 0);
        if(written < 0){
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            // failed to retransmit, giving up
            return 1;
        }

        toWrite -= written;
    }

    return 0;
}

void tcpServerTask(void *pvParameters){
    char addrStr[128];
    int addrFamily = (int)pvParameters;
    int ipProtocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage destAddr;

    if(addrFamily == AF_INET){
        struct sockaddr_in *destAddrIPv4 = (struct sockaddr_in*)&destAddr;
        destAddrIPv4->sin_addr.s_addr = htonl(INADDR_ANY);
        destAddrIPv4->sin_family = AF_INET;
        destAddrIPv4->sin_port = htons(PORT);
        ipProtocol = IPPROTO_IP;
    }

    int listenSock = socket(addrFamily, SOCK_STREAM, ipProtocol);
    if(listenSock < 0){
        fprintf(stderr, "Unable to create socket: errno %d\n", errno);
        vTaskDelete(NULL);
        return;
    }
    
    int opt = 1;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    printf("socket created\n");

    int err = bind(listenSock, (struct sockaddr *)&destAddr, sizeof(destAddr));
    if(err != 0){
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addrFamily);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listenSock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while(1){
        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage sourceAddr; // Large enough for both IPv4 or IPv6
        socklen_t addrLen = sizeof(sourceAddr);
        int sock = accept(listenSock, (struct sockaddr *)&sourceAddr, &addrLen);
        if(sock < 0){
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (sourceAddr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr, addrStr, sizeof(addrStr) - 1);
        }
        
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addrStr);

        handleConnection(sock);
        // do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
        ESP_LOGI(TAG, "Socket closed");
    }

CLEAN_UP:
    close(listenSock);
    vTaskDelete(NULL);
}

void wifiInit(){
    ESP_ERROR_CHECK(nvs_flash_init());
    // initialize tcp/ip stack
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect());
    xTaskCreate(tcpServerTask, "tcpServer", 4096, (void*)AF_INET, 5, NULL);
}
