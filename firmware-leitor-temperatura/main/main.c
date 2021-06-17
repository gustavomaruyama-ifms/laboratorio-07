#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "main.h"

#define WIFI_SSID "IFMS-CX-LAB" 					// ssid da rede wifi
#define WIFI_PASSWORD "u32@ifms"					// senha da rede wifi
#define MQTT_BROKER_URI "mqtt://192.168.43.8:1883" 	// endereco do broker mqtt
#define MQTT_CLIENT_ID "temperature_device_01" 		// endereco do broker mqtt

esp_mqtt_client_handle_t mqtt_client;

void app_main(void){
	gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
	esp_event_loop_create_default();
	nvs_flash_init();
	esp_netif_init();
    esp_netif_create_default_wifi_sta();
	wifi_init();
	xTaskCreatePinnedToCore(publicar, "publicar", 4*1024, NULL, 1, NULL, 1);
}

void wifi_init(){
	wifi_events_register();
	// Configurações iniciais do driver WiFi - Estrutura de controle, Buffer RX/TX, Protocolo de Criptografia...
	wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();

	// Configuração de acesso a rede
	wifi_config_t wifi_config = {
			.sta = {
					.ssid = WIFI_SSID,
					.password = WIFI_PASSWORD
			}
	};

	esp_wifi_init(&init_config);
	esp_wifi_set_mode(WIFI_MODE_STA); // Configura como modo station
	esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
	esp_wifi_start();
}

void wifi_events_register(){
	// Eventos da Wifi
	esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START, &wifi_start_event_handler, NULL);
	esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &wifi_connection_event_handler, NULL);
	// Eventos Ip
	esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_address_bind_event_handler, NULL);
}

/**
 * Executado quando o wifi for inicializado
 */
void wifi_start_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data_event){
	printf("Modo Station de WiFi inicializado!\n");
	printf("Conectando com a rede WiFi.\n");
	esp_wifi_connect();
}

/**
 * Executado quando a conexão wifi for estabelecida
 */
void wifi_connection_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data_event){
	printf("Conectado a rede WiFi.\n");
}

/**
 * Executado quando a um endereço ip for atribuido ao esp32
 */
void ip_address_bind_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data_event){
	ip_event_got_ip_t *event = (ip_event_got_ip_t*) data_event;
	printf("Endereço IP obtido: "IPSTR"\n", IP2STR(&event->ip_info.ip));
	mqtt_init();
}

/**
 * Método que inicializa o cliente MQTT
 */
void mqtt_init(void) {
	// Configuração de conexão MQTT
	const esp_mqtt_client_config_t mqtt_config = {
			.uri = MQTT_BROKER_URI,
			.client_id = MQTT_CLIENT_ID
	};

	// Manipulador do cliente MQTT
	mqtt_client = esp_mqtt_client_init(&mqtt_config);

	// Registro de manipulador de eventos MQTT
	mqtt_events_register(mqtt_client);
	esp_mqtt_client_start(mqtt_client);
}

void mqtt_events_register(esp_mqtt_client_handle_t client){
	esp_mqtt_client_register_event(client, MQTT_EVENT_CONNECTED, &mqtt_connack_event_handler, client);
	esp_mqtt_client_register_event(client, MQTT_EVENT_PUBLISHED, &mqtt_puback_event_handler, client);
	esp_mqtt_client_register_event(client, MQTT_EVENT_SUBSCRIBED, &mqtt_suback_event_handler, client);
	esp_mqtt_client_register_event(client, MQTT_EVENT_DATA, &mqtt_data_event_handler, client);
}

/**
 * Executado quando a conexão com broker MQTT for estabelecida (CONNACK)
 */
void mqtt_connack_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data_event){
	printf("CONNACK RECEBIDO:\n");
	printf("Conectado ao broker MQTT.\n");
	esp_mqtt_client_subscribe(mqtt_client, "dispositivo/comando", 0);
}

/**
 * Executado quando um publish for confirmado pelo broker (PUBACK)
 */
void mqtt_puback_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data_event){
	printf("PUBACK RECEBIDO.\n");
}

/**
 * Executado quando um subscribe for confirmado pelo broker (SUBACK)
 */
void mqtt_suback_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data_event){
	esp_mqtt_event_handle_t evento_mqtt = (esp_mqtt_event_handle_t) data_event;
	printf("SUBACK RECEBIDO:\n");
	printf("Inscrito no topico: %.*s.\n", evento_mqtt->topic_len, evento_mqtt->topic);
}

/**
 * Executado quando um publish for recebido
 */
void mqtt_data_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data_event){
	esp_mqtt_event_handle_t evento_mqtt = (esp_mqtt_event_handle_t) data_event;
	printf("PUBLISH RECEBIDO:\n");
	printf("Topico: %.*s.\n", evento_mqtt->topic_len, evento_mqtt->topic);
	printf("Payload: %.*s.\n", evento_mqtt->data_len, evento_mqtt->data);

	char valor[evento_mqtt->data_len];
	sprintf(valor, "%*s", evento_mqtt->data_len, evento_mqtt->data);

	if(valor[0] == '0'){
		gpio_set_level(GPIO_NUM_2, 0);
	}else if(valor[0] == '1'){
		gpio_set_level(GPIO_NUM_2, 1);
	}
}

/**
 * Publica simulacao de temperatura
 */
void publicar(){
	//UBaseType_t uxHighWaterMark;
	//uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	//printf("Task Size = %d\n",uxHighWaterMark);
	float valor = 0.0;
	while(1){
		vTaskDelay(2000 / portTICK_RATE_MS);
		if(!mqtt_client){
			continue;
		}

		valor += 1.1;
		if(valor > 35.0){
			valor = 0.0;
		}

		char valorString[22];
		sprintf(valorString, "{\"temperature\":\"%2.1f\"}", valor);

		esp_mqtt_client_publish(mqtt_client, "sala/temperatura", valorString, 0, 1, 0);	// Realizar o publish
		//uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
		//printf("Task Size = %d\n",uxHighWaterMark);
	}
}


