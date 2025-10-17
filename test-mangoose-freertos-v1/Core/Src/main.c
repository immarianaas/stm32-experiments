/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "lwip.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

//#include "mongoose.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "usbd_cdc_if.h"
#include <stdarg.h>
#include "lwip/apps/mdns.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define LOG2(msg)  CDC_Transmit_FS((uint8_t *)(msg), strlen(msg))

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart3;

osThreadId MongooseTaskHandle;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_RTC_Init(void);
void StartMongooseTask(void const *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void usb_printf(const char *fmt, ...) {
	char buffer[256];  // adjust size if needed
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	uint8_t result;
	do {
		result = CDC_Transmit_FS((uint8_t*) buffer, strlen(buffer));
		if (result == USBD_BUSY) {
			osDelay(1); // wait a millisecond and retry
		}
	} while (result == USBD_BUSY);
}

#define LOG(msg)  usb_printf("%s\r\n", msg);
//
//static void http_handler(struct mg_connection *c, int ev, void *ev_data) {
//	if (ev == MG_EV_HTTP_MSG) {
//		struct mg_http_message *hm = (struct mg_http_message*) ev_data;
//
//		// Convert URI to C string (not null-terminated)
//		char path[64];
//		int len =
//				(hm->uri.len < sizeof(path) - 1) ?
//						hm->uri.len : sizeof(path) - 1;
//		memcpy(path, hm->uri.buf, len);
//		path[len] = '\0';
//
//		// Simple routing
//		if (strcmp(path, "/") == 0) {
//			mg_http_reply(c, 200, "Content-Type: text/plain\r\n",
//					"Home page!\n");
//		} else if (strcmp(path, "/status") == 0) {
//			mg_http_reply(c, 200, "Content-Type: application/json\r\n",
//					"{\"status\":\"OK\"}\n");
//		} else if (strcmp(path, "/toggle") == 0) {
//			mg_http_reply(c, 200, "Content-Type: text/plain\r\n",
//					"Toggled LED\n");
//		} else {
//			mg_http_reply(c, 404, "Content-Type: text/plain\r\n",
//					"Not found\n");
//		}
//	}
//	if (ev == MG_EV_ERROR) {
//		char *err_str = (char*) ev_data;
//		if (err_str) {
//			usb_printf("HTTP ERROR: %s\r\n", err_str);
//		} else {
//			usb_printf("HTTP ERROR: Unknown\r\n");
//		}
//	}
//
//	char buf[64];
//	sprintf(buf, "[event %d]\r\n", ev);
//	CDC_Transmit_FS((uint8_t*) buf, strlen(buf));
//
//	struct dhcp *d = netif_dhcp_data(netif_default);
//	usb_printf("DHCP state=%d\r\n", d ? d->state : -1);
//
//	LOG("[http handler]");
//
//// CDC_Transmit_FS((uint8_t *)"[http_handler]\r\n", strlen("[http_handler]\r\n"));
//
//}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART3_UART_Init();
	MX_RTC_Init();
	/* USER CODE BEGIN 2 */

	/* USER CODE END 2 */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of MongooseTask */
	osThreadDef(MongooseTask, StartMongooseTask, osPriorityNormal, 0, 1024);
	MongooseTaskHandle = osThreadCreate(osThread(MongooseTask), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI
			| RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 96;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init(void) {

	/* USER CODE BEGIN RTC_Init 0 */

	/* USER CODE END RTC_Init 0 */

	/* USER CODE BEGIN RTC_Init 1 */

	/* USER CODE END RTC_Init 1 */

	/** Initialize RTC Only
	 */
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	if (HAL_RTC_Init(&hrtc) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN RTC_Init 2 */

	/* USER CODE END RTC_Init 2 */

}

/**
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init(void) {

	/* USER CODE BEGIN USART3_Init 0 */

	/* USER CODE END USART3_Init 0 */

	/* USER CODE BEGIN USART3_Init 1 */

	/* USER CODE END USART3_Init 1 */
	huart3.Instance = USART3;
	huart3.Init.BaudRate = 115200;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART3_Init 2 */

	/* USER CODE END USART3_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LD1_Pin | LD3_Pin | LD2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin : USER_Btn_Pin */
	GPIO_InitStruct.Pin = USER_Btn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
	GPIO_InitStruct.Pin = LD1_Pin | LD3_Pin | LD2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_PowerSwitchOn_Pin */
	GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_OverCurrent_Pin */
	GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartMongooseTask */
/**
 * @brief  Function implementing the MongooseTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartMongooseTask */
void StartMongooseTask(void const *argument) {
	/* init code for LWIP */
	MX_LWIP_Init();

	/* init code for USB_DEVICE */
	MX_USB_DEVICE_Init();
	/* USER CODE BEGIN 5 */
	/* Infinite loop */

	/* USER CODE BEGIN 5 */
	// wait for LWIP to be on (probably should be changed..?)
	// osDelay(1000);
	while (netif_default == NULL || !netif_is_up(netif_default)
			|| !ip4_addr_isany_val(*netif_ip4_addr(netif_default))
	// || netif_is_link_up(netif_list) == 0
	) {
		osDelay(100);
		// LOG("inside while");
	}

//	char buf[64];
//	ip4_addr_t ip = *netif_ip4_addr(netif_default);
//	sprintf(buf, "IP acquired: %u.%u.%u.%u", ip4_addr1(&ip), ip4_addr2(&ip),
//			ip4_addr3(&ip), ip4_addr4(&ip));
//	LOG(buf);

	char buf[64];
	ip4_addr_t ip = *netif_ip4_addr(netif_default);
	sprintf(buf, "IP acquired: %u.%u.%u.%u", ip4_addr1(&ip), ip4_addr2(&ip),
			ip4_addr3(&ip), ip4_addr4(&ip));
	LOG(buf);

	extern struct dhcp_state_enum_t;

	struct dhcp *dhcp_client = netif_dhcp_data(netif_default);
	while (dhcp_client == NULL || dhcp_client->state != 10) {
		osDelay(100);
		// safe to read IP
	}

	LOG("OUTSIDE");

//  printf(netif_default);
	// printf("IP: %s\r\n", ipaddr_ntoa(netif_ip4_addr(netif_default)));

//
//
//  /* Wait for network to be ready (DHCP done) */
//  struct netif *netif = netif_list;
//  while (netif == NULL || netif_is_link_up(netif) == 0 || netif_is_up(netif) == 0 || netif->ip_addr.addr == 0) {
//    osDelay(500);
//    netif = netif_list;
//  }
//  printf("IP address assigned: %s\r\n", ipaddr_ntoa(&netif->ip_addr));
//

//
//
//  while (!netif_is_link_up(&gnetif) || !netif_is_up(&gnetif) || gnetif.ip_addr.addr == 0) {
//    osDelay(500);
//  }

//  // Wait for network link and IP (DHCP)
//  while (netif_is_link_up(netif_list) == 0 || netif_is_up(netif_list) == 0) {
//    osDelay(100);
//  }

//    ip = *netif_ip4_addr(netif_default);
//    sprintf(buf, "IP acquired: %u.%u.%u.%u", ip4_addr1(&ip), ip4_addr2(&ip),
//                    ip4_addr3(&ip), ip4_addr4(&ip));
//    LOG(buf);

	err_t err;
	ip = *netif_ip4_addr(netif_default);
	sprintf(buf, "IP acquired: %u.%u.%u.%u", ip4_addr1(&ip), ip4_addr2(&ip),
			ip4_addr3(&ip), ip4_addr4(&ip));
	LOG(buf);

	LOG("mdns_resp_init starting");
	mdns_resp_init();
	LOG("mdns_resp_init finished");

	err = mdns_resp_add_netif(netif_default, "lwip", 3600);

	sprintf(buf, "mdns_resp_add_netif returned: %d", err);
	LOG(buf);

	err = mdns_resp_add_service(netif_default, "stm32-http", "_http",
			DNSSD_PROTO_TCP, 80, 3600, NULL, NULL);

	sprintf(buf, "mdns_resp_add_service returned: %d", err);
	LOG(buf);

	mdns_resp_announce(netif_default);  // 🔹 actively broadcast the service
	LOG("mdns_resp_announce finished");

//	struct mg_mgr mgr;
//	mg_mgr_init(&mgr);
//
//	// Start HTTP server
//	// struct mg_connection *c =
//	mg_http_listen(&mgr, "http://0.0.0.0:80", http_handler, &mgr);
//  if (c == NULL) {
//	  CDC_Transmit_FS((uint8_t *)"Error: Cannot start HTTP listener\r\n", strlen("Error: Cannot start HTTP listener\r\n"));
//  } else {
//	  CDC_Transmit_FS((uint8_t *)"HTTP server started on port 80\r\n", strlen("HTTP server started on port 80\r\n"));
//  }

	// Main event loop
	for (;;) {
//		mg_mgr_poll(&mgr, 10);   // 10 ms polling
		osDelay(1);              // Yield to other FreeRTOS tasks
	}

	// mg_mgr_free(&mgr);

	/* USER CODE END 5 */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM6) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
