#include "stm32f1xx_hal.h"
#include "oled.h"
#include "wifi_config.h"

I2C_HandleTypeDef hi2c1;
ADC_HandleTypeDef hadc1;

/* 16 x 16 boot-screen Chinese glyphs: 小 花 盆 系 统 正 在 加 载 中 完 毕. */
static const uint8_t boot_hzk[][32] = {
    { 0x00, 0x00, 0xE0, 0x30, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x10, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x04, 0x03, 0x00, 0x20, 0x20, 0x20, 0x20, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x00 },
    { 0x04, 0x04, 0x04, 0xC4, 0x64, 0x0E, 0x04, 0x04, 0xE4, 0x04, 0x8E, 0x84, 0x44, 0x24, 0x04, 0x00, 0x04, 0x02, 0x01, 0x3F, 0x00, 0x08, 0x0C, 0x06, 0x3F, 0x21, 0x21, 0x20, 0x20, 0x20, 0x1C, 0x00 },
    { 0x90, 0x90, 0x88, 0x58, 0x54, 0x32, 0x10, 0x90, 0x92, 0x94, 0x94, 0xF8, 0x10, 0x10, 0x10, 0x00, 0x20, 0x20, 0x3E, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x3E, 0x20, 0x20, 0x00 },
    { 0x00, 0x24, 0x24, 0x34, 0x34, 0xAC, 0xAC, 0x64, 0x64, 0x64, 0xA2, 0x32, 0x12, 0x02, 0x00, 0x00, 0x20, 0x22, 0x12, 0x1B, 0x0B, 0x22, 0x22, 0x3E, 0x02, 0x02, 0x0A, 0x1B, 0x13, 0x32, 0x24, 0x00 },
    { 0x60, 0xD0, 0x4C, 0x62, 0x20, 0x88, 0xC8, 0xA8, 0xB8, 0x9A, 0x8C, 0xA8, 0xC8, 0x88, 0x08, 0x00, 0x12, 0x0B, 0x09, 0x25, 0x25, 0x10, 0x08, 0x07, 0x00, 0x00, 0x3F, 0x20, 0x20, 0x20, 0x1D, 0x00 },
    { 0x00, 0x00, 0x02, 0xF2, 0x02, 0x02, 0x02, 0x02, 0xFE, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00, 0x00, 0x00, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00 },
    { 0x00, 0x04, 0x04, 0xC4, 0x34, 0x8C, 0x87, 0x84, 0x84, 0xF4, 0x84, 0x84, 0x84, 0x84, 0x04, 0x00, 0x00, 0x04, 0x03, 0x3F, 0x00, 0x20, 0x20, 0x20, 0x20, 0x3F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00 },
    { 0x00, 0x10, 0x10, 0xFE, 0x10, 0x10, 0xF0, 0x00, 0x00, 0xF8, 0x08, 0x08, 0x08, 0xF8, 0x00, 0x00, 0x00, 0x20, 0x1C, 0x27, 0x20, 0x20, 0x1F, 0x00, 0x00, 0x3F, 0x10, 0x10, 0x10, 0x3F, 0x00, 0x00 },
    { 0x50, 0x54, 0xD4, 0x74, 0x5E, 0x54, 0x54, 0x54, 0x50, 0xFE, 0x10, 0x12, 0x96, 0x54, 0x10, 0x00, 0x10, 0x13, 0x12, 0x12, 0x3F, 0x0A, 0x0A, 0x2A, 0x10, 0x18, 0x0F, 0x1A, 0x21, 0x20, 0x1C, 0x00 },
    { 0x00, 0xF0, 0x10, 0x10, 0x10, 0x10, 0x10, 0xFE, 0x10, 0x10, 0x10, 0x10, 0x10, 0xF0, 0x00, 0x00, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x3F, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00 },
    { 0x00, 0x18, 0x08, 0x48, 0x48, 0x48, 0x4A, 0x4C, 0x48, 0x48, 0x48, 0x48, 0x48, 0x18, 0x00, 0x00, 0x00, 0x21, 0x21, 0x11, 0x19, 0x07, 0x01, 0x01, 0x3F, 0x21, 0x21, 0x21, 0x21, 0x1D, 0x00, 0x00 },
    { 0x00, 0x00, 0xFE, 0x88, 0x88, 0x48, 0x48, 0x00, 0xFE, 0x90, 0x90, 0x88, 0x88, 0x84, 0x64, 0x00, 0x00, 0x08, 0x09, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00 }
};

/* YL-69 calibration measured with this probe and soil. */
#define SOIL_ADC_WET  1134U
#define SOIL_ADC_DRY  4001U
#define SOIL_SAMPLES          64U
#define SOIL_FILTER_SHIFT      3U  /* 1/8 IIR low-pass filter. */
#define SOIL_DISPLAY_DEADBAND  2U
#define DISPLAY_PERIOD_MS    500U

/* KEY2 is PB12 and is wired to GND when pressed. It manually selects the
   OLED page, so the display stays on the page the user is reading. */
#define OLED_PAGE_KEY_PORT   GPIOB
#define OLED_PAGE_KEY_PIN GPIO_PIN_12
#define OLED_PAGE_DEBOUNCE_MS  50U
#define OLED_PAGE_COUNT         4U

/* The other three front-panel buttons are also pulled up and go low when
   pressed.  They provide safe, visible manual controls for the flowerpot. */
#define PUMP_MANUAL_KEY_PORT GPIOB
#define PUMP_MANUAL_KEY_PIN GPIO_PIN_13
#define SOIL_LIMIT_KEY_PORT GPIOB
#define SOIL_LIMIT_KEY_PIN GPIO_PIN_14
#define LAMP_MODE_KEY_PORT GPIOB
#define LAMP_MODE_KEY_PIN GPIO_PIN_15
#define KEY_DEBOUNCE_MS 50U

/* BUZ1 is marked 2.4 kHz. PB5 (TIM3_CH2) therefore supplies a 2.4 kHz
   tone through the S8050 transistor, which works with a passive buzzer. */
#define BUZZER_GPIO_PORT GPIOB
#define BUZZER_GPIO_PIN GPIO_PIN_5
#define BUZZER_SHORT_BEEP_MS 120U
#define BUZZER_CONFIRM_BEEP_MS 400U
#define BUZZER_KEY4_BEEP_MS 1000U
#define BUZZER_DRY_ALARM_MS 3000U
#define PUMP_MODE_DOUBLE_CLICK_MS 500U

/* MOSFET-15A automatic watering. The module is active high: PB1 high turns
   the pump on. Calibration makes 30% correspond to approximately ADC 3140.
   A short dose, soak delay and cycle limit protect a small plant pot. */
#define PUMP_AUTO_ENABLED        1U
#define PUMP_GPIO_PORT        GPIOB
#define PUMP_GPIO_PIN      GPIO_PIN_1
#define PUMP_DRY_THRESHOLD       30U
#define PUMP_RECOVER_THRESHOLD   35U	
#define PUMP_AUTO_WATER_MS     3000U
#define PUMP_MANUAL_WATER_MS   3000U
#define PUMP_SOAK_MS        120000U
#define PUMP_MAX_CYCLES          3U
/* Probe out of soil measured 4077-4095. Do not water on that invalid value. */
#define SOIL_SENSOR_AIR_ADC    4050U

#define LAMP_MODE_AUTO       0U
#define LAMP_MODE_FORCE_ON   1U
#define LAMP_MODE_FORCE_OFF  2U

/* Four-wire photoresistor module: AO is connected to PA0. */
#define LIGHT_ENABLED           1U
#if LIGHT_ENABLED
#define LIGHT_ADC_CHANNEL    ADC_CHANNEL_0
#define LIGHT_SAMPLES              32U

/* AO rises when this module gets darker (measured: bright ~= 110, dark ~=
   2789). Use two thresholds and a delay so the lamp cannot chatter at the
   boundary. The photoresistor must be shielded from the lamp's direct beam. */
#define LAMP_DARK_ON_ADC         1800U
#define LAMP_BRIGHT_OFF_ADC      1000U
#define LAMP_CONFIRM_MS         10000U
#define LAMP_ON_HOUR                 8U  /* 08:00 local time */
#define LAMP_OFF_HOUR               22U  /* 22:00 local time */
#endif

