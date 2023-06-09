/**
 ****************************************************************************************
 *
 * @file user_central_app.c
 *
 * @brief Cental application project source code.
 *
 * Copyright (c) 2012-2020 Dialog Semiconductor. All rights reserved.
 *
 * This software ("Software") is owned by Dialog Semiconductor.
 *
 * By using this Software you agree that Dialog Semiconductor retains all
 * intellectual property and proprietary rights in and to this Software and any
 * use, reproduction, disclosure or distribution of the Software without express
 * written permission or a license agreement from Dialog Semiconductor is
 * strictly prohibited. This Software is solely for use on or in conjunction
 * with Dialog Semiconductor products.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE
 * SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. EXCEPT AS OTHERWISE
 * PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * DIALOG SEMICONDUCTOR BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */
#include "rwip_config.h"             // SW configuration


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app_api.h"
#include "user_central_app.h"
#include "user_ble_gap.h"
#include "user_ble_common.h"
#include "user_ble_gatt.h"
#include "user_config.h"
#include "user_central_config.h"
#include "gap.h"
#include "ke_task.h"
#include "wkupct_quadec.h"
#include "user_periph_setup.h"
#include "spi_flash.h"
//#include "EventRecorder.h"
#include "bme680_task.h"


#define SCAN_INTVL_MS		(50)
#define SCAN_WINDOW_MS	(50)



typedef struct
{
	struct bd_addr addr;
}connecting_device_t;



typedef struct
{
	periph_device_t 	periph_devices[CFG_MAX_CONNECTIONS];
	
	bool 							connect_to_periph;
	struct bd_addr 		connect_to_addr;
	uint8_t 					connect_to_addr_type;
	uint8_t						num_connections;
	timer_hnd					connection_timer;
	
}central_app_env_t;

central_app_env_t central_app_env;
void read_temp(void);

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/


static inline void format_adv_string(uint8_t * data, uint8_t len, char *out_string)
{
	memcpy(out_string, data, len);
	out_string[len] = '\0';
	
}

/* return true if UUIDs match */
static bool match_uuid(uint8_t * uuid1, uint8_t *uuid2, uint8_t len)
{
	return !memcmp(uuid1, uuid2, len);
}

#if (SCAN_FILTER == SCAN_FILTER_NONE || SCAN_FILTER == SCAN_FILTER_16_BIT_SVC_DATA || SCAN_FILTER == SCAN_FILTER_MFG_DATA)

/* return static buffer with formatted hex string */
static char *format_hex_string(uint8_t *data, uint8_t len)	
{
	
	static char buf[61];
	uint8_t i;
	for(i = 0; i < len; i++)
	{
		sprintf(&buf[i*2], "%02x", data[i]);
	}
	
	buf[(i+1)*2+1] = '\0';
	
	return buf;
	
}

#endif
	/* return static buffer with formatted address */
static const char *format_bd_address(const struct bd_addr *addr)
{
        static char buf[19];
        int i;

        for (i = 0; i < sizeof(addr->addr); i++) {
                int idx;

                // for printout, address should be reversed
                idx = sizeof(addr->addr) - i - 1;
                sprintf(&buf[i * 3], "%02x:", addr->addr[idx]);
        }

        buf[sizeof(buf) - 2] = '\0';

        return buf;
}
/* return static buffer with formatted UUID */
static const char *format_uuid(att_uuid_t *uuid)
{
        static char buf[37];

        if (uuid->type == ATT_UUID_16) {
                sprintf(buf, "0x%04x", uuid->uuid.uuid16);
        } else {
                int i;
                int idx = 0;

                for (i = ATT_UUID_LENGTH; i > 0; i--) {
                        if (i == 12 || i == 10 || i == 8 || i == 6) {
                                buf[idx++] = '-';
                        }

                        idx += sprintf(&buf[idx], "%02x", uuid->uuid.uuid128[i - 1]);
                }
        }

        return buf;
}

