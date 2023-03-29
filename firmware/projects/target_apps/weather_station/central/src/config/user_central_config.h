#ifndef _USER_CENTRAL_CONFIG_H_
#define _USER_CENTRAL_CONFIG_H_

#define SCAN_FILTER_NONE									(0)
#define SCAN_FILTER_NAME									(1)
#define SCAN_FILTER_16_BIT_SVC_DATA				(2)
#define SCAN_FILTER_TX_POWER							(3)
#define SCAN_FILTER_MFG_DATA							(4)

#define	SCAN_FILTER												(SCAN_FILTER_NAME)

#define CONNECT_TO_PERIPHERAL							(1)

#if CONNECT_TO_PERIPHERAL == 1
	#if SCAN_FILTER == SCAN_FILTER_NAME
		#define PERIPH_MATCH_DATA			"DLG-PROXR"
		#define PERIPH_MATCH_DATA_LEN	(sizeof(PERIPH_MATCH_DATA))
	#else
		#undef CONNECT_TO_PERIPHERAL
	#endif
		
	#define ENABLE_BAS
		
	#undef ENABLE_IAS
		
	#define CONNECTION_TIMEOUT_10MS			(500) //5 seconds

#endif 
		
		
#ifdef APP_EASY_MAX_ACTIVE_CONNECTION
#undef APP_EASY_MAX_ACTIVE_CONNECTION
#define APP_EASY_MAX_ACTIVE_CONNECTION      (CFG_MAX_CONNECTIONS)
#endif

#endif //_USER_CENTRAL_CONFIG_H_
		

