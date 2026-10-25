idf_component_register(SRCS "esp32.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_adc
                            esp_driver_gpio
                            esp_timer
                            esp_wifi 
                            esp-tls
                            nvs_flash 
                            esp_event 
                            esp_netif 
                            driver)