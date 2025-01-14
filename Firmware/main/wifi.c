#include "wifi.h"

const static char *TAG = "wifi";

uint8_t handleConnection(httpd_req_t *req, char *message);

int activeSocket = 0;

struct asyncRespArg{
    httpd_handle_t hd;
    int fd;
    char *message;
};

static void wsAsyncSend(void *arg){
    ESP_LOGI(TAG, "arg point: %p", arg);
    struct asyncRespArg *respArg = arg;
    const char *message = respArg->message;
    httpd_handle_t hd = respArg->hd;
    int fd = respArg->fd;

    httpd_ws_frame_t wsPkt;
    memset(&wsPkt, 0, sizeof(httpd_ws_frame_t));
    wsPkt.payload = (uint8_t*)message;
    wsPkt.len = strlen(message);
    wsPkt.type = HTTPD_WS_TYPE_TEXT;
    
    httpd_ws_send_frame_async(hd, fd, &wsPkt);
    free(respArg->message);
    free(respArg);
}

esp_err_t wsSend(int fd, httpd_handle_t handle, char *message){
    struct asyncRespArg *respArg = malloc(sizeof(struct asyncRespArg));
    if(respArg == NULL){
        return ESP_ERR_NO_MEM;
    }

    respArg->hd = handle;
    respArg->message = strdup(message);
    respArg->fd = fd;
    esp_err_t res = httpd_queue_work(handle, wsAsyncSend, respArg);
    
    if(res != ESP_OK){
        free(respArg->message);
        free(respArg);
    }

    return res;
}


esp_err_t wsSendReq(httpd_req_t *req, char *message){
    ESP_LOGI(TAG, "wsSend       req addr: %p", req);
    struct asyncRespArg *respArg = malloc(sizeof(struct asyncRespArg));
    if(respArg == NULL){
        return ESP_ERR_NO_MEM;
    }

    respArg->hd = req->handle;
    respArg->message = strdup(message);
    respArg->fd = httpd_req_to_sockfd(req);
    esp_err_t res = httpd_queue_work(req->handle, wsAsyncSend, respArg);
    
    if(res != ESP_OK){
        free(respArg->message);
        free(respArg);
    }

    return res;
}

static esp_err_t wsHandler(httpd_req_t *req){
    if(req->method == HTTP_GET){
        ESP_LOGI(TAG, "Handshake done, new websocket connection was opened");
        return ESP_OK;
    }

    httpd_ws_frame_t wsPkt;
    uint8_t *buf = NULL;
    
    memset(&wsPkt, 0, sizeof(httpd_ws_frame_t));
    wsPkt.type = HTTPD_WS_TYPE_TEXT;
    
    // receive size
    esp_err_t ret = httpd_ws_recv_frame(req, &wsPkt, 0);
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get fram len with %d", ret);
        return ret;
    }

    ESP_LOGI(TAG, "frame len is %zu", wsPkt.len);
    if(wsPkt.len){
        buf = calloc(1, wsPkt.len + 1);
        if(buf == NULL){
            ESP_LOGE(TAG, "failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }

        wsPkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &wsPkt, wsPkt.len);
        if(ret != ESP_OK){
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "got packet with message: %s", wsPkt.payload);

        int sd = httpd_req_to_sockfd(req);
        handleConnection(req, (char*)wsPkt.payload);
        return ESP_OK;
    }

    ESP_LOGI(TAG, "packet type: %d", wsPkt.type);
    // if(wsPkt.type == HTTPD_WS_TYPE_TEXT){
    //     free(buf);
    // }

    ret = httpd_ws_send_frame(req, &wsPkt);
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    }

    free(buf);
    
    return ESP_OK;   
}

static const httpd_uri_t ws = {
    .uri = "/ws",
    .method = HTTP_GET,
    .handler = wsHandler,
    .user_ctx = NULL,
    .is_websocket = true
};

static httpd_handle_t startWebserver(void){
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if(httpd_start(&server, &config) == ESP_OK){
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &ws);
        return server;
    }

    ESP_LOGI(TAG, "Error while starting server");
    return NULL;
}

static void connectHandler(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventDate){
    httpd_handle_t *server = (httpd_handle_t*)arg;
    if(*server == NULL){
        ESP_LOGI(TAG, "Starting webserver");
        *server = startWebserver();
    }
}

static esp_err_t stopWebserver(httpd_handle_t server){
    return httpd_stop(server);
}

static void disconnectHandler(void *arg, esp_event_base_t eventBase, int32_t eventId, void *eventData){
    httpd_handle_t *server = (httpd_handle_t*) arg;
    if(*server){
        ESP_LOGI(TAG, "Stopping webserver");
        if(stopWebserver(*server) == ESP_OK){
            *server = NULL;
        }
        else{
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

void wifiInit(){
    static httpd_handle_t server = NULL;

    ESP_ERROR_CHECK(nvs_flash_init());
    // initialize tcp/ip stack
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    ESP_ERROR_CHECK(example_connect());

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connectHandler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &disconnectHandler, &server));

    server = startWebserver();
}
