/**
 ****************************************************************************************
 *
 * @file user_proxr.c
 *
 * @brief Proximity reporter project source code.
 *
 * Copyright (C) 2015-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "gapc_task.h"
#include "user_periph_setup.h"
#include "wkupct_quadec.h"
#include "app_easy_msg_utils.h"
#include "gpio.h"
#include "app_security.h"
#include "user_peripheral.h"
#include "arch.h"
#include "arch_api.h"
#if defined (__DA14531__) && (defined (CFG_APP_GOTO_HIBERNATION) || defined (CFG_APP_GOTO_STATEFUL_HIBERNATION))
#include "arch_hibernation.h"
#endif
#include "app_task.h"
#include "app_env.h"

#if defined (__DA14531__)
#include "rtc.h"
#include "timer1.h"
#endif

#if (BLE_SUOTA_RECEIVER)
#include "app_suotar.h"
#endif

#if defined (CFG_SPI_FLASH_ENABLE)
#include "spi_flash.h"
#endif

static void configure_rtc_wakeup(void);
void app_meas_temp(void);

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles APP_WAKEUP_MSG sent when device exits deep sleep. Triggered by button press.
 ****************************************************************************************
*/
static void app_wakeup_cb(void)
{
    // If state is not idle, ignore the message
    if (ke_state_get(TASK_APP) == APP_CONNECTABLE)
    {
        default_advertise_operation();
    }
}

/**
 ****************************************************************************************
 * @brief Routine to resume system from sleep state.
 ****************************************************************************************
 */
static void app_resume_system_from_sleep(void)
{
#if !defined (__DA14531__)
    if (GetBits16(SYS_STAT_REG, PER_IS_DOWN))
    {
        periph_init();
    }
#endif
		
    if (arch_ble_ext_wakeup_get())
    {
        arch_set_sleep_mode(app_default_sleep_mode);
        arch_ble_force_wakeup();
        arch_ble_ext_wakeup_off();
        app_easy_wakeup();
    }
		app_meas_temp();
}

/**
 ****************************************************************************************
 * @brief Button press callback function. Registered in WKUPCT driver.
 ****************************************************************************************
 */
/*static void app_button_press_cb(void)
{
#if (BLE_PROX_REPORTER)
    if (alert_state.lvl != PROXR_ALERT_NONE)
    {
        app_env_alert_stop();
    }
#endif
    app_resume_system_from_sleep();

    app_button_enable();
}*/
/*
void app_button_enable(void)
{
    app_easy_wakeup_set(app_wakeup_cb);
    wkupct_register_callback(app_button_press_cb);

    if (GPIO_GetPinStatus(GPIO_BUTTON_PORT, GPIO_BUTTON_PIN))
    {
        wkupct_enable_irq(WKUPCT_PIN_SELECT(GPIO_BUTTON_PORT, GPIO_BUTTON_PIN), // select pin (GPIO_BUTTON_PORT, GPIO_BUTTON_PIN)
                          WKUPCT_PIN_POLARITY(GPIO_BUTTON_PORT, GPIO_BUTTON_PIN, WKUPCT_PIN_POLARITY_LOW), // polarity low
                          1, // 1 event
                          0); // debouncing time = 0
    }
}
*/
#if (BLE_SUOTA_RECEIVER)
void on_suotar_status_change(const uint8_t suotar_event)
{
#if (!SUOTAR_SPI_DISABLE)
    uint8_t dev_id;

    // Release the SPI flash memory from power down
    spi_flash_release_from_power_down();

    // Disable the SPI flash memory protection (unprotect all sectors)
    spi_flash_configure_memory_protection(SPI_FLASH_MEM_PROT_NONE);

    // Try to auto-detect the SPI flash memory
    spi_flash_auto_detect(&dev_id);

    if (suotar_event == SUOTAR_END)
    {
        // Power down the SPI flash memory
        spi_flash_power_down();
    }
#endif
}
#endif

void user_app_on_disconnect(struct gapc_disconnect_ind const *param)
{
    default_app_on_disconnect(NULL);

}

void user_app_on_connection(uint8_t conidx, struct gapc_connection_req_ind const *param)
{
    
		// set timer for next wakeup
		//configure_rtc_wakeup();
	
		default_app_on_connection(conidx, param);
}

void user_app_on_set_dev_config_complete(void)
{
    default_app_on_set_dev_config_complete();
}

void user_app_on_init()
{
  app_meas_temp();  
	default_app_on_init();

}