/* return static buffer with characteristics properties mask */
static const char *format_properties(uint8_t properties)
{
        static const char props_str[] = "BRXWNISE"; // each letter corresponds to single property
        static char buf[9];
        int i;

        // copy full properties mask
        memcpy(buf, props_str, sizeof(props_str));

        for (i = 0; i < 8; i++) {
                // clear letter from mask if property not present
                if ((properties & (1 << i)) == 0) {
                        buf[i] = '-';
                }
        }

        return buf;
}


void main_routine(){
		float sensor_data[4];
		
		// read temperature from peripheral
		read_temp();
		// Turn on power switch
		GPIO_SetActive(PWR_SW_PORT, PWR_SW_PIN);
		delay_usec(500000, NULL);
		// Measure temperature, humidity, pressure, and gas
		uint8_t result = read_sensor(sensor_data);
		if(result == 0){
			// Update display with all new values
		}
		else{
			//
		}

		
	// Turn off power switch
		GPIO_SetInactive(PWR_SW_PORT, PWR_SW_PIN);
		// Go to sleep
}
/**
 ****************************************************************************************
 * @brief callback from alert button
 * @return void
 ****************************************************************************************
 */
void read_temp()
{
	user_gatt_read_simple(central_app_env.periph_devices[0].con_idx, central_app_env.periph_devices[0].serv_disc.temp_char.handle);
	printf("Trying to read temperature\n");		
}
		


/**
 ****************************************************************************************
 * @brief After full discovery finished
 * @param[in] con_idx - connection identifier
 * @return void
 ****************************************************************************************
 */
static void handle_service_disc_finished(uint8_t con_idx)
{
	// start main routine
	main_routine();
	//app_button_press_cb();
	
}

/**
 ****************************************************************************************
 * @brief Scan for devices
 * @return void
 ****************************************************************************************
 */
static void ble_scan_for_devices()
{
	
	ke_state_set(TASK_APP, APP_CONNECTABLE);
	ble_gap_error_t err = user_ble_gap_start_scan(true, SCAN_INTVL_MS, SCAN_WINDOW_MS, false);
	ASSERT_ERROR(err == BLE_GAP_ERROR_NO_ERROR);
}

/**
 ****************************************************************************************
 * @brief Callback from SDK when disconnect occurs.
 * @param[in] report  advertising report information
 * @return void
 ****************************************************************************************
 */
