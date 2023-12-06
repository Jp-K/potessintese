#include "include/server.h"

#define LED_PUMP_PIN 2 //LED da ESP
#define DHT_PIN 4  //CABO AZUL
#define PUMP_PIN 25 //CABO AMARELO
#define LED_PIN 26 //CABO MARROM ESCURO
#define LDR_PIN 34 //CABO LARANJA // ADC1_CHANNEL_6
#define HYGRO_PIN 35 //CABO ROXO // ADC1_CHANNEL_7


//static const char *TAG = "server"; // TAG for debug
int pump_state = 0;
int led_state = 0;
int *temp = 0;  //endereço da memoria passado para temperatura
int *lum = 0;   //endereço da memoria passado para luminosidade
int *hygro = 0; //endereço da memoria passado para hygrometro/umidade


esp_err_t send_web_page(httpd_req_t *req)
{
    int response;
    char default_resp[100] = "";
    char resp_string_temp[10];
    char resp_string_lum[10];
    char resp_string_led[10];
    char resp_string_pump[10];
    char resp_string_hygro[10];

    sprintf(resp_string_temp, "%d", (int)*temp);   //temperatura para string
    sprintf(resp_string_lum, "%d", *lum);          //luminosidade para string
    sprintf(resp_string_hygro, "%d", *hygro);   //estado da pump para string
    sprintf(resp_string_led, "%d", led_state);     //estado do led para string
    sprintf(resp_string_pump, "%d", pump_state);   //estado da pump para string
    
    //temperatura,luminosidade,umidade,estado_pump,estado_ledboard
    strcat(default_resp, resp_string_temp);
    strcat(default_resp, ",");
    strcat(default_resp, resp_string_lum);
    strcat(default_resp, ",");
    strcat(default_resp, resp_string_hygro);
    strcat(default_resp, ",");
    strcat(default_resp, resp_string_pump);
    strcat(default_resp, ",");
    strcat(default_resp, resp_string_led);
    
    response = httpd_resp_send(req,default_resp,HTTPD_RESP_USE_STRLEN);
    return response;
}

esp_err_t get_req_handler(httpd_req_t *req)
{
    return send_web_page(req);
}

esp_err_t pump_on_handler(httpd_req_t *req)
{
    gpio_set_level(LED_PUMP_PIN, 1);
    gpio_set_level(PUMP_PIN, 1);
    pump_state = 1;
    return send_web_page(req);
}

esp_err_t pump_off_handler(httpd_req_t *req)
{
    gpio_set_level(LED_PUMP_PIN, 0);
    gpio_set_level(PUMP_PIN, 0);
    pump_state = 0;
    return send_web_page(req);
}

esp_err_t led_on_handler(httpd_req_t *req)
{
    gpio_set_level(LED_PIN, 1);
    led_state = 1;
    return send_web_page(req);
}

esp_err_t led_off_handler(httpd_req_t *req)
{
    gpio_set_level(LED_PIN, 0);
    led_state = 0;
    return send_web_page(req);
}


httpd_uri_t uri_get = {
    .uri = "/get",
    .method = HTTP_GET,
    .handler = get_req_handler,
    .user_ctx = NULL};

httpd_uri_t uri_led_on = {
    .uri = "/led2on",
    .method = HTTP_GET,
    .handler = led_on_handler,
    .user_ctx = NULL};

httpd_uri_t uri_led_off = {
    .uri = "/led2off",
    .method = HTTP_GET,
    .handler = led_off_handler,
    .user_ctx = NULL};

httpd_uri_t uri_pump_on = {
    .uri = "/pump2on",
    .method = HTTP_GET,
    .handler = pump_on_handler,
    .user_ctx = NULL};

httpd_uri_t uri_pump_off = {
    .uri = "/pump2off",
    .method = HTTP_GET,
    .handler = pump_off_handler,
    .user_ctx = NULL};

httpd_handle_t setup_server(int *temp_ref, int *lum_ref, int *hygro_ref)
{
    //led_pot_state = 0;
    temp = temp_ref;
    lum = lum_ref;
    hygro = hygro_ref;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_pump_on);
        httpd_register_uri_handler(server, &uri_pump_off);
        httpd_register_uri_handler(server, &uri_led_on);
        httpd_register_uri_handler(server, &uri_led_off);
    }
    return server;
}