/* SGP30 shares I2C1 (PB6/PB7) with the OLED. */
#define SGP30_ENABLED           1U
#if SGP30_ENABLED
#define SGP30_I2C_ADDRESS     (0x58U << 1)
#define SGP30_PERIOD_MS        1000U
#define SGP30_WARMUP_MS       15000U
#endif

#if SGP30_ENABLED
static uint8_t sgp30_last_response[6];
static uint8_t sgp30_crc0_expected;
static uint8_t sgp30_crc1_expected;
#endif
#define ESP01S_ENABLED          1U  /* ESP-01S uses USART2: PA2 TX, PA3 RX. */
#if ESP01S_ENABLED
#define ESP_AT_PERIOD_MS     3000U
#define ESP_AT_TIMEOUT_MS    1000U
#define ESP_AT_BAUD        115200U
#define ESP_WIFI_RETRY_MS   10000U
#define ESP_SEND_PERIOD_MS   5000U
#define ESP_NTP_QUERY_MS    30000U
#define ESP_NTP_RETRY_MS    30000U
#define ESP_NTP_TIMEOUT_MS   5000U
#endif

/* CJDHT11 module: DATA is connected to STM32 PA11. */
#define DHT11_ENABLED           1U
#if DHT11_ENABLED
#define DHT11_PERIOD_MS      2000U
#define DHT11_GPIO_PORT      GPIOA
#define DHT11_GPIO_PIN       GPIO_PIN_11
#endif