void user_on_adv_report_ind(struct gapm_adv_report_ind const * adv_ind)
{
	bool dev_found = false;
	
#if CONNECT_TO_PERIPHERAL == 1
	bool conn_to_device = false;
	struct bd_addr dev_addr = adv_ind->report.adv_addr;
#endif 

	uint8_t num_ad_elements = user_ble_gap_get_adv_num_elements(adv_ind->report.data, 
																																		adv_ind->report.data_len);
	ble_gap_adv_struct_t adv_data_structs[num_ad_elements]; 
	
	user_ble_gap_parse_adv_data(adv_ind->report.data_len, adv_ind->report.data, adv_data_structs);
	
	uint8_t i;
	for(i = 0 ; i < num_ad_elements; i++)
	{
		
		switch(adv_data_structs[i].type)
		{
#if SCAN_FILTER == SCAN_FILTER_NONE
			case GAP_AD_TYPE_FLAGS:
			{
				dev_found = true;
				printf("GAP FLAGS: %s\r\n", 
													format_hex_string(adv_data_structs[i].data, adv_data_structs[i].len) );
				break;
			}
#endif 
#if (SCAN_FILTER == SCAN_FILTER_NONE) 
			case GAP_AD_TYPE_MORE_16_BIT_UUID:
			{
				dev_found = true;
				printf("INCOMP LIST 16-BIT SVC: %s\r\n", 
													format_hex_string(adv_data_structs[i].data, adv_data_structs[i].len) );
				break;
			}
#endif
#if (SCAN_FILTER == (SCAN_FILTER_NAME) || SCAN_FILTER == SCAN_FILTER_NONE) 

			case GAP_AD_TYPE_COMPLETE_NAME:
			{
				dev_found = true;
				
				char local_name[adv_data_structs[i].len + 1];
				format_adv_string(adv_data_structs[i].data,adv_data_structs[i].len, local_name);
				printf("Device Local Name: %s\r\n", local_name);
	#if CONNECT_TO_PERIPHERAL == 1
				conn_to_device = memcmp(PERIPH_MATCH_DATA, adv_data_structs[i].data, PERIPH_MATCH_DATA_LEN) ? false : true;
	#endif 
				break;
			}
#endif 
#if SCAN_FILTER == SCAN_FILTER_NONE
			
			case GAP_AD_TYPE_TRANSMIT_POWER:
			{
				dev_found = true;
				
				int8_t power;
				memcpy(&power, adv_data_structs[i].data, 1); 
				printf("TX_POWER: %i dBm\r\n", power);
													
				break;
			}
#endif
#if (SCAN_FILTER == SCAN_FILTER_16_BIT_SVC_DATA || SCAN_FILTER == SCAN_FILTER_NONE) 
			case GAP_AD_TYPE_SERVICE_16_BIT_DATA:
			{
				dev_found = true;
				
				uint16_t UUID;
				memcpy(&UUID, adv_data_structs[i].data, sizeof(uint16_t));
				printf("GAP_TYPE: SVC_DATA_16_BIT: UUID:%04X DATA:%s\r\n", 
														UUID, format_hex_string(adv_data_structs[i].data+2, adv_data_structs[i].len-2) );
				break;
			}
#endif 
#if (SCAN_FILTER == SCAN_FILTER_MFG_DATA || SCAN_FILTER == SCAN_FILTER_NONE) 
			case GAP_AD_TYPE_MANU_SPECIFIC_DATA:
			{
				dev_found = true;
				printf("MFG_SPECIFIC_DATA: %s\r\n", 
													format_hex_string(adv_data_structs[i].data, adv_data_structs[i].len) );
				break;
			}
#endif 
			default:
			{
#if SCAN_FILTER == SCAN_FILTER_NONE
				dev_found = true;
				
				printf("GAP Type: %02x, Data: %s\r\n", adv_data_structs[i].type, 
												format_hex_string(adv_data_structs[i].data, adv_data_structs[i].len) );
#endif
				break;
			}
			
		}
		
	}
	
	if(dev_found){
		uint8_t rssi_abs = 256 - adv_ind->report.rssi;
		printf("RSSI: -%d\r\n", rssi_abs);
		printf("BD_ADDR:%s \r\n---------------END_ADV-----------\r\n", 
																						format_bd_address(&adv_ind->report.adv_addr) );
	}

	#if CONNECT_TO_PERIPHERAL
	if(conn_to_device)
	{
		printf("Connecting to Device...\r\n");
		central_app_env.connect_to_periph = true;
		memcpy(&central_app_env.connect_to_addr, &dev_addr, sizeof(struct bd_addr));
		central_app_env.connect_to_addr_type = adv_ind->report.adv_addr_type;
		
		user_ble_gap_stop_scan();
		
	}
	#endif

	
	

}

/**
 ****************************************************************************************
 * @brief Callback from SDK when  connection occurs.
 * @param[in] connection_idx  connection identifier
 * @param[in] param  peer connection info
 * @return void
 ****************************************************************************************
 */
void user_on_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
		if(central_app_env.connection_timer != EASY_TIMER_INVALID_TIMER){
			app_easy_timer_cancel(central_app_env.connection_timer);
		}
	
    default_app_on_connection(connection_idx, param);
		printf("%s\r\n", __func__);
		
		central_app_env.periph_devices[connection_idx].addr = param->peer_addr;
		central_app_env.periph_devices[connection_idx].con_idx = connection_idx;
		central_app_env.periph_devices[connection_idx].con_valid = true;
		central_app_env.num_connections++;
	
		if(central_app_env.num_connections < CFG_MAX_CONNECTIONS)
		{
			ble_scan_for_devices();
		}

#ifdef ENABLE_IAS		
		central_app_env.periph_devices[connection_idx].serv_disc.ias_handle_valid = false;
#endif
#ifdef ENABLE_BAS
		central_app_env.periph_devices[connection_idx].serv_disc.bas_handle_valid = false;
#endif
		
		user_gatt_discover_all_services(connection_idx, 1);
}

/**
 ****************************************************************************************
 * @brief Callback from SDK when disconnect occurs.
 * @param[in] param  peer connection info
 * @return void
 ****************************************************************************************
 */
