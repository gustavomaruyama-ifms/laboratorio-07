/**
 * M�todo para inicializar o driver WiFi
 */
void wifi_init();

/**
 * M�todo para registrar todos os manipuladores de eventos do driver WiFi
 */
void wifi_events_register();

/**
 * Executado quando o driver WiFi for inicializado
 */
void wifi_start_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data_event);

/**
 * Executado quando a conex�o wifi for estabelecida
 */
void wifi_connection_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data_event);
/**
 * Executado quando a um endere�o ip for atribuido ao esp32
 */
void ip_address_bind_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data_event);

/**
 * M�todo que inicializa o cliente MQTT
 */
void mqtt_init(void);

/**
 * M�todo para registrar todos os manipuladores de eventos MQTT
 */
void mqtt_events_register(esp_mqtt_client_handle_t client);

/**
 * Executado quando a conex�o com broker MQTT for estabelecida (CONNACK)
 */
void mqtt_connack_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data_event);
/**
 * Executado quando um publish for confirmado pelo broker (PUBACK)
 */
void mqtt_puback_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data_event);

/**
 * Executado quando um subscribe for confirmado pelo broker (SUBACK)
 */
void mqtt_suback_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data_event);

/**
 * Executado quando um publish for recebido
 */
void mqtt_data_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data_event);

/**
 * Publica simulacao de temperatura
 */
void publicar();