static void SystemClock_Config(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
static void MX_GPIO_Init(void);
static void Pump_GPIO_Init(void);
static void Pump_Set(uint8_t on);
static void OLED_ShowBootScreen(void);
static void Buzzer_GPIO_Init(void);
static void Buzzer_Beep(uint32_t duration_ms);
static void Buzzer_Task(void);
static uint8_t Key_Pressed(GPIO_TypeDef *port, uint16_t pin,
                           GPIO_PinState *last_state,
                           uint32_t *last_pressed_at);
#if ESP01S_ENABLED
static void ESP_USART2_Init(void);
static uint8_t ESP_ConnectWiFi(void);
static uint8_t ESP_SendSensorData(uint8_t temperature, uint8_t humidity,
                                  uint8_t soil, uint16_t soil_adc,
                                  uint16_t eco2, uint16_t tvoc,
                                  uint16_t light_adc);
static uint8_t ESP_StartNetworkTime(void);
static uint8_t ESP_GetNetworkHour(uint8_t *hour);
#endif
static void Lamp_PWM_Init(void);
static void Lamp_SetBrightness(uint8_t percent);
#if LIGHT_ENABLED
static void Lamp_AutoControl(uint16_t light_adc, uint8_t time_valid,
                             uint8_t hour, uint8_t lamp_mode,
                             uint8_t *lamp_is_on,
                             uint8_t *pending_state,
                             uint32_t *pending_since);
#endif
#if DHT11_ENABLED
static void DWT_DelayInit(void);
static uint8_t DHT11_Read(uint8_t *temperature, uint8_t *humidity);
#endif
#if SGP30_ENABLED
static uint8_t SGP30_Init(void);
static uint8_t SGP30_Read(uint16_t *eco2, uint16_t *tvoc);
static void SGP30_ShowHexByte(uint8_t x, uint8_t y, uint8_t value);
#endif
static uint16_t ADC_ReadAverage(uint32_t channel, uint32_t samples);
static uint8_t Soil_ToPercent(uint16_t adc);
static void Error_Handler(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_I2C1_Init();
    MX_ADC1_Init();
    MX_GPIO_Init();
    Pump_GPIO_Init();
    Buzzer_GPIO_Init();
#if ESP01S_ENABLED
    ESP_USART2_Init();
#endif
    Lamp_PWM_Init();
#if DHT11_ENABLED
    DWT_DelayInit();
#endif

    OLED_Init();
    OLED_ColorTurn(0);
    OLED_DisplayTurn(0);
    OLED_ShowBootScreen();
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t *)"Soil:000%", 16, 1);
    OLED_ShowString(0, 16, (uint8_t *)"ADC:0000", 16, 1);
    OLED_ShowString(0, 32, (uint8_t *)"T:--C H:--% WAIT", 16, 1);
    OLED_ShowString(0, 48, (uint8_t *)"ESP:WAIT        ", 16, 1);
    OLED_Refresh();

    {
    uint16_t soil_display = 0U;
#if LIGHT_ENABLED
    uint16_t light_adc = 0U;
#endif
    int32_t soil_filtered_x8 = -1;
    uint32_t display_at = 0U;
    uint32_t pump_state_at = HAL_GetTick();
    uint32_t dht11_at = HAL_GetTick();
    uint32_t esp_at = HAL_GetTick();
    uint32_t esp_send_at = HAL_GetTick();
#if ESP01S_ENABLED
    uint32_t esp_ntp_at = HAL_GetTick();
    uint8_t esp_ntp_configured = 0U;
    uint8_t esp_ntp_status = 0U;
    uint8_t network_time_valid = 0U;
    uint8_t network_hour = 0U;
#endif
#if SGP30_ENABLED
    uint32_t sgp30_at = HAL_GetTick();
    uint32_t sgp30_started_at = HAL_GetTick();
    uint32_t oled_page_key_at = HAL_GetTick();
    GPIO_PinState oled_page_key_last = GPIO_PIN_SET;
#endif
    uint8_t air_temperature = 0U;
    uint8_t air_humidity = 0U;
    uint8_t dht11_status = 0U;
    uint8_t esp_status = 0U;
    uint8_t esp_wifi_connected = 0U;
    uint8_t esp_send_failures = 0U;
    uint8_t pump_is_on = 0U;
    uint8_t pump_soaking = 0U;
    uint8_t pump_cycles = 0U;
    uint8_t pump_manual_active = 0U;
    uint8_t pump_auto_mode = 1U;
    uint8_t pump_mode_key_waiting = 0U;
    uint8_t dry_alarm_sent = 0U;
    uint8_t soil_dry_limit = PUMP_DRY_THRESHOLD;
    uint8_t soil_recover_limit = PUMP_RECOVER_THRESHOLD;
    uint32_t pump_manual_key_at = HAL_GetTick();
    uint32_t pump_mode_key_first_at = HAL_GetTick();
    uint32_t soil_limit_key_at = HAL_GetTick();
    uint32_t lamp_mode_key_at = HAL_GetTick();
    GPIO_PinState pump_manual_key_last = GPIO_PIN_SET;
    GPIO_PinState soil_limit_key_last = GPIO_PIN_SET;
    GPIO_PinState lamp_mode_key_last = GPIO_PIN_SET;
    uint32_t soil_limit_notice_at = 0U;
#if SGP30_ENABLED
    uint16_t sgp30_eco2 = 0U;
    uint16_t sgp30_tvoc = 0U;
    uint8_t sgp30_status;
    uint8_t sgp30_page = 0U;
#endif
#if LIGHT_ENABLED
    uint8_t lamp_is_on = 0U;
    uint8_t lamp_pending_state = 0U;
    uint8_t lamp_mode = LAMP_MODE_AUTO;
    uint32_t lamp_pending_since = HAL_GetTick();
#endif

    /* The network-time controlled lamp starts safely off. */
    Lamp_SetBrightness(0U);
    Pump_Set(0U);
#if SGP30_ENABLED
    sgp30_status = SGP30_Init();
#endif

    while (1)
    {
        uint16_t soil_raw = ADC_ReadAverage(ADC_CHANNEL_1, SOIL_SAMPLES);
        uint16_t soil_adc;
        uint8_t moisture;
        uint32_t now = HAL_GetTick();
#if SGP30_ENABLED
        GPIO_PinState oled_page_key;
#endif

#if SGP30_ENABLED
        oled_page_key = HAL_GPIO_ReadPin(OLED_PAGE_KEY_PORT,
                                         OLED_PAGE_KEY_PIN);
        if ((oled_page_key == GPIO_PIN_RESET) &&
            (oled_page_key_last != GPIO_PIN_RESET) &&
            ((now - oled_page_key_at) >= OLED_PAGE_DEBOUNCE_MS))
        {
            sgp30_page++;
            if (sgp30_page >= OLED_PAGE_COUNT) sgp30_page = 0U;
            oled_page_key_at = now;
        }
        oled_page_key_last = oled_page_key;
#endif

#if LIGHT_ENABLED
        light_adc = ADC_ReadAverage(LIGHT_ADC_CHANNEL, LIGHT_SAMPLES);
#endif

        if (soil_filtered_x8 < 0)
        {
            soil_filtered_x8 = (int32_t)soil_raw << SOIL_FILTER_SHIFT;
            soil_display = soil_raw;
        }
        else
        {
            soil_filtered_x8 +=
                ((int32_t)soil_raw -
                 (soil_filtered_x8 >> SOIL_FILTER_SHIFT));
        }
        soil_adc = (uint16_t)(soil_filtered_x8 >> SOIL_FILTER_SHIFT);
        moisture = Soil_ToPercent(soil_adc);

#if PUMP_AUTO_ENABLED
        /* A removed probe is an invalid reading, not dry soil. Stop a pump
           immediately in that case so it cannot keep watering unattended. */
        if (soil_adc >= SOIL_SENSOR_AIR_ADC)
        {
            if (pump_is_on) Pump_Set(0U);
            pump_is_on = 0U;
            pump_soaking = 0U;
            pump_cycles = 0U;
            pump_manual_active = 0U;
            dry_alarm_sent = 0U;
            pump_state_at = now;
        }
        else if (pump_is_on)
        {
            if ((now - pump_state_at) >=
                (pump_manual_active ? PUMP_MANUAL_WATER_MS : PUMP_AUTO_WATER_MS))
            {
                Pump_Set(0U);
                pump_is_on = 0U;
                /* Manual watering ends immediately.  Only automatic doses
                   enter the two-minute soak period. */
                if (pump_manual_active)
                {
                    pump_manual_active = 0U;
                    pump_soaking = 0U;
                }
                else
                {
                    pump_soaking = 1U;
                }
                pump_state_at = now;
            }
        }
        else if (pump_auto_mode && pump_soaking)
        {
            if ((now - pump_state_at) >= PUMP_SOAK_MS)
            {
                pump_soaking = 0U;
                pump_state_at = now;
            }
        }
        else if (pump_auto_mode && (moisture >= soil_recover_limit))
        {
            pump_cycles = 0U;
            dry_alarm_sent = 0U;
        }
        else if (pump_auto_mode && (moisture <= soil_dry_limit) &&
                 (pump_cycles < PUMP_MAX_CYCLES))
        {
            Pump_Set(1U);
            pump_is_on = 1U;
            pump_manual_active = 0U;
            pump_cycles++;
            pump_state_at = now;
            Buzzer_Beep(BUZZER_SHORT_BEEP_MS);
        }
        else if (pump_auto_mode && (moisture <= soil_dry_limit) &&
                 (pump_cycles >= PUMP_MAX_CYCLES) && !dry_alarm_sent)
        {
            /* Three short doses did not recover the soil: ask the user to
               check the water tank, tube, and probe rather than retrying. */
            Buzzer_Beep(BUZZER_DRY_ALARM_MS);
            dry_alarm_sent = 1U;
        }

        /* KEY3 uses a double click to switch AUTO/MANUAL safely. In MANUAL,
           the first click starts the three-second dose immediately; a second
           quick click cancels that dose and returns to AUTO. */
        if (Key_Pressed(PUMP_MANUAL_KEY_PORT, PUMP_MANUAL_KEY_PIN,
                        &pump_manual_key_last, &pump_manual_key_at))
        {
            if (pump_auto_mode)
            {
                if (pump_mode_key_waiting &&
                    ((now - pump_mode_key_first_at) <= PUMP_MODE_DOUBLE_CLICK_MS))
                {
                    pump_mode_key_waiting = 0U;
                    pump_auto_mode = 0U;
                    pump_cycles = 0U;
                    dry_alarm_sent = 0U;
                    pump_soaking = 0U;
                    if (pump_is_on)
                    {
                        Pump_Set(0U);
                        pump_is_on = 0U;
                        pump_manual_active = 0U;
                    }
                    Buzzer_Beep(BUZZER_CONFIRM_BEEP_MS);
                }
                else
                {
                    pump_mode_key_waiting = 1U;
                    pump_mode_key_first_at = now;
                }
            }
            else
            {
                if (pump_mode_key_waiting &&
                    ((now - pump_mode_key_first_at) <= PUMP_MODE_DOUBLE_CLICK_MS))
                {
                    /* MANUAL double click: cancel the dose and restore AUTO. */
                    pump_mode_key_waiting = 0U;
                    pump_auto_mode = 1U;
                    pump_soaking = 0U;
                    pump_cycles = 0U;
                    dry_alarm_sent = 0U;
                    if (pump_is_on)
                    {
                        Pump_Set(0U);
                        pump_is_on = 0U;
                        pump_manual_active = 0U;
                    }
                    Buzzer_Beep(BUZZER_CONFIRM_BEEP_MS);
                }
                else
                {
                    pump_mode_key_waiting = 1U;
                    pump_mode_key_first_at = now;
                    /* Manual mode deliberately accepts a user command even
                       when the probe is unplugged; only AUTO is blocked by
                       an invalid probe reading. */
                    if (!pump_is_on)
                    {
                        Pump_Set(1U);
                        pump_is_on = 1U;
                        pump_manual_active = 1U;
                        pump_state_at = now;
                        Buzzer_Beep(BUZZER_CONFIRM_BEEP_MS);
                    }
                }
            }
        }
        if (pump_mode_key_waiting &&
            ((now - pump_mode_key_first_at) > PUMP_MODE_DOUBLE_CLICK_MS))
        {
            pump_mode_key_waiting = 0U;
        }

        /* KEY4: choose a reasonable dry-soil limit: 25, 30, or 35 percent.
           Recovery remains five points above the selected limit. */
        if (Key_Pressed(SOIL_LIMIT_KEY_PORT, SOIL_LIMIT_KEY_PIN,
                        &soil_limit_key_last, &soil_limit_key_at))
        {
            if (soil_dry_limit >= 35U) soil_dry_limit = 25U;
            else soil_dry_limit += 5U;
            soil_recover_limit = (uint8_t)(soil_dry_limit + 5U);
            pump_cycles = 0U;
            dry_alarm_sent = 0U;
            soil_limit_notice_at = now;
            /* A full second makes the threshold-setting confirmation
               unambiguous during a demonstration. */
            Buzzer_Beep(BUZZER_KEY4_BEEP_MS);
        }
#endif

#if LIGHT_ENABLED
        /* KEY5: auto daylight logic, then fixed-on and fixed-off test modes.
           Fixed-on is particularly useful for separating lamp hardware faults
           from light-sensor control during debugging. */
        if (Key_Pressed(LAMP_MODE_KEY_PORT, LAMP_MODE_KEY_PIN,
                        &lamp_mode_key_last, &lamp_mode_key_at))
        {
            lamp_mode++;
            if (lamp_mode > LAMP_MODE_FORCE_OFF) lamp_mode = LAMP_MODE_AUTO;
            Buzzer_Beep(BUZZER_SHORT_BEEP_MS);
        }
#endif

        Buzzer_Task();
 
        if ((soil_adc > soil_display + SOIL_DISPLAY_DEADBAND) ||
            (soil_display > soil_adc + SOIL_DISPLAY_DEADBAND))
        {
            soil_display = soil_adc;
        }

        if ((now - dht11_at) >= DHT11_PERIOD_MS)
        {
            dht11_at = now; 
            dht11_status = DHT11_Read(&air_temperature, &air_humidity);
        }

#if SGP30_ENABLED
        /* The SGP30 air-quality algorithm expects one measurement each second. */
        if ((now - sgp30_at) >= SGP30_PERIOD_MS)
        {
            sgp30_at = now;
            sgp30_status = SGP30_Read(&sgp30_eco2, &sgp30_tvoc);
        }
#endif

        if ((!esp_wifi_connected) &&
            ((now - esp_at) >= ((esp_status == 0U) ?
                                ESP_AT_PERIOD_MS : ESP_WIFI_RETRY_MS)))
        {
            esp_status = ESP_ConnectWiFi();
            esp_wifi_connected = (esp_status == 5U) ? 1U : 0U;
            esp_ntp_configured = 0U;
            esp_ntp_status = 0U;
            network_time_valid = 0U;
            esp_at = HAL_GetTick();
            esp_send_at = esp_at;
            esp_ntp_at = esp_at - ESP_NTP_RETRY_MS;
        }

        /* ESP-01S gets China local time (UTC+8) from an SNTP server. A
           failed or not-yet-updated clock keeps the lamp off. */
        if (esp_wifi_connected && !esp_ntp_configured &&
            ((now - esp_ntp_at) >= ESP_NTP_RETRY_MS))
        {
            esp_ntp_configured = ESP_StartNetworkTime();
            esp_ntp_at = HAL_GetTick();
            esp_ntp_status = esp_ntp_configured ? 1U : 3U;
        }

        if (esp_wifi_connected && esp_ntp_configured &&
            ((now - esp_ntp_at) >= ESP_NTP_QUERY_MS))
        {
            esp_ntp_at = now;
            network_time_valid = ESP_GetNetworkHour(&network_hour);
            esp_ntp_status = network_time_valid ? 2U : 4U;
        }

#if LIGHT_ENABLED
        Lamp_AutoControl(light_adc, network_time_valid, network_hour, lamp_mode,
                         &lamp_is_on, &lamp_pending_state,
                         &lamp_pending_since);
#endif

        if (esp_wifi_connected &&
            ((now - esp_send_at) >= ESP_SEND_PERIOD_MS))
        {
            esp_send_at = now;
            if (ESP_SendSensorData(air_temperature, air_humidity,
                                   moisture, soil_display,
#if SGP30_ENABLED
                                   sgp30_eco2, sgp30_tvoc,
#if LIGHT_ENABLED
                                   light_adc))
#else
                                   0U))
#endif
#else
                                   0U, 0U,
#if LIGHT_ENABLED
                                   light_adc))
