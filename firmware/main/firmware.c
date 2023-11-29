#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

//Biblioteca de conexao com o Wifi
#include "include/wifi_connection.h"

//Biblioteca de criacao do servidor
#include "include/server.h"

//Bibioteca para ADC
#include "driver/adc.h"
#include "esp_adc_cal.h"
//esp_adc/adc_oneshot.h and esp_adc/adc_continuous.h

//Biblioteca para o DHT11
#include "include/dht11.h"

#define LED_PUMP_PIN 2 //LED da ESP32
#define DHT_PIN 4  //CABO AZUL
#define PUMP_PIN 25 //CABO MARROM CLARO
#define LED_PIN 26 //CABO MARROM ESCURO
#define LDR_PIN 34 //CABO LARANJA // ADC1_CHANNEL_6
#define HYGRO_PIN 35 //CABO ROXO // ADC1_CHANNEL_7

static const char *TAGwifi = "wifi_task";
static const char *TAGr = "reading_task";
static const char *TAGs = "server_task";

int ldr_value = 0;
int hygro_value = 0;
int temperature = 0;


void getReadingsTask(void * parameters){
    //Var Init
    int reading_delay = 500;
    int status = -1;

    //DHT Setup
    DHT11_init(DHT_PIN);

    //ADC Setup
    esp_adc_cal_characteristics_t adc1_chars;
    adc1_config_width(ADC_WIDTH_BIT_12); //WIDHT
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12, 0, &adc1_chars); //CALIBRACAO

    adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_6); //ATENUACAO LDR
    adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_0); //ATENUACAO HYGRO

    //Laço
    for( ;; ){
        ESP_LOGI(TAGr,"Get sensors readings");

        //LDR
        ldr_value = adc1_get_raw(ADC1_CHANNEL_6);
        //voltage = esp_adc_cal_raw_to_voltage(reading, &adc1_chars);
        ESP_LOGI(TAGr, "LDR: %d", ldr_value);
        //ESP_LOGI(TAGr, "LDR_ADC: %lu mV", voltage);

        //HYGRO
        hygro_value = adc1_get_raw(ADC1_CHANNEL_7);
        ESP_LOGI(TAGr, "HYGRO: %d", hygro_value);

        //DHT11
        temperature = DHT11_read().temperature;
        status = DHT11_read().status;
        ESP_LOGI(TAGr,"Status code is %d", status);
        ESP_LOGI(TAGr,"Temperature is %d", temperature);
        //ESP_LOGI(TAGr,"Humidity is %d", DHT11_read().humidity);
        
        //Delay
        ESP_LOGI(TAGr,"Sensors reading finished. Next in: %d\n", reading_delay);
        vTaskDelay(reading_delay);
    }
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAGwifi, "Starting Wifi Login ... ... \n"); //Utilizar menuconfig para configurar rede
    wifi_init_sta();
    ESP_LOGI(TAGwifi, "Wifi is Running! \n");

    // GPIO initialization PUMP + LED INBOARD
    gpio_reset_pin(LED_PUMP_PIN);
    gpio_set_direction(LED_PUMP_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(PUMP_PIN);
    gpio_set_direction(PUMP_PIN, GPIO_MODE_OUTPUT);

    // GPIO initialization LED OFFBOARD
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // GPIO initialization LDR
    gpio_reset_pin(LDR_PIN);
    gpio_set_direction(LDR_PIN, GPIO_MODE_INPUT);

    // GPIO initialization HYGRO
    gpio_reset_pin(HYGRO_PIN);
    gpio_set_direction(HYGRO_PIN, GPIO_MODE_INPUT);

    // GPIO initialization DHT
    gpio_reset_pin(DHT_PIN);
    gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT);

    ESP_LOGI(TAGs, "Starting Web Server ... ...\n");
    setup_server(&temperature, &ldr_value, &hygro_value);
    ESP_LOGI(TAGs, "Web Server is running! \n");

   // xTaskCreate(&serverTask, "serverTask",2048,NULL,5,NULL); //funct_name, task_name, stack_size, task_param, task_priority, task_handle
    xTaskCreate(&getReadingsTask, "getReadingsTask",2048,NULL,5,NULL);
}