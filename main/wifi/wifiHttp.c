#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_http_server.h"

#include "wifiHttp.h"
#include <usb/usbState.h>
#include <usb/usbLibrary.h>

static const char *TAG = "WIFI_HTTP";

static httpd_handle_t s_httpServer = NULL;

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");

static esp_err_t RootHandler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /");
    const size_t length = index_html_end - index_html_start;

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, length);

    return ESP_OK;
}
static esp_err_t StateHandler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /state");

    bool random = UsbState_GetUsbRandom();

    char response[32];

    snprintf(
        response,
        sizeof(response),
        "{\"random\":%s}",
        random ? "true" : "false"
    );

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}
static esp_err_t RandomHandler(httpd_req_t *req)
{
    char query[32];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK)
    {
        httpd_resp_send_err(
            req,
            HTTPD_400_BAD_REQUEST,
            "Missing value"
        );
        return ESP_FAIL;
    }

    char value[8];

    if (httpd_query_key_value(query, "value", value, sizeof(value)) != ESP_OK)
    {
        httpd_resp_send_err(
            req,
            HTTPD_400_BAD_REQUEST,
            "Missing value"
        );

        return ESP_FAIL;
    }

    if (strcmp(value, "1") == 0)
    {
        UsbState_SetUsbRandom(true);
    }
    else if (strcmp(value, "0") == 0)
    {
        UsbState_SetUsbRandom(false);
    }
    else
    {
        httpd_resp_send_err(
            req,
            HTTPD_400_BAD_REQUEST,
            "Invalid value"
        );

        return ESP_FAIL;
    }
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}
static esp_err_t PlaylistsHandler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /playlists");

    const UsbPlaylist *playlists = UsbLibrary_GetPlaylists();
    uint16_t count = UsbLibrary_GetPlaylistCount();

    char response[2048];
    size_t pos = 0;

    int written = snprintf(
        response,
        sizeof(response),
        "{\"playlists\":[{\"index\":0,\"name\":\"All tracks\",\"trackCount\":%u}",
        (unsigned int)UsbLibrary_GetCount()
    );

    if (written < 0 || (size_t)written >= sizeof(response))
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Response too large");

    pos = (size_t)written;

    for (uint16_t i = 0; i < count; i++)
    {
        written = snprintf(
            response + pos,
            sizeof(response) - pos,
            ",{\"index\":%u,\"name\":\"%s\",\"trackCount\":%lu}",
            (unsigned int)(i + 1),
            playlists[i].name,
            (unsigned long)playlists[i].trackCount
        );

        if (written < 0 || (size_t)written >= sizeof(response) - pos)
            return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Response too large");

        pos += (size_t)written;
    }

    written = snprintf(
        response + pos,
        sizeof(response) - pos,
        "],\"current\":%u}",
        (unsigned int)UsbLibrary_GetCurrentPlaylistIndex()
    );

    if (written < 0 || (size_t)written >= sizeof(response) - pos)
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Response too large");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}
static esp_err_t PlaylistHandler(httpd_req_t *req)
{
    char query[32];

    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing index");
        return ESP_FAIL;
    }

    char value[8];

    if (httpd_query_key_value(query, "index", value, sizeof(value)) != ESP_OK)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing index");
        return ESP_FAIL;
    }

    char *end = NULL;
    unsigned long index = strtoul(value, &end, 10);

    if (end == value || *end != '\0' || index > UINT16_MAX)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid index");
        return ESP_FAIL;
    }

    if (index == 0)
    {
        UsbLibrary_SetAllTracks();
    }
    else
    {
        if (!UsbLibrary_SetCurrentPlayList((uint8_t)(index - 1)))
        {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Playlist not found");
            return ESP_FAIL;
        }
    }

    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}
void WifiHttp_Start(void)
{
    if (s_httpServer != NULL)
        return;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.task_priority = 7;
    config.stack_size = 6144;

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

    httpd_uri_t state = {
        .uri = "/state",
        .method = HTTP_GET,
        .handler = StateHandler,
        .user_ctx = NULL
    };
    ESP_ERROR_CHECK(httpd_register_uri_handler(s_httpServer, &state));

    httpd_uri_t random = {
        .uri = "/random",
        .method = HTTP_GET,
        .handler = RandomHandler,
        .user_ctx = NULL
    };
    ESP_ERROR_CHECK(httpd_register_uri_handler(s_httpServer, &random));

    httpd_uri_t playlists = {
    .uri = "/playlists",
    .method = HTTP_GET,
    .handler = PlaylistsHandler,
    .user_ctx = NULL
    };

    ESP_ERROR_CHECK(
        httpd_register_uri_handler(s_httpServer, &playlists)
    );

    httpd_uri_t playlist = {
        .uri = "/playlist",
        .method = HTTP_GET,
        .handler = PlaylistHandler,
        .user_ctx = NULL
    };

    ESP_ERROR_CHECK(
        httpd_register_uri_handler(s_httpServer, &playlist)
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