#else
                                   0U))
#endif
#endif
            {
                esp_status = 9U;
                esp_send_failures = 0U;
            }
            else
            {
                esp_status = 10U;
                esp_send_failures++;
                if (esp_send_failures >= 3U)
                {
                    esp_wifi_connected = 0U;
                    esp_send_failures = 0U;
                    esp_ntp_configured = 0U;
                    esp_ntp_status = 0U;
                    network_time_valid = 0U;
                    esp_at = HAL_GetTick();
                }
            }
        }

        if ((now - display_at) >= DISPLAY_PERIOD_MS)
        {
            display_at = now; 
#if SGP30_ENABLED
            if (sgp30_page == 1U)
            {
                OLED_ShowString(0, 0, (uint8_t *)"SGP30 AIR TEST  ", 16, 1);
                OLED_ShowString(0, 16, (uint8_t *)"CO2:00000ppm   ", 16, 1);
                OLED_ShowNum(32, 16, sgp30_eco2, 5, 16, 1);
                OLED_ShowString(0, 32, (uint8_t *)"VOC:00000ppb   ", 16, 1);
                OLED_ShowNum(32, 32, sgp30_tvoc, 5, 16, 1);
                if (sgp30_status == 1U)
                {
                    if ((now - sgp30_started_at) < SGP30_WARMUP_MS)
                        OLED_ShowString(0, 48, (uint8_t *)"SGP:WARMUP     ", 16, 1);
                    else
                        OLED_ShowString(0, 48, (uint8_t *)"SGP:OK         ", 16, 1);
                }
                else if (sgp30_status == 2U)
                {
                    OLED_ShowString(0, 48, (uint8_t *)"SGP:I2C FAIL   ", 16, 1);
                }
                else
                {
                    /* Show the six response bytes and the CRC comparison.
                       This is only a temporary diagnostic view for I2C faults. */
                    OLED_ShowString(0, 0, (uint8_t *)"SGP CRC DEBUG  ", 16, 1);
                    OLED_ShowString(0, 16, (uint8_t *)"R:             ", 16, 1);
                    SGP30_ShowHexByte(16, 16, sgp30_last_response[0]);
                    SGP30_ShowHexByte(40, 16, sgp30_last_response[1]);
                    SGP30_ShowHexByte(64, 16, sgp30_last_response[2]);
                    OLED_ShowString(0, 32, (uint8_t *)"R:             ", 16, 1);
                    SGP30_ShowHexByte(16, 32, sgp30_last_response[3]);
                    SGP30_ShowHexByte(40, 32, sgp30_last_response[4]);
                    SGP30_ShowHexByte(64, 32, sgp30_last_response[5]);
                    OLED_ShowString(0, 48, (uint8_t *)"C:             ", 16, 1);
                    SGP30_ShowHexByte(16, 48, sgp30_crc0_expected);
                    OLED_ShowChar(32, 48, '/', 16, 1);
                    SGP30_ShowHexByte(40, 48, sgp30_last_response[2]);
                    SGP30_ShowHexByte(64, 48, sgp30_crc1_expected);
                    OLED_ShowChar(80, 48, '/', 16, 1);
                    SGP30_ShowHexByte(88, 48, sgp30_last_response[5]);
                }
            }
            else if (sgp30_page == 2U)
            {
                if (lamp_mode == LAMP_MODE_FORCE_ON)
                    OLED_ShowString(0, 0, (uint8_t *)"LIGHT FORCE ON  ", 16, 1);
                else if (lamp_mode == LAMP_MODE_FORCE_OFF)
                    OLED_ShowString(0, 0, (uint8_t *)"LIGHT FORCE OFF ", 16, 1);
                else
                    OLED_ShowString(0, 0, (uint8_t *)"LIGHT AUTO      ", 16, 1);
                OLED_ShowString(0, 16, (uint8_t *)"AO ADC:0000     ", 16, 1);
#if LIGHT_ENABLED
                OLED_ShowNum(56, 16, light_adc, 4, 16, 1);
#endif
                OLED_ShowString(0, 32, (uint8_t *)"TIME:-- LAMP:-- ", 16, 1);
#if ESP01S_ENABLED
                if (network_time_valid)
                {
                    OLED_ShowNum(40, 32, network_hour, 2, 16, 1);
                    OLED_ShowString(104, 32,
                                    (uint8_t *)(lamp_is_on ? "ON " : "OFF"),
                                    16, 1);
                }
                else
                {
                    if (esp_ntp_status == 1U)
                    {
                        OLED_ShowString(0, 32,
                                        (uint8_t *)"TIME:-- NTP:WAIT", 16, 1);
                    }
                    else if (esp_ntp_status == 3U)
                    {
                        OLED_ShowString(0, 32,
                                        (uint8_t *)"TIME:-- NTP:CMD!", 16, 1);
                    }
                    else if (esp_ntp_status == 4U)
                    {
                        OLED_ShowString(0, 32,
                                        (uint8_t *)"TIME:-- NTP:QERR", 16, 1);
                    }
                    else
                    {
                        OLED_ShowString(0, 32,
                                        (uint8_t *)"TIME:-- NTP:OFF  ", 16, 1);
                    }
                }
#endif
                OLED_ShowString(0, 48, (uint8_t *)"KEY5 MODE PB15  ", 16, 1);
            }
            else if (sgp30_page == 3U)
            {
                /* Pump status owns the home-page footer, so ESP gets a
                   dedicated page rather than hiding communication results. */
                OLED_ShowString(0, 0, (uint8_t *)"ESP-01S STATUS  ", 16, 1);
#if ESP01S_ENABLED
                OLED_ShowString(0, 16, (uint8_t *)(esp_wifi_connected ?
                                "WIFI:CONNECTED " : "WIFI:CONNECTING"),
                                16, 1);
                if (esp_status == 1U)
                    OLED_ShowString(0, 32, (uint8_t *)"ESP:MODE FAIL   ", 16, 1);
                else if (esp_status == 2U)
                    OLED_ShowString(0, 32, (uint8_t *)"ESP:AT FAIL     ", 16, 1);
                else if (esp_status == 5U)
                    OLED_ShowString(0, 32, (uint8_t *)"ESP:WIFI OK     ", 16, 1);
                else if (esp_status == 6U)
                    OLED_ShowString(0, 32, (uint8_t *)"ESP:WIFI FAIL   ", 16, 1);
                else if (esp_status == 7U)
                    OLED_ShowString(0, 32, (uint8_t *)"ESP:TCP OK      ", 16, 1);
                else if (esp_status == 8U)
                    OLED_ShowString(0, 32, (uint8_t *)"ESP:TCP FAIL    ", 16, 1);
                else if (esp_status == 9U)
                    OLED_ShowString(0, 32, (uint8_t *)"ESP:DATA SENT   ", 16, 1);
                else if (esp_status == 10U)
                    OLED_ShowString(0, 32, (uint8_t *)"ESP:SEND FAIL   ", 16, 1);
                else
                    OLED_ShowString(0, 32, (uint8_t *)"ESP:WAIT        ", 16, 1);

                if (network_time_valid)
                {
                    OLED_ShowString(0, 48, (uint8_t *)"TIME:00 NTP:OK ", 16, 1);
                    OLED_ShowNum(40, 48, network_hour, 2, 16, 1);
                }
                else
                {
                    OLED_ShowString(0, 48, (uint8_t *)"TIME:-- NTP:WAIT", 16, 1);
                }
#else
                OLED_ShowString(0, 16, (uint8_t *)"ESP:DISABLED    ", 16, 1);
                OLED_ShowString(0, 32, (uint8_t *)"KEY2 NEXT PAGE  ", 16, 1);
                OLED_ShowString(0, 48, (uint8_t *)"                ", 16, 1);
#endif
            }
            else
            {
                /* Rewrite fixed labels after the SGP30 page so stale text
                   from the other page cannot remain on the OLED. */
                /* The selected limit and AUTO/MAN mode stay visible even
                   when the probe is removed or the pump is busy. */
                OLED_ShowString(0, 0,
                                (uint8_t *)(pump_auto_mode ?
                                "S:000% AUTO L00 " : "S:000% MAN  L00 "),
                                16, 1);
                OLED_ShowNum(16, 0, moisture, 3, 16, 1);
                OLED_ShowNum(104, 0, soil_dry_limit, 2, 16, 1);
                OLED_ShowString(0, 16, (uint8_t *)"ADC:0000        ", 16, 1);
                OLED_ShowNum(32, 16, soil_display, 4, 16, 1);
                if (dht11_status == 1U)
                {
                    OLED_ShowString(0, 32, (uint8_t *)"T:00C H:00% OK  ", 16, 1);
                    OLED_ShowNum(16, 32, air_temperature, 2, 16, 1);
                    OLED_ShowNum(64, 32, air_humidity, 2, 16, 1);
                }
                else if (dht11_status == 0U)
                {
                    OLED_ShowString(0, 32, (uint8_t *)"T:--C H:--% WAIT", 16, 1);
                }
                else
                {
                    OLED_ShowString(0, 32, (uint8_t *)"DHT ERR CODE:0  ", 16, 1);
                    OLED_ShowNum(104, 32, dht11_status, 1, 16, 1);
                }
                if (PUMP_AUTO_ENABLED)
                {
                    if (pump_is_on && pump_manual_active)
                    {
                        OLED_ShowString(0, 48,
                                        (uint8_t *)"PUMP:MANUAL ON  ", 16, 1);
                    }
                    else if (soil_display >= SOIL_SENSOR_AIR_ADC)
                    {
                        OLED_ShowString(0, 48,
                                        (uint8_t *)"PUMP:CHECK SOIL ", 16, 1);
                    }
                    else if (pump_is_on)
                    {
                        OLED_ShowString(0, 48,
                                        (uint8_t *)"PUMP:ON 0/0     ", 16, 1);
                        OLED_ShowNum(64, 48, pump_cycles, 1, 16, 1);
                        OLED_ShowNum(80, 48, PUMP_MAX_CYCLES, 1, 16, 1);
                    }
                    else if (pump_soaking)
                    {
                        OLED_ShowString(0, 48,
                                        (uint8_t *)"PUMP:SOAK WAIT  ", 16, 1);
                    }
                    else if (!pump_auto_mode)
                    {
                        OLED_ShowString(0, 48,
                                        (uint8_t *)"PUMP:MANUAL KEY3", 16, 1);
                    }
                    else if ((now - soil_limit_notice_at) < 2000U)
                    {
                        OLED_ShowString(0, 48,
                                        (uint8_t *)"SOIL LIMIT:00%  ", 16, 1);
                        OLED_ShowNum(88, 48, soil_dry_limit, 2, 16, 1);
                    }
                    else if ((moisture <= soil_dry_limit) &&
                             (pump_cycles >= PUMP_MAX_CYCLES))
                    {
                        OLED_ShowString(0, 48,
                                        (uint8_t *)"PUMP:CHECK WATER", 16, 1);
                    }
                    else
                    {
                        OLED_ShowString(0, 48,
                                        (uint8_t *)"PUMP:AUTO<=00%  ", 16, 1);
                        OLED_ShowNum(88, 48, soil_dry_limit, 2, 16, 1);
                    }
                }
                else if (esp_status == 1U)
                {
                    OLED_ShowString(0, 48, (uint8_t *)"ESP:MODE FAIL   ", 16, 1);
                }
                else if (esp_status == 2U)
                {
                    OLED_ShowString(0, 48, (uint8_t *)"ESP:AT FAIL     ", 16, 1);
                }
                else if (esp_status == 5U)
                {
                    OLED_ShowString(0, 48, (uint8_t *)"ESP:WIFI OK     ", 16, 1);
                }
                else if (esp_status == 6U)
                {
                    OLED_ShowString(0, 48, (uint8_t *)"ESP:WIFI FAIL   ", 16, 1);
                }
                else if (esp_status == 7U)
                {
                    OLED_ShowString(0, 48, (uint8_t *)"ESP:TCP OK      ", 16, 1);
                }
                else if (esp_status == 8U)
                {
                    OLED_ShowString(0, 48, (uint8_t *)"ESP:TCP FAIL    ", 16, 1);
                }
                else if (esp_status == 9U)
                {
                    OLED_ShowString(0, 48, (uint8_t *)"ESP:DATA SENT   ", 16, 1);
                }
                else if (esp_status == 10U)
                {
                    OLED_ShowString(0, 48, (uint8_t *)"ESP:SEND FAIL   ", 16, 1);
                }
                else
                {
                    OLED_ShowString(0, 48, (uint8_t *)"ESP:WAIT        ", 16, 1);
                }
            }
#else
            OLED_ShowNum(40, 0, moisture, 3, 16, 1);
#endif
            OLED_Refresh();
        }
        HAL_Delay(20);
    }
    }
}
                                                                                   
