#include <stdint.h>

#include "esp_log.h"
#include "esp_http_server.h"

#include "wifiHttp.h"

static const char *TAG = "WIFI_HTTP";

static httpd_handle_t s_httpServer = NULL;

extern const uint8_t index_html_start[] asm("_binary_wifi_web_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_wifi_web_index_html_end");


static esp_err_t RootHandler(httpd_req_t *req)
{
    const size_t length = index_html_end - index_html_start;

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, length);

    return ESP_OK;
}


void WifiHttp_Start(void)
{
    if (s_httpServer != NULL)
        return;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(
        httpd_start(&s_httpServer, &config)
    );

    httpd_uri_t root = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = RootHandler,
        .user_ctx = NULL
    };

    ESP_ERROR_CHECK(
        httpd_register_uri_handler(s_httpServer, &root)
    );

    ESP_LOGI(TAG, "HTTP server started");
}


void WifiHttp_Stop(void)
{
    if (s_httpServer == NULL)
        return;

    ESP_ERROR_CHECK(
        httpd_stop(s_httpServer)
    );
    s_httpServer = NULL;
    ESP_LOGI(TAG, "HTTP server stopped");
}