void user_on_disconnect( struct gapc_disconnect_ind const *param )
{
    default_app_on_disconnect(param);
		central_app_env.periph_devices[param->conhdl].con_valid = false;
	
		if(central_app_env.connection_timer != EASY_TIMER_INVALID_TIMER){
			app_easy_timer_cancel(central_app_env.connection_timer);
		}
		if( central_app_env.num_connections == CFG_MAX_CONNECTIONS)
		{
			ble_scan_for_devices();
		}
		central_app_env.num_connections--;
		
		printf("%s: reason:%02x\r\n", __func__, param->reason);
#ifdef ENABLE_IAS		
		if(central_app_env.num_connections < 1)
		{
			wkupct_disable_irq();
		}
#endif
		
		
}

/**
 ****************************************************************************************
 * @brief Callback from SDK when disconnect occurs.
 * @return void
 ****************************************************************************************
 */
static void connection_timeout_cb(){
	
	central_app_env.connection_timer = EASY_TIMER_INVALID_TIMER;
	user_ble_cancel_connect();
}

/**
 ****************************************************************************************
 * @brief Callback from SDK when disconnect occurs.
 * @param[in] reason reason for disconnect BT SIG reason defined in co_error.h
 * @return void
 ****************************************************************************************
 */
void user_on_scanning_completed(uint8_t reason)
{
	
	if(reason == GAP_ERR_CANCELED)
	{
		user_ble_gap_connect(central_app_env.connect_to_addr.addr, central_app_env.connect_to_addr_type);
		central_app_env.connection_timer = app_easy_timer(CONNECTION_TIMEOUT_10MS, connection_timeout_cb);
	}
	else{
			
		printf("%s: ERR: reason: %d\r\n", __func__, reason);
	}
	
	
	
}

/**
 ****************************************************************************************
 * @brief Callback from SDK once stack is configured
 * @return void
 ****************************************************************************************
 */
void user_app_on_set_dev_config_complete(void)
{
		//EventRecorderInitialize(EventRecordAll, 1);
		main_routine();
		/*Advertising starts within this default handler, but we have made the callback NULL to avoid advertising. 
		* alternate method, would be to copy the default handler code into here, removing the advertising call
		*/
		default_app_on_set_dev_config_complete();
	
		ble_scan_for_devices();
	
		spi_flash_power_down(); //power down flash before entering sleep
}


static void handle_svc_ind(uint8_t con_idx, struct gattc_sdp_svc_ind const *disc_svc)
{
	service_info_t *svc = ke_malloc(user_ble_get_svc_size(disc_svc), KE_MEM_NON_RETENTION);
	printf("%s: conn_idx=%04x start_h=%04x end_h=%04x\r\n", __func__, con_idx,
                                                                        disc_svc->start_hdl, disc_svc->end_hdl);

	user_gatt_parse_service(disc_svc, svc, con_idx);
	
	uint16_t i;
	
#ifdef ENABLE_IAS
	bool ias_uuid_match;
	uint16_t ias_uuid = ATT_SVC_IMMEDIATE_ALERT;
	uint16_t ias_alert_uuid = ATT_CHAR_ALERT_LEVEL;
	if(svc->svc_uuid.type == ATT_UUID_16){
		ias_uuid_match = match_uuid((uint8_t *)&ias_uuid, (uint8_t *)&svc->svc_uuid.uuid.uuid16, sizeof(uint16_t));
	}
	
	if(ias_uuid_match){
		printf("Immediate Alert Service: ");
	}
#endif 
	
#ifdef ENABLE_BAS
	bool temp_uuid_match = false;
	uint16_t env_uuid = ATT_UUID_16(0x181A);
	uint16_t temp_uuid= ATT_UUID_16(0x2A1C);
	printf("UUID: %x\n", svc->svc_uuid.uuid.uuid16);
	
	if(svc->svc_uuid.type == ATT_UUID_16){
		temp_uuid_match = match_uuid((uint8_t *)&env_uuid, (uint8_t *)&svc->svc_uuid.uuid.uuid16, sizeof(uint16_t));
	}

#endif
	printf("%s: \r\n", format_uuid(&svc->svc_uuid) );
	for(i = 0 ; i<svc->num_chars; i++)
	{
		gattc_chars_t *gatt_char = &svc->items[i];

		if(temp_uuid_match == 1)
		{
			if(match_uuid( (uint8_t *)&temp_uuid , (uint8_t *)&gatt_char->uuid.uuid, 2) )
			{
				memcpy(&central_app_env.periph_devices[con_idx].serv_disc.temp_char, 
																										gatt_char, sizeof(gattc_chars_t));
				
				central_app_env.periph_devices[con_idx].serv_disc.temp_handle_valid = true;
				printf("Temperature Char\r\n");
			}
		}

		printf("\t%04x char %s prop=%02x (%s)\r\n", gatt_char->c.value_handle,
                                                format_uuid(&gatt_char->uuid), gatt_char->c.properties,
                                                format_properties(gatt_char->c.properties));	
	}
	
	ke_free(svc);
	
	central_app_env.periph_devices[con_idx].serv_disc.last_handle = disc_svc->end_hdl;
	
	
}