static void OLED_ShowBootChinese(uint8_t x, uint8_t y, uint8_t glyph)
{
    uint8_t byte_index;
    uint8_t bit;
    uint8_t column = x;
    uint8_t row_base = y;

    for (byte_index = 0U; byte_index < 32U; byte_index++)
    {
        uint8_t dots = boot_hzk[glyph][byte_index];
        for (bit = 0U; bit < 8U; bit++)
        {
            OLED_DrawPoint(column, (uint8_t)(row_base + bit),
                           (dots & (1U << bit)) ? 1U : 0U);
        }
        column++;
        if ((column - x) == 16U)
        {
            column = x;
            row_base = (uint8_t)(row_base + 8U);
        }
    }
}

static void OLED_ShowBootScreen(void)
{
    uint8_t step;
    uint8_t glyph;

    /* "小花盆系统" and "正在加载中..." are rendered from the small local
       Chinese font above, so they do not depend on the limited default font. */
    for (step = 1U; step <= 10U; step++)
    {
        uint8_t fill_width = (uint8_t)((step * 108U) / 10U);

        OLED_Clear();
        for (glyph = 0U; glyph < 5U; glyph++)
            OLED_ShowBootChinese((uint8_t)(24U + glyph * 16U), 0U, glyph);
        for (glyph = 0U; glyph < 5U; glyph++)
            OLED_ShowBootChinese((uint8_t)(12U + glyph * 16U), 16U,
                                 (uint8_t)(glyph + 5U));
        OLED_ShowString(92U, 16U, (uint8_t *)"...", 16U, 1U);
        OLED_ShowString(32U, 32U, (uint8_t *)"LOADING", 16U, 1U);
        OLED_DrawLine(8U, 48U, 119U, 48U, 1U);
        OLED_DrawLine(8U, 58U, 119U, 58U, 1U);
        OLED_DrawLine(8U, 48U, 8U, 58U, 1U);
        OLED_DrawLine(119U, 48U, 119U, 58U, 1U);
        for (glyph = 0U; glyph < 8U; glyph++)
        {
            OLED_DrawLine(10U, (uint8_t)(50U + glyph),
                          (uint8_t)(10U + fill_width),
                          (uint8_t)(50U + glyph), 1U);
        }
        OLED_Refresh();
        HAL_Delay(200U);
    }

    OLED_Clear();
    for (glyph = 0U; glyph < 5U; glyph++)
        OLED_ShowBootChinese((uint8_t)(24U + glyph * 16U), 0U, glyph);
    OLED_ShowBootChinese(32U, 24U, 7U);  /* 加 */
    OLED_ShowBootChinese(48U, 24U, 8U);  /* 载 */
    OLED_ShowBootChinese(64U, 24U, 10U); /* 完 */
    OLED_ShowBootChinese(80U, 24U, 11U); /* 毕 */
    OLED_ShowString(36U, 44U, (uint8_t *)"READY!", 16U, 1U);
    OLED_Refresh();
    HAL_Delay(1000U);
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    osc.HSIState = RCC_HSI_ON;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) Error_Handler();
    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

