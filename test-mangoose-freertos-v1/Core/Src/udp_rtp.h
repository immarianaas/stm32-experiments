/*
 * udp_rtp.h
 *
 *  Created on: Nov 19, 2025
 *      Author: mar
 */

#ifndef SRC_UDP_RTP_H_
#define SRC_UDP_RTP_H_

#include "udp_ptp.h"
#include "ptp_comp.h"

static void print_sub_payload(uint8_t *data, int sindex, int len) {
	for (u16_t i = sindex; i < sindex + len; i++) {
		printf("%02X ", data[i]);
	}
	printf("\r\n");
}

static void print_deltatime_rtp(DeltaTimeType value, char *add_str) {
	char sign = (value < 0) ? '-' : '+';
	if (value < 0)
		value = -value;   // make positive for printing

	int32_t seconds = (int32_t) (value / NANO);
	int32_t nanoseconds = (int32_t) (value % NANO);

	printf("%s: deltatime = %c%" PRId32 ".%09" PRId32 "\r\n", add_str, sign,
			seconds, nanoseconds);
}

static void print_deltatime_rtp_inline(DeltaTimeType value, char *add_str) {

	int32_t seconds = (int32_t) (value / NANO);
	uint32_t nanoseconds = (uint32_t) (value % NANO);

	printf("%s = %" PRId32 ".%09" PRIu32 " | ", add_str, seconds, nanoseconds);
}

static DeltaTimeType obtain_secondary_ts(DeltaTimeType pts_primary, struct ptp_metrics ptp_info)
{
//	return (ptp_info.skew * pts_primary) + ptp_info.offset;
	return (1 * pts_primary) + ptp_info.offset;
}


static void handle_udp_rtp(void *arg, struct udp_pcb *upcb, struct pbuf *p,
		const ip_addr_t *addr, u16_t port) {
	if (p == NULL)
		return;

//	printf("received rtp\r\n");
	uint8_t *data = (uint8_t*) p->payload;
	// first header extension is on byte 17th)
//	printf("timestamp: ");
//	print_sub_payload(data, 16+1, 10);
//
//	printf("delay: ");
//	print_sub_payload(data, 27+1, 1);

	DeltaTimeType delay = data[28] * 1000000; // convert [ms] to [ns]
	DeltaTimeType ts = get_dt_time_from_msg(17, data); // fetches 10 bytes


//
//	print_deltatime_rtp(ts, "ts from RTP msg");
//	print_deltatime_rtp(delay, "delay from RTP msg");

	print_deltatime_rtp_inline(ts, "msg ts");
	print_deltatime_rtp_inline(delay, "msg delay");


	printf("ptp skew = %f | ", result_ptp.skew);
	print_deltatime_rtp_inline(result_ptp.offset, "ptp offset");

//	printf("latest ptp skew: %f\r\n", result_ptp.skew);

	DeltaTimeType secondary_ts = obtain_secondary_ts(ts+delay, result_ptp);
	print_deltatime_rtp_inline(secondary_ts, "calc sec ts");

	printf("\r");

//	print_deltatime_rtp(secondary_ts, "calc secundary ts");


	// payload is the rest of the message..
	pbuf_free(p);

}

#endif /* SRC_UDP_RTP_H_ */
