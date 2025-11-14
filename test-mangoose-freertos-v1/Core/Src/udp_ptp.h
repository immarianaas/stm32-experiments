/*
 * udp_ptp.h
 *
 *  Created on: Nov 2, 2025
 *      Author: mar
 */

#ifndef SRC_UDP_PTP_H_
#define SRC_UDP_PTP_H_

#include "lwip.h"
// #include "stm32f7xx_hal.h"

#include "FreeRTOS.h"

static uint8_t ptp_delay_req_payload[] = { 0x01, 0x02, 0x00, 0x2C, 0x00, 0x00,
		0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00 };

//struct time {
//	uint16_t seconds_high;
//	uint32_t seconds_low;
//	uint32_t nanoseconds;
//};

typedef struct {
	uint16_t seconds_high;
	uint32_t seconds_low;
	uint32_t nanoseconds;
} timeTypeDef;

struct ts {
	struct ts *prev;
	ETH_TimeStampTypeDef t1;
	ETH_TimeStampTypeDef t2;
	ETH_TimeStampTypeDef t3;
	ETH_TimeStampTypeDef t4;
};

struct ts ptp_ts = { .prev = NULL };


static void convert_to_ETH_TimeStamp(timeTypeDef *timestamp, ETH_TimeStampTypeDef *converted) {
	converted->TimeStampHigh = timestamp->seconds_low;
	converted->TimeStampLow = timestamp->nanoseconds;
	return;
}

static timeTypeDef get_time_from_msg(const int sindex /*starting index34*/,
		uint8_t *data) {

	timeTypeDef result;

	result.seconds_high = ((uint64_t) data[sindex] << 8)
			| ((uint64_t) data[sindex + 1]);

	result.seconds_low = ((uint64_t) data[sindex + 2] << 24)
			| ((uint64_t) data[sindex + 3] << 16)
			| ((uint64_t) data[sindex + 4] << 8)
			| ((uint64_t) data[sindex + 5]);

	result.nanoseconds = ((uint32_t) data[sindex + 6] << 24)
			| ((uint32_t) data[sindex + 7] << 16)
			| ((uint32_t) data[sindex + 8] << 8)
			| ((uint32_t) data[sindex + 9]);
	return result;
}

static void print_time(timeTypeDef *time_to_print, char *add_str) {
	printf(
			"%s: seconds=0x%02" PRIX32 "%08" PRIX32 ", nanoseconds=0x%08" PRIX32 "\r\n",
			add_str, time_to_print->seconds_high, time_to_print->seconds_low,
			time_to_print->nanoseconds);
}

static void print_timestamp(ETH_TimeStampTypeDef *timestamp, char *add_str) {
	printf("%s: seconds=0x%08" PRIX32 ", nanoseconds=0x%08" PRIX32 "\r\n",
			add_str, timestamp->TimeStampHigh, timestamp->TimeStampLow);
}

static void print_payload(uint8_t *data, int len) {
	for (u16_t i = 0; i < len; i++) {
		printf("%02X ", data[i]);
	}
	printf("\r\n");
}

static void send_delay_req(struct udp_pcb *upcb, const ip_addr_t *addr,
		u16_t port) {
	// update sequenceId which is 2 bytes and starts in index 30
	uint16_t sequenceId = 3;
	ptp_delay_req_payload[30] = (uint8_t) (sequenceId & 0xff00) >> 8;
	ptp_delay_req_payload[31] = (uint8_t) (sequenceId & 0x00ff);

	struct pbuf *send_p = pbuf_alloc(PBUF_TRANSPORT,
			sizeof(ptp_delay_req_payload), PBUF_RAM);

	//if (send_p == NULL)

	osDelay(1000); // TODO: fix this


	//TickType_t xDelay = 5000 / portTICK_PERIOD_MS;
	//vTaskDelay(xDelay);


	pbuf_take(send_p, ptp_delay_req_payload, sizeof(ptp_delay_req_payload));
	err_t res = udp_sendto(upcb, send_p, addr, 3190);
	print_timestamp(&(send_p->timestamp), "[send_delay_req] finished sending DELAY REQ ((t3))");
	printf("res= %d\r\n", res);
	ptp_ts.t3 = send_p->timestamp;
	printf("should have sent, size=%d\r\n", sizeof(ptp_delay_req_payload));

	pbuf_free(send_p);
}

static void handle_udp_ptp_sync(void *arg, // User argument - udp_recv `arg` parameter
		struct udp_pcb *upcb,   // Receiving Protocol Control Block
		struct pbuf *p,         // Pointer to Datagram
		const ip_addr_t *addr,  // Address of sender
		u16_t port)            // Sender port
{
	if (p == NULL)
		return;

//	printf("[handle_udp_ptp_sync] p->payload: ");
//	print_payload(p->payload);

	uint8_t *data = (uint8_t*) p->payload;

	if ((data[0] & 0xf) == 0) {
		print_timestamp(&(p->timestamp), "[handle_udp_ptp_sync] received SYNC ((t2))");
		ptp_ts.t2 = p->timestamp;

	}

	pbuf_free(p);

}

static void handle_udp_ptp(void *arg, // User argument - udp_recv `arg` parameter
		struct udp_pcb *upcb,   // Receiving Protocol Control Block
		struct pbuf *p,         // Pointer to Datagram
		const ip_addr_t *addr,  // Address of sender
		u16_t port)            // Sender port
{
	if (p == NULL)
		return;


	uint8_t *data = (uint8_t*) p->payload;

	if ((data[0] & 0xf) == 8) {
		// printf("[handle_udp_ptp] Received FOLLOW UP\r\n");

		timeTypeDef msgtime = get_time_from_msg(34, data);
		convert_to_ETH_TimeStamp(&msgtime, &ptp_ts.t1);
		print_timestamp(&(ptp_ts.t1), "[handle_udp_ptp] received FOLLOW Up ((t1))");


		send_delay_req(upcb, addr, port);
	} else if ((data[0] & 0xf) == 9) {
		timeTypeDef msgtime = get_time_from_msg(34, data);
		convert_to_ETH_TimeStamp(&msgtime, &ptp_ts.t4);
		// print_time(&msgtime, "[handle_udp_ptp DELAY RESPONSEP]");
		print_timestamp(&(ptp_ts.t1), "[handle_udp_ptp] received DELAY RESPONSE ((t4))");
	} else {
		printf("[handle_udp_ptp] shouldn't be here");
	}

	pbuf_free(p);

}


static int compare_ETH_TimeStamps(ETH_TimeStampTypeDef *t1, ETH_TimeStampTypeDef *t2) {
	if (t1->TimeStampHigh == t2->TimeStampHigh) {
		if (t1->TimeStampLow == t2->TimeStampLow)
			return 0;
		return (t1->TimeStampLow > t2->TimeStampLow) ? 1 : -1;
	}
	return (t1->TimeStampHigh > t2->TimeStampHigh) ? 1 : -1;
}

static void handle_end_ptp_exchange() {
	if (ptp_ts.prev != NULL) {

	}

	int is_valid = (compare_ETH_TimeStamps(&ptp_ts.t1, &ptp_ts.t4) == -1) && (compare_ETH_TimeStamps(&ptp_ts.t2, &ptp_ts.t3) == -1);
	printf("is PTP valid? %d \r\n", is_valid);
	ptp_ts.prev = &ptp_ts;
}

/**
 * Returns:
 * -  1 if t1  > t2
 * -  0 if t1 == t2
 * - -1 if t1  < t2
 */


#endif /* SRC_UDP_PTP_H_ */