static void MX_I2C1_Init(void)
{
    GPIO_InitTypeDef gpio = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();
    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio);
    hi2c1.Instance = I2C1;
    /* OLED and SGP30 share jumper-wire I2C bus. Standard-mode 100 kHz gives
       more signal margin than 400 kHz while the SGP30 is being tested. */
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();
}

static void MX_ADC1_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_ADC_CONFIG(RCC_ADCPCLK2_DIV6); /* 72 MHz / 6 = 12 MHz */

    /* PA1: soil sensor AO; PA0: photoresistor module AO. */
    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    gpio.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &gpio);

    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();

    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    /* PB0 is TIM3_CH3 for the lamp; PB5 is TIM3_CH2 for the buzzer tone. */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    gpio.Pin = GPIO_PIN_0 | BUZZER_GPIO_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio);

    /* KEY2..KEY5: PB12..PB15 are pulled up internally and become low when
       pressed.  The board buttons are wired directly to GND. */
    gpio.Pin = OLED_PAGE_KEY_PIN | PUMP_MANUAL_KEY_PIN |
               SOIL_LIMIT_KEY_PIN | LAMP_MODE_KEY_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(OLED_PAGE_KEY_PORT, &gpio);

#if DHT11_ENABLED
    /* CJDHT11 module DATA idles high. The STM32 pull-up also keeps PA11 in a
       defined state if the module is unplugged. */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    gpio.Pin = DHT11_GPIO_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &gpio);
#endif
}

static void Pump_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    /* PB1 -> H5 pin 1 -> MOSFET-15A IO. The 10 k pull-down on the PCB holds
       the input low before firmware configures PB1. */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    gpio.Pin = PUMP_GPIO_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(PUMP_GPIO_PORT, &gpio);
    HAL_GPIO_WritePin(PUMP_GPIO_PORT, PUMP_GPIO_PIN, GPIO_PIN_RESET);
}

static void Pump_Set(uint8_t on)
{
    HAL_GPIO_WritePin(PUMP_GPIO_PORT, PUMP_GPIO_PIN,
                      on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void Buzzer_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    /* PB5 / TIM3_CH2 -> 1 k -> S8050 base. R9 keeps Q1 off during reset. */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    gpio.Pin = BUZZER_GPIO_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUZZER_GPIO_PORT, &gpio);
}

static uint8_t buzzer_active;
static uint32_t buzzer_started_at;
static uint32_t buzzer_duration_ms;

static void Buzzer_Beep(uint32_t duration_ms)
{
    /* 50% duty cycle at the timer's 2.4 kHz frequency. */
    TIM3->CCR2 = 500U;
    buzzer_active = 1U;
    buzzer_started_at = HAL_GetTick();
    buzzer_duration_ms = duration_ms;
}

static void Buzzer_Task(void)
{
    if (buzzer_active &&
        ((HAL_GetTick() - buzzer_started_at) >= buzzer_duration_ms))
    {
        TIM3->CCR2 = 0U;
        buzzer_active = 0U;
    }
}

static uint8_t Key_Pressed(GPIO_TypeDef *port, uint16_t pin,
                           GPIO_PinState *last_state,
                           uint32_t *last_pressed_at)
{
    GPIO_PinState state = HAL_GPIO_ReadPin(port, pin);
    uint32_t now = HAL_GetTick();
    uint8_t pressed = 0U;

    if ((state == GPIO_PIN_RESET) && (*last_state != GPIO_PIN_RESET) &&
        ((now - *last_pressed_at) >= KEY_DEBOUNCE_MS))
    {
        *last_pressed_at = now;
        pressed = 1U;
    }
    *last_state = state;
    return pressed;
}

#if ESP01S_ENABLED
static void ESP_USART2_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    /* PA2 (STM32 TX) -> ESP-01S RXD. */
    gpio.Pin = GPIO_PIN_2;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* PA3 (STM32 RX) <- ESP-01S TXD. */
    gpio.Pin = GPIO_PIN_3;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio);

    USART2->CR1 = 0U;
    USART2->CR2 = 0U;
    USART2->CR3 = 0U;
    USART2->BRR = (HAL_RCC_GetPCLK1Freq() + (ESP_AT_BAUD / 2U)) /
                  ESP_AT_BAUD;
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

static void ESP_USART2_Send(const char *text)
{
    while (*text != '\0')
    {
        while ((USART2->SR & USART_SR_TXE) == 0U) { }
        USART2->DR = (uint8_t)*text++;
    }
    while ((USART2->SR & USART_SR_TC) == 0U) { }
}

static uint8_t ESP_WaitOK(uint32_t timeout_ms)
{
    uint32_t start;
    uint8_t saw_o = 0U;
    uint8_t error_pos = 0U;
    uint8_t fail_pos = 0U;
    static const char error_text[] = "ERROR";
    static const char fail_text[] = "FAIL";

    start = HAL_GetTick();

    while ((HAL_GetTick() - start) < timeout_ms)
    {
        if ((USART2->SR & USART_SR_RXNE) != 0U)
        {
            uint8_t data = (uint8_t)USART2->DR;
            if (saw_o && (data == 'K')) return 1U;
            saw_o = (data == 'O') ? 1U : 0U;

            if (data == (uint8_t)error_text[error_pos])
            {
                error_pos++;
                if (error_text[error_pos] == '\0') return 0U;
            }
            else
            {
                error_pos = (data == 'E') ? 1U : 0U;
            }

            if (data == (uint8_t)fail_text[fail_pos])
            {
                fail_pos++;
                if (fail_text[fail_pos] == '\0') return 0U;
            }
            else
            {
                fail_pos = (data == 'F') ? 1U : 0U;
            }
        }
    }
    return 0U;
}

static uint8_t ESP_SendCommand(const char *command, uint32_t timeout_ms)
{
    /* Discard old boot text or the remainder of an earlier response. */
    while ((USART2->SR & USART_SR_RXNE) != 0U)
    {
        (void)USART2->DR;
    }
    ESP_USART2_Send(command);
    return ESP_WaitOK(timeout_ms);
}

static uint8_t ESP_WaitText(const char *expected, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    uint16_t position = 0U;

    while ((HAL_GetTick() - start) < timeout_ms)
    {
        if ((USART2->SR & USART_SR_RXNE) != 0U)
        {
            uint8_t data = (uint8_t)USART2->DR;
            if (data == (uint8_t)expected[position])
            {
                position++;
                if (expected[position] == '\0') return 1U;
            }
            else
            {
                position = (data == (uint8_t)expected[0]) ? 1U : 0U;
            }
        }
    }
    return 0U;
}

static char *ESP_AppendText(char *destination, const char *source)
{
    while (*source != '\0') *destination++ = *source++;
    return destination;
}

static char *ESP_AppendUnsigned(char *destination, uint32_t value)
{
    char reverse[10];
    uint8_t count = 0U;

    do
    {
        reverse[count++] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);

    while (count != 0U) *destination++ = reverse[--count];
    return destination;
}

static void ESP_ShowLine(const char *text)
{
    OLED_ShowString(0, 48, (uint8_t *)text, 16, 1);
    OLED_Refresh();
}

