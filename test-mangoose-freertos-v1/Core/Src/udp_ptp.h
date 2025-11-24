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

#include "ptp_comp.h"

static uint8_t ptp_delay_req_payload[] = { 0x01, 0x02, 0x00, 0x2C, 0x00, 0x00,
		0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00 };

struct deltatime_ts ptp_ts;
struct deltatime_ts *prev_ptp_ts = NULL;

static uint16_t seqId;

static void handle_end_ptp_exchange() {
	if (prev_ptp_ts == NULL) {
		printf("[handle_end_ptp_exchange] is PTP valid? %d \r\n",
				is_deltatime_ts_valid(&ptp_ts));

		prev_ptp_ts = malloc(sizeof(struct deltatime_ts));
		*prev_ptp_ts = ptp_ts;

		return;
	}

	printf("[handle_end_ptp_exchange] is PTP valid? %d \r\n",
			is_deltatime_ts_valid(&ptp_ts));

	compute_all_metrics(&ptp_ts, prev_ptp_ts);

	*prev_ptp_ts = ptp_ts;
}

static DeltaTimeType get_dt_time_from_msg(const int sindex /*starting index34*/,
		uint8_t *data) {

	DeltaTimeType result = 0;

	for (int i = 0; i < 10; i++) {
		result = (result << 8) | data[sindex + i];
	}

	return result;
}

static void print_payload(uint8_t *data, int len) {
	for (u16_t i = 0; i < len; i++) {
		printf("%02X ", data[i]);
	}
	printf("\r\n");
}

static void send_delay_req(struct udp_pcb *upcb, const ip_addr_t *addr,
		u16_t port) {
	// TODO: update sequenceId which is 2 bytes and starts in index 30 (I think this is wrong, should be 29)
	// uint16_t sequenceId = 3;
	ptp_delay_req_payload[30] = (uint8_t) ((seqId & 0xff00) >> 8);
	ptp_delay_req_payload[31] = (uint8_t) (seqId & 0x00ff);

	struct pbuf *send_p = pbuf_alloc(PBUF_TRANSPORT,
			sizeof(ptp_delay_req_payload), PBUF_RAM);

	// osDelay(1000); // when using with the python thing

	pbuf_take(send_p, ptp_delay_req_payload, sizeof(ptp_delay_req_payload));
	err_t res = udp_sendto(upcb, send_p, addr, 3190);

	/* --- SAVING T3 --- */
	ptp_ts.t3 = toDeltaTimeType(&(send_p->timestamp));
	print_deltatime(ptp_ts.t3,
			"[send_delay_req] finished sending DELAY REQ ((t3))");
	pbuf_free(send_p);
}

static void handle_udp_ptp_sync(void *arg, struct udp_pcb *upcb, struct pbuf *p,
		const ip_addr_t *addr, u16_t port) {
	if (p == NULL)
		return;

	uint8_t *data = (uint8_t*) p->payload;

	if ((data[0] & 0xf) == 0) {

		/* --- SAVING T2 --- */
		ptp_ts.t2 = toDeltaTimeType(&p->timestamp);
		seqId = ((uint16_t) data[29] << 8) | (uint16_t) data[30];
		// printf("SYNC seqId=%d, (uint16_t)data[30]<<8 = %d, (uint16_t)data[31] = %d\r\n", seqId, ((uint16_t)data[30] <<8), (uint16_t)data[31]);
		print_deltatime(ptp_ts.t2,
				"[handle_udp_ptp_sync] received SYNC ((t2))");
	}

	pbuf_free(p);

}

static void handle_udp_ptp(void *arg,
		struct udp_pcb *upcb,
		struct pbuf *p,
		const ip_addr_t *addr,
		u16_t port)
{
	if (p == NULL)
		return;

	uint8_t *data = (uint8_t*) p->payload;

	if ((data[0] & 0xf) == 8) {

		/* --- STORING T1 -- */
		ptp_ts.t1 = get_dt_time_from_msg(34, data);
		print_deltatime(ptp_ts.t1,
				"[handle_udp_ptp] received FOLLOW Up ((t1))");

		send_delay_req(upcb, addr, port);

	} else if ((data[0] & 0xf) == 9) {
		/* --- STORING T4 -- */
		ptp_ts.t4 = get_dt_time_from_msg(34, data);

		print_deltatime(ptp_ts.t4,
				"[handle_udp_ptp] received DELAY RESPONSE ((t4))");

		handle_end_ptp_exchange();

	} else {
		printf("[handle_udp_ptp] shouldn't be here");
	}

	pbuf_free(p);

}

#endif /* SRC_UDP_PTP_H_ */
