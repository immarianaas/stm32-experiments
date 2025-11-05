/*
 * udp_ptp.h
 *
 *  Created on: Nov 2, 2025
 *      Author: mar
 */

#ifndef SRC_UDP_PTP_H_
#define SRC_UDP_PTP_H_

#include "lwip.h"

static uint8_t ptp_delay_req_payload[] = { 0x01, 0x02, 0x00, 0x2C, 0x00, 0x00,
		0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00 };

struct time {
	uint16_t seconds_high;
	uint32_t seconds_low;
	uint32_t nanoseconds;
};

static struct time get_time_from_msg(const int sindex /*starting index34*/,
		uint8_t *data) {

	struct time result;

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

static void print_time(struct time *time_to_print, char* add_str)
{
	printf("%s: seconds=0x%02" PRIX32 "%08" PRIX32 ", nanoseconds=0x%08" PRIX32 "\r\n",
			add_str, time_to_print->seconds_high, time_to_print->seconds_low, time_to_print->nanoseconds);
}

static void send_delay_req(struct udp_pcb *upcb, const ip_addr_t *addr,
		u16_t port) {
	// update sequenceId which is 2 bytes and starts in index 30
	uint16_t sequenceId = 3;
	ptp_delay_req_payload[30] = (uint8_t) (sequenceId & 0xff00) >> 8;
	ptp_delay_req_payload[31] = (uint8_t) (sequenceId & 0x00ff);

	struct pbuf *send_p = pbuf_alloc(PBUF_TRANSPORT,
			sizeof(ptp_delay_req_payload), PBUF_RAM);
	if (send_p == NULL)
		printf("IT IS NULL\r\n");

	osDelay(1000);
	pbuf_take(send_p, ptp_delay_req_payload, sizeof(ptp_delay_req_payload));
	udp_sendto(upcb, send_p, addr, 3190);

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

	printf("[handle_udp_ptp_sync] p->len = %d\r\n", p->len);
	// printf("[handle_udp_ptp] p->payload = %s\r\n", (char *)p->payload);

	printf("[handle_udp_ptp_sync] txtimestamp high: %d \r\n",
			heth.TxTimestamp.TimeStampHigh);
	printf("[handle_udp_ptp_sync] txtimestamp low: %d \r\n",
			heth.TxTimestamp.TimeStampLow);
	printf("[handle_udp_ptp_sync] IsPtpConfigured: %d \r\n",
			heth.IsPtpConfigured);

	printf("[handle_udp_ptp_sync] p->payload: ");

	uint8_t *data = (uint8_t*) p->payload;
	for (u16_t i = 0; i < p->len; i++) {
		printf("%02X ", data[i]);
	}
	printf("\r\n");

	if ((data[0] & 0xf) == 0) {
		printf("Received SYNC\r\n");

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

	printf("[handle_udp_ptp] p->len = %d\r\n", p->len);
	// printf("[handle_udp_ptp] p->payload = %s\r\n", (char *)p->payload);

	printf("[handle_udp_ptp] p->payload: ");

	uint8_t *data = (uint8_t*) p->payload;
	for (u16_t i = 0; i < p->len; i++) {
		printf("%02X ", data[i]);
	}
	printf("\r\n");

	if ((data[0] & 0xf) == 8) {
		printf("[handle_udp_ptp] Received FOLLOW UP\r\n");

		struct time msgtime = get_time_from_msg(34, data);
		print_time(&msgtime, "[handle_udp_ptp FOLLOW UP]");

		send_delay_req(upcb, addr, port);
	} else if ((data[0] & 0xf) == 9) {
		printf("[handle_udp_ptp] Received DELAY RESPONSE\r\n");
		struct time msgtime = get_time_from_msg(34, data);
		print_time(&msgtime, "[handle_udp_ptp DELAY RESPONSEP]");
	} else {
		printf("[handle_udp_ptp] shouldn't be here");
	}

	pbuf_free(p);

}

#endif /* SRC_UDP_PTP_H_ */