static uint8_t ESP_ConnectWiFi(void)
{
    ESP_ShowLine("ESP:AT TEST     ");
    if (!ESP_SendCommand("AT\r\n", ESP_AT_TIMEOUT_MS))
    {
        return 2U;
    }

    ESP_ShowLine("ESP:SET MODE    ");
    if (!ESP_SendCommand("AT+CWMODE=1\r\n", 3000U))
    {
        return 1U;
    }

    ESP_ShowLine("ESP:JOIN WIFI   ");
    if (!ESP_SendCommand("AT+CWJAP=\"" ESP_WIFI_SSID "\",\""
                         ESP_WIFI_PASSWORD "\"\r\n", 25000U))
    {
        return 6U;
    }

    return 5U;
}

static uint8_t ESP_StartNetworkTime(void)
{
    /* ESP-AT SNTP: China Standard Time is UTC+8. The command itself only
       configures SNTP; the first valid time can arrive a few seconds later. */
    ESP_ShowLine("ESP:NTP SETUP  ");
    return ESP_SendCommand("AT+CIPSNTPCFG=1,8,\"cn.ntp.org.cn\","
                           "\"ntp.sjtu.edu.cn\"\r\n", ESP_NTP_TIMEOUT_MS);
}

static uint8_t ESP_ParseNetworkHour(const char *text, uint8_t length,
                                    uint8_t *hour)
{
    uint8_t i;

    /* ESP-AT returns, for example: "Sat Sep 20 21:30:00 2026". */
    for (i = 0U; (uint16_t)i + 4U < (uint16_t)length; i++)
    {
        if ((text[i] >= '0') && (text[i] <= '9') &&
            (text[i + 1U] >= '0') && (text[i + 1U] <= '9') &&
            (text[i + 2U] == ':') &&
            (text[i + 3U] >= '0') && (text[i + 3U] <= '9') &&
            (text[i + 4U] >= '0') && (text[i + 4U] <= '9'))
        {
            uint8_t value = (uint8_t)((text[i] - '0') * 10U +
                                      (text[i + 1U] - '0'));
            if (value < 24U)
            {
                *hour = value;
                return 1U;
            }
        }
    }
    return 0U;
}

static uint8_t ESP_GetNetworkHour(uint8_t *hour)
{
    static const char prefix[] = "+CIPSNTPTIME:";
    char time_text[36];
    uint8_t prefix_at = 0U;
    uint8_t collecting = 0U;
    uint8_t length = 0U;
    uint32_t started = HAL_GetTick();
    uint8_t found = 0U;

    while ((USART2->SR & USART_SR_RXNE) != 0U)
    {
        (void)USART2->DR;
    }
    ESP_USART2_Send("AT+CIPSNTPTIME?\r\n");

    while ((HAL_GetTick() - started) < ESP_NTP_TIMEOUT_MS)
    {
        if ((USART2->SR & USART_SR_RXNE) != 0U)
        {
            uint8_t data = (uint8_t)USART2->DR;

            if (!collecting)
            {
                if (data == (uint8_t)prefix[prefix_at])
                {
                    prefix_at++;
                    if (prefix[prefix_at] == '\0') collecting = 1U;
                }
                else
                {
                    prefix_at = (data == (uint8_t)prefix[0]) ? 1U : 0U;
                }
            }
            else if ((data == '\r') || (data == '\n'))
            {
                if (length != 0U)
                {
                    time_text[length] = '\0';
                    found = ESP_ParseNetworkHour(time_text, length, hour);
                    break;
                }
            }
            else if (length < (uint8_t)(sizeof(time_text) - 1U))
            {
                time_text[length++] = (char)data;
            }
        }
    }

    /* Consume the response's final OK before the next AT command. */
    if (found && ESP_WaitOK(1000U)) return 1U;
    return 0U;
}

static uint8_t ESP_SendSensorData(uint8_t temperature, uint8_t humidity,
                                  uint8_t soil, uint16_t soil_adc,
                                  uint16_t eco2, uint16_t tvoc,
                                  uint16_t light_adc)
{
    char payload[96];
    char send_command[24];
    char *write;
    uint32_t payload_length;

    /* Packet Sender closes its incoming connection after one packet. Open a
       fresh TCP connection for every report instead of reusing a dead link. */
    ESP_ShowLine("ESP:TCP SETUP   ");
    if (!ESP_SendCommand("AT+CIPMUX=0\r\n", 3000U)) return 0U;

    (void)ESP_SendCommand("AT+CIPCLOSE\r\n", 1000U);

    ESP_ShowLine("ESP:TCP CONNECT ");
    if (!ESP_SendCommand("AT+CIPSTART=\"TCP\",\"" ESP_SERVER_IP "\","
                         ESP_SERVER_PORT "\r\n", 10000U))
    {
        return 0U;
    }

    write = payload;
    write = ESP_AppendText(write, "T=");
    write = ESP_AppendUnsigned(write, temperature);
    write = ESP_AppendText(write, ",H=");
    write = ESP_AppendUnsigned(write, humidity);
    write = ESP_AppendText(write, ",SOIL=");
    write = ESP_AppendUnsigned(write, soil);
    write = ESP_AppendText(write, ",ADC=");
    write = ESP_AppendUnsigned(write, soil_adc);
    write = ESP_AppendText(write, ",CO2=");
    write = ESP_AppendUnsigned(write, eco2);
    write = ESP_AppendText(write, ",TVOC=");
    write = ESP_AppendUnsigned(write, tvoc);
    write = ESP_AppendText(write, ",LIGHT=");
    write = ESP_AppendUnsigned(write, light_adc);
    write = ESP_AppendText(write, "\r\n");
    *write = '\0';
    payload_length = (uint32_t)(write - payload);

    write = send_command;
    write = ESP_AppendText(write, "AT+CIPSEND=");
    write = ESP_AppendUnsigned(write, payload_length);
    write = ESP_AppendText(write, "\r\n");
    *write = '\0';

    while ((USART2->SR & USART_SR_RXNE) != 0U)
    {
        (void)USART2->DR;
    }
    ESP_USART2_Send(send_command);
    if (!ESP_WaitText(">", 3000U)) return 0U;

    ESP_USART2_Send(payload);
    if (!ESP_WaitText("SEND OK", 5000U)) return 0U;

    /* It is harmless if the PC has already closed this one-shot socket. */
    (void)ESP_SendCommand("AT+CIPCLOSE\r\n", 1000U);

    return 1U;
}
#endif

#if SGP30_ENABLED
static uint8_t SGP30_CRC8(const uint8_t *data)
{
    uint8_t crc = 0xFFU;
    uint8_t i;
    uint8_t bit;

    for (i = 0U; i < 2U; i++)
    {
        crc ^= data[i];
        for (bit = 0U; bit < 8U; bit++)
        {
            crc = ((crc & 0x80U) != 0U) ?
                  (uint8_t)((crc << 1U) ^ 0x31U) :
                  (uint8_t)(crc << 1U);
        }
    }
    return crc;
}

static uint8_t SGP30_Init(void)
{
    uint8_t command[2] = {0x20U, 0x03U}; /* IAQ init */

    if (HAL_I2C_Master_Transmit(&hi2c1, SGP30_I2C_ADDRESS,
                                command, sizeof(command), 100U) != HAL_OK)
    {
        return 2U;
    }
    HAL_Delay(20U);
    return 1U;
}

static uint8_t SGP30_Read(uint16_t *eco2, uint16_t *tvoc)
{
    uint8_t command[2] = {0x20U, 0x08U}; /* Measure air quality */
    /* STM32F1's polling I2C receive path can duplicate the sixth byte on a
       six-byte transfer. Request one trailing byte and use only bytes 0..5,
       which makes the SGP30's second CRC byte arrive correctly. */
    uint8_t response[7];
    uint8_t i;

    if (HAL_I2C_Master_Transmit(&hi2c1, SGP30_I2C_ADDRESS,
                                command, sizeof(command), 100U) != HAL_OK)
    {
        return 2U;
    }
    HAL_Delay(20U); /* Datasheet requires at least 12 ms. */
    if (HAL_I2C_Master_Receive(&hi2c1, SGP30_I2C_ADDRESS,
                               response, sizeof(response), 100U) != HAL_OK)
    {
        return 2U;
    }
    for (i = 0U; i < 6U; i++)
    {
        sgp30_last_response[i] = response[i];
    }
    sgp30_crc0_expected = SGP30_CRC8(&response[0]);
    sgp30_crc1_expected = SGP30_CRC8(&response[3]);
    if ((sgp30_crc0_expected != response[2]) ||
        (sgp30_crc1_expected != response[5]))
    {
        return 3U;
    }

    *eco2 = (uint16_t)(((uint16_t)response[0] << 8U) | response[1]);
    *tvoc = (uint16_t)(((uint16_t)response[3] << 8U) | response[4]);
    return 1U;
}

