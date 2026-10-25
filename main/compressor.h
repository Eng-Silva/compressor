// Protection levels
#define U_V_LIMIT    200 // [V] Under voltage limit.
#define O_V_LIMIT    230 // [V] Over voltage limit.
#define U_C_LIMIT    3.5 // [A] Under current limit.
#define O_C_LIMIT    5.5 // [A] Over current limit.
#define S_C_LIMIT    6.5 // [A] Start current limit.
#define U_P_LIMIT    5.5 // [kgf/cm²] Under pressure limit.
#define O_P_LIMIT    8.0 // [kgf/cm²] Over pressure limit.
#define ON_TIME_OUT   30000000 // [us] Max time allowed in ON_STATE.
#define MIN_ON_TIME   40000000 // [us] Minimum ON time to consider stable voltage.
#define START_TIME_1  10000000 // [us] Used to relax current limits and minimum
#define START_TIME_2  30000000 // [us] pressure limit during engine start.
#define V_RECOV_TIME  40000000 //180000000 // [us] Time to recover from voltage error.

// Error masks
#define U_V_MASK     0x0001 // Indicates under voltage condition.
#define O_V_MASK     0x0002 // Indicates over voltage condition.
#define V_C_MASK     0x0004 // Indicates no voltage carrier signal.
#define U_C_MASK     0x0008 // Indicates under current condition.
#define O_C_MASK     0x0010 // Indicates under current condition.
#define C_C_MASK     0x0020 // Indicates no current carrier signal.
#define U_P_MASK     0x0040 // Indicates under pressure condition.
#define O_P_MASK     0x0080 // Indicates under pressure condition.
#define P_C_MASK     0x0100 // Indicates no pressure carrier signal.
#define RELE1_MASK   0x0200 // Indicates error at relay 1.
#define RELE2_MASK   0x0400 // Indicates error at relay 2.
#define TIMEOUT_MASK 0x0800 // Indicates timeout occured during ON_STATE.
#define UNISTABLE_MASK 0x1000 // Indicates timeout occured during ON_STATE.
#define WATCHDOG_MASK  0x2000 // ### Indicates timeout of the ADC task? ###.

// Wafe constants
#define AVG_V_READ        1928
#define V_L_THRESHOLD     -100
#define V_H_THRESHOLD      100
#define AVG_I_READ        1914
#define I_L_THRESHOLD     -100
#define I_H_THRESHOLD      100
#define MIN_ZONE_COUNT       5
#define Z_CHANGE_BUFFER     60
#define MIN_P_CARRIER      540
#define MAX_P_ADC_READ    3280
#define P_CHANNEL_WINDOW   333 // Number os samples.
#define CARRIER_TIMEOUT_US   100000  // 100 ms

// Wave conversion constants
#define kv 0.201900
#define ki 0.015422
#define kp 0.003065

// ADC channel configuration
#define V_CHANNEL ADC_CHANNEL_6
#define C_CHANNEL ADC_CHANNEL_7
#define P_CHANNEL ADC_CHANNEL_4
#define SAMPLE_RATE_HZ   60000
#define RAW_BUFFER_SIZE    256

/* GPIOs */
#define BUTTON    GPIO_NUM_21
#define S_LED     GPIO_NUM_25
#define V_LED     GPIO_NUM_26
#define C_LED     GPIO_NUM_27
#define P_LED     GPIO_NUM_4
#define R_LED     GPIO_NUM_23
#define RELAY1    GPIO_NUM_18
#define RELAY2    GPIO_NUM_19

// Compressor states
typedef enum {
    STATE_WAIT,
    STATE_RELE_1_ON,
    STATE_RELE_2_ON,
    STATE_ON,
    STATE_RELE_2_OFF,
    STATE_RELE_1_OFF,
    STATE_ERROR
} state_t;

// Wave zones
typedef enum {
    H_ZONE,
    Z_ZONE,
    L_ZONE
} zone_t;

// 
struct zone_change_t {
    zone_t zone;
    int64_t time;
};