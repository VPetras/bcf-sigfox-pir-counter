#include <application.h>

#define SIGFOX_REPORT_SECONDS (15 * 60)
#define SENSOR_DATA_STREAM_SAMPLES 15
#define SENSOR_UPDATE_INTERVAL (1 * 60 * 1000)

BC_DATA_STREAM_FLOAT_BUFFER(sm_buffer_thermometer, SENSOR_DATA_STREAM_SAMPLES)
BC_DATA_STREAM_FLOAT_BUFFER(sm_voltage_buffer, 8)
bc_data_stream_t sm_thermometer;
bc_data_stream_t sm_voltage;

bc_led_t led;
bc_button_t button;
bc_tmp112_t tmp112;
bc_module_pir_t pir;
bc_module_sigfox_t sigfox_module;

uint16_t pir_count = 0;

void tmp112_event_handler(bc_tmp112_t *self, bc_tmp112_event_t event, void *event_param)
{
    if (event == BC_TMP112_EVENT_UPDATE)
    {
        float temperature;

        if (bc_tmp112_get_temperature_celsius(&tmp112, &temperature))
        {
            bc_data_stream_feed(&sm_thermometer, &temperature);
        }
        else
        {
            bc_data_stream_reset(&sm_thermometer);
        }
    }
}

void battery_event_handler(bc_module_battery_event_t event, void *event_param)
{
    if (event == BC_MODULE_BATTERY_EVENT_UPDATE)
    {
        float voltage = NAN;

        bc_module_battery_get_voltage(&voltage);

        bc_data_stream_feed(&sm_voltage, &voltage);
    }
}

void battery_measure_task(void *param)
{
    if (!bc_module_battery_measure())
    {
        bc_scheduler_plan_current_now();
    }
}

void sigfox_module_event_handler(bc_module_sigfox_t *self, bc_module_sigfox_event_t event, void *event_param)
{
    if (event == BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_START)
    {
        bc_led_set_mode(&led, BC_LED_MODE_ON);
    }
    else if (event == BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE)
    {
        bc_led_set_mode(&led, BC_LED_MODE_OFF);
    }
    else if (event == BC_MODULE_SIGFOX_EVENT_ERROR)
    {
        bc_led_set_mode(&led, BC_LED_MODE_BLINK);
    }
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    if (event == BC_BUTTON_EVENT_CLICK)
    {
        bc_scheduler_plan_now(0);
    }
}

void pir_event_handler(bc_module_pir_t *self, bc_module_pir_event_t event, void *event_param)
{
    if (event == BC_MODULE_PIR_EVENT_MOTION)
    {
        if (pir_count < 0xffff)
        {
            pir_count++;
        }
    }
}

void application_init(void)
{
    bc_data_stream_init(&sm_thermometer, SENSOR_DATA_STREAM_SAMPLES, &sm_buffer_thermometer);
    bc_data_stream_init(&sm_voltage, 1, &sm_voltage_buffer);

    bc_module_pir_init(&pir);
    bc_module_pir_set_sensitivity(&pir, BC_MODULE_PIR_SENSITIVITY_MEDIUM);
    bc_module_pir_set_event_handler(&pir, pir_event_handler, NULL);

    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_FLASH);

    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    bc_tmp112_init(&tmp112, BC_I2C_I2C0, 0x49);
    bc_tmp112_set_event_handler(&tmp112, tmp112_event_handler, NULL);
    bc_tmp112_set_update_interval(&tmp112, SENSOR_UPDATE_INTERVAL);

    bc_module_sigfox_init(&sigfox_module, BC_MODULE_SIGFOX_REVISION_R2);
    bc_module_sigfox_set_event_handler(&sigfox_module, sigfox_module_event_handler, NULL);

    bc_scheduler_plan_absolute(0, 10 * 1000);
}

void application_task(void)
{
    static uint8_t buffer[5];

    memset(buffer, 0xff, sizeof(buffer));
    
    float voltage_avg = NAN;

    bc_data_stream_get_average(&sm_voltage, &voltage_avg);

    if (!isnan(voltage_avg))
    {
        buffer[0] = ceil(voltage_avg * 10.f);
    }
    float temperature_avg = NAN;

    bc_data_stream_get_average(&sm_thermometer, &temperature_avg);

    if (!isnan(temperature_avg))
    {
        int16_t temperature_i16 = (int16_t) (temperature_avg * 10.f);

        buffer[1] = temperature_i16;
        buffer[2] = temperature_i16 >> 8;
    }

    buffer[3] = pir_count;
    buffer[4] = pir_count >> 8;

    if (bc_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer)))
    {
        pir_count = 0;
        bc_scheduler_plan_current_relative(SIGFOX_REPORT_SECONDS * 1000);
    }
    else
    {
        bc_scheduler_plan_current_relative(1000);
    }
}