static void SGP30_ShowHexByte(uint8_t x, uint8_t y, uint8_t value)
{
    static const char hex[] = "0123456789ABCDEF";
    OLED_ShowChar(x, y, (uint8_t)hex[value >> 4U], 16, 1);
    OLED_ShowChar((uint8_t)(x + 8U), y,
                  (uint8_t)hex[value & 0x0FU], 16, 1);
}
#endif

static void Lamp_PWM_Init(void)
{
    /* APB1 timer clock is 72 MHz. 72/(29+1)/(999+1) = 2.4 kHz.
       CH2 makes the passive buzzer tone; CH3 still controls the lamp. */
    __HAL_RCC_TIM3_CLK_ENABLE();
    TIM3->PSC = 29U;
    TIM3->ARR = 999U;
    TIM3->CCR2 = 0U;
    TIM3->CCR3 = 0U;
    TIM3->CCMR1 = TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 |
                  TIM_CCMR1_OC2PE;
    TIM3->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 |
                  TIM_CCMR2_OC3PE;
    TIM3->CCER = TIM_CCER_CC2E | TIM_CCER_CC3E;
    TIM3->CR1 = TIM_CR1_ARPE;
    TIM3->EGR = TIM_EGR_UG;
    TIM3->CR1 |= TIM_CR1_CEN;
}

static void Lamp_SetBrightness(uint8_t percent)
{
    if (percent > 100U) percent = 100U;
    TIM3->CCR3 = ((uint32_t)percent * 1000U) / 100U;
}

#if LIGHT_ENABLED
static void Lamp_AutoControl(uint16_t light_adc, uint8_t time_valid,
                             uint8_t hour, uint8_t lamp_mode,
                             uint8_t *lamp_is_on,
                             uint8_t *pending_state,
                             uint32_t *pending_since)
{
    uint8_t wanted = *lamp_is_on;
    uint32_t now = HAL_GetTick();

    /* Manual modes bypass the ten-second light confirmation delay. They are
       for a clear lamp-output test and always return to AUTO after cycling. */
    if (lamp_mode == LAMP_MODE_FORCE_ON)
    {
        wanted = 1U;
    }
    else if (lamp_mode == LAMP_MODE_FORCE_OFF)
    {
        wanted = 0U;
    }
    else
    {
        /* No valid Wi-Fi/NTP time, or the protected night period: lamp off. */
        if (!time_valid || (hour < LAMP_ON_HOUR) || (hour >= LAMP_OFF_HOUR))
        {
            wanted = 0U;
        }
        else if (light_adc >= LAMP_DARK_ON_ADC)
        {
            wanted = 1U;
        }
        else if (light_adc <= LAMP_BRIGHT_OFF_ADC)
        {
            wanted = 0U;
        }
    }

    if (lamp_mode != LAMP_MODE_AUTO)
    {
        *lamp_is_on = wanted;
        *pending_state = wanted;
        *pending_since = now;
        Lamp_SetBrightness(wanted ? 100U : 0U);
        return;
    }

    if (wanted == *lamp_is_on)
    {
        *pending_state = wanted;
        *pending_since = now;
        return;
    }

    if (wanted != *pending_state)
    {
        *pending_state = wanted;
        *pending_since = now;
        return;
    }

    if ((now - *pending_since) >= LAMP_CONFIRM_MS)
    {
        *lamp_is_on = wanted;
        Lamp_SetBrightness(wanted ? 100U : 0U);
        *pending_since = now;
    }
}
#endif

#if DHT11_ENABLED
static uint32_t dwt_cycles_per_us;

static void DWT_DelayInit(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    dwt_cycles_per_us = HAL_RCC_GetHCLKFreq() / 1000000U;
}

static void DHT11_SetOutput(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = DHT11_GPIO_PIN;
    /* Open-drain is the correct electrical mode for the bidirectional
       single-wire bus used by the CJDHT11 module. */
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &gpio);
}

static void DHT11_SetInput(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = DHT11_GPIO_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &gpio);
}

static uint8_t DHT11_WaitWhile(GPIO_PinState state, uint32_t timeout_us,
                               uint32_t *elapsed_cycles)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t timeout_cycles = timeout_us * dwt_cycles_per_us;

    while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == state)
    {
        if ((uint32_t)(DWT->CYCCNT - start) >= timeout_cycles)
        {
            return 0U;
        }
    }
    if (elapsed_cycles != NULL)
    {
        *elapsed_cycles = (uint32_t)(DWT->CYCCNT - start);
    }
    return 1U;
}

static uint8_t DHT11_Read(uint8_t *temperature, uint8_t *humidity)
{
    uint8_t data[5] = {0U};
    uint8_t status = 1U;
    uint32_t high_cycles;
    uint32_t i;
    uint32_t primask;

    DHT11_SetOutput();
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_SET);
    HAL_Delay(2U);
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_RESET);
    HAL_Delay(20U);
    primask = __get_PRIMASK();
    __disable_irq();
    /* Release the bus and immediately listen. The sensor pulls DATA low
       after a 20-40 us high interval. */
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_SET);
    DHT11_SetInput();

    if (!DHT11_WaitWhile(GPIO_PIN_SET, 500U, NULL))
    {
        status = 2U;
        goto dht11_done;
    }
    if (!DHT11_WaitWhile(GPIO_PIN_RESET, 500U, NULL))
    {
        status = 3U;
        goto dht11_done;
    }
    if (!DHT11_WaitWhile(GPIO_PIN_SET, 500U, NULL))
    {
        status = 4U;
        goto dht11_done;
    }

    for (i = 0U; i < 40U; i++)
    {
        if (!DHT11_WaitWhile(GPIO_PIN_RESET, 200U, NULL))
        {
            status = 5U;
            goto dht11_done;
        }
        if (!DHT11_WaitWhile(GPIO_PIN_SET, 200U, &high_cycles))
        {
            status = 6U;
            goto dht11_done;
        }
        data[i / 8U] <<= 1U;
        if (high_cycles > (45U * dwt_cycles_per_us))
        {
            data[i / 8U] |= 1U;
        }
    }

    if ((uint8_t)(data[0] + data[1] + data[2] + data[3]) != data[4])
    {
        status = 7U;
        goto dht11_done;
    }

    *humidity = data[0];
    *temperature = data[2];

dht11_done:
    if (primask == 0U) __enable_irq();
    DHT11_SetInput();
    return status;
}
#endif

static uint16_t ADC_ReadAverage(uint32_t adc_channel, uint32_t samples)
{
    uint32_t sum = 0;
    uint32_t i;
    ADC_ChannelConfTypeDef channel = {0};

    channel.Channel = adc_channel;
    channel.Rank = ADC_REGULAR_RANK_1;
    channel.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
    if (HAL_ADC_ConfigChannel(&hadc1, &channel) != HAL_OK) Error_Handler();

    /* Discard the first conversion after channel selection so the ADC sample
       capacitor can settle to the YL-69 analog output voltage. */
    if (HAL_ADC_Start(&hadc1) != HAL_OK) Error_Handler();
    if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK) Error_Handler();
    (void)HAL_ADC_GetValue(&hadc1);
    (void)HAL_ADC_Stop(&hadc1);

    for (i = 0; i < samples; i++)
    {
        if (HAL_ADC_Start(&hadc1) != HAL_OK) Error_Handler();
        if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK) Error_Handler();
        sum += HAL_ADC_GetValue(&hadc1);
        (void)HAL_ADC_Stop(&hadc1);
    }
    return (uint16_t)(sum / samples);
}

static uint8_t Soil_ToPercent(uint16_t adc)
{
    uint32_t percent;

    if (adc <= SOIL_ADC_WET) return 100U;
    if (adc >= SOIL_ADC_DRY) return 0U;

    percent = ((uint32_t)(SOIL_ADC_DRY - adc) * 100U) /
              (SOIL_ADC_DRY - SOIL_ADC_WET);
    return (uint8_t)percent;
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

static void Error_Handler(void)
{
    __disable_irq();
    while (1) { }
}