/**
 ****************************************************************************************
 * @brief Callback from SDK when an unhandled stack event occurs
 * @param[in] msgid ID of message from stack
 * @param[in] param - message data
 * @param[in] dest_id - destination task of message, e.g. TASK_APP
 * @param[in] src_id - task originator of message
 * @return void
 ****************************************************************************************
 */
void user_catch_rest_hndl(ke_msg_id_t const msgid,
                          void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
	
	printf("MESSAGE ID: %d\n", msgid);
	uint8_t conn_idx = KE_IDX_GET(src_id);
	switch(msgid)
	{
		
		case GATTC_CMP_EVT:
		{
			
			struct gattc_cmp_evt const* evt = (struct gattc_cmp_evt const *)(param);
			switch(evt->operation)
			{
				case GATTC_SDP_DISC_SVC_ALL:
				{
					
					printf("Completion Event: GATTC_DISC_ALL_SVC: status: %04X\r\n", evt->status);
					if(evt->status != CO_ERROR_NO_ERROR){

							user_gatt_discover_all_services(conn_idx, central_app_env.periph_devices[conn_idx].serv_disc.last_handle);
					}else
					{
							handle_service_disc_finished(conn_idx);
					}
					
					
				}break;
				case GATTC_WRITE:
				{
					printf("Completion Event: GATTC_WRITE\r\n");
					break;
				}
				case GATTC_READ:
					printf("GATTC_READ: status: %04x\r\n", evt->status);
					break;
				
				
				default:
					break;
			}
			
		}break;
				
		case GATTC_EVENT_IND:
		{
			struct gattc_event_ind const *ind = (struct gattc_event_ind const *)param;
#ifdef ENABLE_BAS
			if(ind->handle == central_app_env.periph_devices[conn_idx].serv_disc.bas_char.c.value_handle)
			{
				printf("Battery Level Changed: %d \r\n", ind->value[0]);		
				printf("Battery Level Changed: %d \r\n", ind->value[1]);
			}
#endif		
		}break;
		case GAPM_CMP_EVT:
		{
			struct gapm_cmp_evt const* evt = (struct gapm_cmp_evt const *)(param);
			
			if(evt->operation == GAPM_CANCEL)
			{
				printf("Connection Timeout\r\n");
				ble_scan_for_devices();
			}
			break;
			
		}
		case GATTC_SDP_SVC_IND:
		{
			struct gattc_sdp_svc_ind const *disc_svc = (struct gattc_sdp_svc_ind const *)(param);
			
			uint8_t con_idx = KE_IDX_GET(src_id);
			handle_svc_ind(con_idx, disc_svc);
		}break;
		
		case GATTC_READ_IND:
		{
			struct gattc_read_ind const *ind = (struct gattc_read_ind const*)param;
			if(ind->handle == central_app_env.periph_devices[conn_idx].serv_disc.temp_char.handle)
			{
				printf("Temperature Read: %d \r\n", ind->value[0]);
				printf("Temperature Read: %d \r\n", ind->value[1]);
			}
		}break;
		//case 3084:
		
		default:
			break;
		
	}
	
	
}
 

/// @} APP