#if defined (CFG_EXT_SLEEP_WAKEUP_RTC) || defined (CFG_DEEP_SLEEP_WAKEUP_RTC)
/**
 ****************************************************************************************
 * @brief RTC interrupt handler routine for wakeup.
 ****************************************************************************************
*/

static void rtc_interrupt_hdlr(uint8_t event)
{
#if defined (CFG_EXT_SLEEP_WAKEUP_RTC)
    app_resume_system_from_sleep();
#endif
}

/**
 ****************************************************************************************
 * @brief Configure RTC to generate an interrupt after 10 seconds.
 ****************************************************************************************
*/
static void configure_rtc_wakeup(void)
{
    rtc_time_t alarm_time;

    // Init RTC
    rtc_reset();

    // Configure the RTC clock; RCX is the RTC clock source (14420 Hz)
    rtc_clk_config(RTC_DIV_DENOM_1000, 14420);
    rtc_clock_enable();

    rtc_config_t cfg = {.hour_clk_mode = RTC_HOUR_MODE_24H, .keep_rtc = 0};

    rtc_time_t time = {.hour_mode = RTC_HOUR_MODE_24H, .pm_flag = 0, .hour = 11,
                       .minute = 55, .sec = 30, .hsec = 00};

    // Alarm interrupt in ten seconds
    alarm_time = time;
    alarm_time.sec += 10;

    // Initialize RTC, set time and data, register interrupt handler callback function and enable seconds interrupt
    rtc_init(&cfg);

    // Start RTC
    rtc_set_time_clndr(&time, NULL);
    rtc_set_alarm(&alarm_time, NULL, RTC_ALARM_EN_SEC);

    // Clear pending interrupts
    rtc_get_event_flags();
    rtc_register_intr(rtc_interrupt_hdlr, RTC_INTR_ALRM);
#if defined (CFG_EXT_SLEEP_WAKEUP_RTC)
    app_easy_wakeup_set(app_wakeup_cb);
#endif
}
#endif


#if defined (CFG_APP_GOTO_DEEP_SLEEP)
/**
 ****************************************************************************************
 * @brief Put the system into deep sleep mode. It demonstrates the deep sleep mode usage
 *        and how the system can wake up from it. The exit from the deep sleep state causes 
 *        a system reboot.
 * @note  The system can wake up from deep sleep by:
 *          - external wake up interrupt, caused e.g. by button press (properly configured GPIO pin)
 *          - power on reset, caused e.g. by button press (properly configured GPIO pin)
 *          - interrupt generated from RTC
 *          - interrupt generated from Timer1
 *        When the system exits deep sleep state, the boot process is triggered.
 *        The application code has to be programmed in an external memory resource or
 *        in the OTP memory, in order for the system to reboot properly.
 ****************************************************************************************
*/
static void put_system_into_deep_sleep(void)
{
#if defined (CFG_DEEP_SLEEP_WAKEUP_RTC)
    configure_rtc_wakeup();
#endif


    // Go to deep sleep
    arch_set_deep_sleep(CFG_DEEP_SLEEP_RAM1,
                        CFG_DEEP_SLEEP_RAM2,
                        CFG_DEEP_SLEEP_RAM3,
                        CFG_DEEP_SLEEP_PAD_LATCH_EN);
}
#endif // (CFG_APP_GOTO_DEEP_SLEEP)


void app_advertise_complete(const uint8_t status)
{
    if ((status == GAP_ERR_NO_ERROR) || (status == GAP_ERR_CANCELED))
    {

			// restart adv
    }

    if (status == GAP_ERR_CANCELED)
    {

#if defined (__DA14531__)
    // Configure PD_TIM
#if defined (CFG_EXT_SLEEP_WAKEUP_RTC) || defined (CFG_EXT_SLEEP_WAKEUP_TIMER1) || \
    defined (CFG_DEEP_SLEEP_WAKEUP_RTC) || defined (CFG_DEEP_SLEEP_WAKEUP_TIMER1)
        // Ensure PD_TIM is open
        SetBits16(PMU_CTRL_REG, TIM_SLEEP, 0);
        // Wait until PD_TIM is opened
        while ((GetWord16(SYS_STAT_REG) & TIM_IS_UP) != TIM_IS_UP);
#else
        // Close PD_TIM
        SetBits16(PMU_CTRL_REG, TIM_SLEEP, 1);
        // Wait until PD_TIM is closed
        while ((GetWord16(SYS_STAT_REG) & TIM_IS_DOWN) != TIM_IS_DOWN);
#endif
#endif
				spi_flash_power_down();
        put_system_into_deep_sleep();

		}

}

/// @} APP
