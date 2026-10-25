#include <stdio.h>
#include <math.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_crt_bundle.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "driver/gpio.h"
#include "compressor.h"
#include "mqttwifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_continuous.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "soc/soc_caps.h"

// State of the state machine.
static state_t state = STATE_WAIT;
// Global error variable, 0 indicates no error.
static uint16_t error = 0;

static bool v_carrier = false;
static bool c_carrier = false;
static bool p_carrier = false;

static void gpio_init(void) {

    gpio_set_level(RELAY1, 0);
    gpio_set_level(RELAY2, 0);
    gpio_config_t io_conf = {
        .pin_bit_mask =
            (1ULL << S_LED)  |
            (1ULL << V_LED)  |
            (1ULL << C_LED)  |
            (1ULL << P_LED)  |
            (1ULL << R_LED)  |
            (1ULL << RELAY1) |
            (1ULL << RELAY2),

        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_reset_pin(BUTTON);
    gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON, GPIO_PULLUP_ONLY);

    // Estado inicial: todos apagados
    gpio_set_level(S_LED, 0);
    gpio_set_level(V_LED, 0);
    gpio_set_level(C_LED, 0);
    gpio_set_level(P_LED, 0);
    gpio_set_level(R_LED, 0);
}

// Prints the error messages. Debug only.
static void print_error(void) {
    printf("error = %04x\n", error);
    if (error & O_V_MASK) printf("Over voltage detected.\n");
    if (error & U_V_MASK) printf("Under voltage detected.\n");
    if (error & V_C_MASK) printf("No voltage carrier signal.\n");
    if (error & O_C_MASK) printf("Over current detected.\n");
    if (error & U_C_MASK) printf("Under current detected.\n");
    if (error & C_C_MASK) printf("No current carrier signal.\n");
    if (error & O_P_MASK) printf("Over pressure detected.\n");
    if (error & U_P_MASK) printf("Under pressure detected.\n");
    if (error & P_C_MASK) printf("No pressure carrier signal.\n");
    if (error & RELE1_MASK) printf("Rele 1 error.\n");
    if (error & RELE2_MASK) printf("Rele 2 error.\n");
    if (error & TIMEOUT_MASK) printf("Timeout error.\n");
    if (error & UNISTABLE_MASK) printf("Unistabel voltage error.\n");
    if (error & WATCHDOG_MASK) printf("Watchdog detected.\n");
}

static void led_task(void *arg) {
    static uint64_t count = 0;
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(250);
    static uint8_t s_led_blink = 0; // State LED
    static uint8_t v_led_blink = 0; // Voltage LED
    static uint8_t c_led_blink = 0; // Current LED
    static uint8_t p_led_blink = 0; // Pressure LED
    static uint8_t r_led_blink = 0; // Relay LED

    while(1) {
        vTaskDelayUntil(&last_wake_time, period);
        count = (count + 1) % 8;
        print_error();
        if (state == STATE_WAIT) {
            s_led_blink = 0b00000001;
        } else if (state == STATE_ON) {
            s_led_blink = 0b11111111;
        } else if ((state == STATE_ERROR) && (error == TIMEOUT_MASK)) {
            s_led_blink = 0b00000101;
        } else if ((state == STATE_ERROR) && (error == UNISTABLE_MASK)) {
            s_led_blink = 0b00010101;
        } else if (state == STATE_ERROR) {
            s_led_blink = 0b01010101;
        }

        if (v_carrier && (!(error & U_V_MASK)) && (!(error & O_V_MASK))) {
            v_led_blink = 0b11111111;
        } else if ((!v_carrier) && (!(error & U_V_MASK)) && (!(error & O_V_MASK))) {
            v_led_blink = 0b00000000;
        } else if ((error & U_V_MASK) && (error & O_V_MASK)) {
            v_led_blink = 0b00010101;
        } else if (error & O_V_MASK) {
            v_led_blink = 0b00000101;
        } else if (error & U_V_MASK) {
            v_led_blink = 0b00000001;
        } 

        if (c_carrier && (!(error & U_C_MASK)) && (!(error & O_C_MASK))) {
            c_led_blink = 0b11111111;
        } else if ((!c_carrier) && (!(error & U_C_MASK)) && (!(error & O_C_MASK))) {
            c_led_blink = 0b00000000;
        } else if ((error & U_C_MASK) && (error & O_C_MASK)) {
            c_led_blink = 0b00010101;
        } else if (error & O_C_MASK) {
            c_led_blink = 0b00000101;
        } else if (error & U_C_MASK) {
            c_led_blink = 0b00000001;
        } 

        if (p_carrier && (!(error & U_P_MASK)) && (!(error & O_P_MASK))) {
            p_led_blink = 0b11111111;
        } else if ((!p_carrier) && (!(error & U_P_MASK)) && (!(error & O_P_MASK))) {
            p_led_blink = 0b00000000;
        } else if ((error & U_P_MASK) && (error & O_P_MASK)) {
            p_led_blink = 0b00010101;
        } else if (error & O_P_MASK) {
            p_led_blink = 0b00000101;
        } else if (error & U_P_MASK) {
            p_led_blink = 0b00000001;
        } 

        if ((!(error & RELE1_MASK)) && (!(error & RELE2_MASK))) {
            r_led_blink = 0b11111111;
        } else if ((error & RELE1_MASK) && (error & RELE2_MASK)) {
            r_led_blink = 0b00010101;
        } else if (error & RELE2_MASK) {
            r_led_blink = 0b00000101;
        } else if (error & RELE1_MASK) {
            r_led_blink = 0b00000001;
        } 

        gpio_set_level(S_LED, ((s_led_blink >> count) & 0x01));
        gpio_set_level(V_LED, ((v_led_blink >> count) & 0x01));
        gpio_set_level(C_LED, ((c_led_blink >> count) & 0x01)); 
        gpio_set_level(P_LED, ((p_led_blink >> count) & 0x01));
        gpio_set_level(R_LED, ((r_led_blink >> count) & 0x01));
    }
}

static void state_monitor(void *arg) {
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(500);
    static bool c_led_state = false;
    
    while(1) {
        switch (state) {
        //#################################################################            
        case STATE_WAIT:
            break;
        //#################################################################
        case STATE_RELE_1_ON:
            break;
        //#################################################################
        case STATE_RELE_2_ON:
            break;
        //#################################################################
        case STATE_ON:
            break;
        //#################################################################
        case STATE_RELE_2_OFF:
            break;
        //#################################################################
        case STATE_RELE_1_OFF:
            break;
        //#################################################################
        case STATE_ERROR:
            break;
        //#################################################################
        default:
            break;
        }
        c_led_state = !c_led_state;
        //gpio_set_level(C_LED, c_led_state);
        
        vTaskDelayUntil(&last_wake_time, period);
    }
}

void app_main(void) {
    static bool v_led_state = false;
    gpio_init();
    xTaskCreate(
        state_monitor,
        "state_monitor",
        4096,
        NULL,
        5,
        NULL
    );
    xTaskCreate(
        led_task,
        "led_task",
        4096,
        NULL,
        3,
        NULL
    );    
    state = STATE_ERROR;
    error = 0;
    while (1) {
        v_led_state = !v_led_state;
        //gpio_set_level(V_LED, v_led_state);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}
