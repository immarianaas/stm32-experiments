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
	ETH_TimeStampTypeDef t1;
	ETH_TimeStampTypeDef t2;
	ETH_TimeStampTypeDef t3;
	ETH_TimeStampTypeDef t4;
};

struct ts ptp_ts;
struct ts *prev_ptp_ts = NULL;

static int compare_ETH_TimeStamps(ETH_TimeStampTypeDef *t1,
		ETH_TimeStampTypeDef *t2) {
	if (t1->TimeStampHigh == t2->TimeStampHigh) {
		if (t1->TimeStampLow == t2->TimeStampLow)
			return 0;
		return (t1->TimeStampLow > t2->TimeStampLow) ? 1 : -1;
	}
	return (t1->TimeStampHigh > t2->TimeStampHigh) ? 1 : -1;
}

static uint64_t to_uint64(ETH_TimeStampTypeDef *a) {
	return ((uint64_t) a->TimeStampHigh * 1000000000)
			+ (uint64_t) a->TimeStampLow;
}

static ETH_TimeStampTypeDef sub(ETH_TimeStampTypeDef *a,
		ETH_TimeStampTypeDef *b) {
	uint64_t a_64 = ((uint64_t) a->TimeStampHigh * 1000000000)
			+ (uint64_t) a->TimeStampLow;
	uint64_t b_64 = ((uint64_t) b->TimeStampHigh * 1000000000)
			+ (uint64_t) b->TimeStampLow;
	uint64_t sub_res = a_64 - b_64;

	ETH_TimeStampTypeDef res;
	res.TimeStampHigh = sub_res / 1000000000;
	res.TimeStampLow = sub_res % 1000000000;

	printf("[sub] res = %" PRIu32 " . %" PRIu32 " \r\n", res.TimeStampHigh,
			res.TimeStampLow);
	return res;
}

static ETH_TimeStampTypeDef add(ETH_TimeStampTypeDef *a,
		ETH_TimeStampTypeDef *b) {
	uint64_t a_64 = ((uint64_t) a->TimeStampHigh * 1000000000)
			+ (uint64_t) a->TimeStampLow;
	uint64_t b_64 = ((uint64_t) b->TimeStampHigh * 1000000000)
			+ (uint64_t) b->TimeStampLow;
	uint64_t add_res = a_64 + b_64;

	ETH_TimeStampTypeDef res;
	res.TimeStampHigh = add_res / 1000000000;
	res.TimeStampLow = add_res % 1000000000;

	printf("[add] res = %" PRIu32 " . %" PRIu32 " \r\n", res.TimeStampHigh,
			res.TimeStampLow);
	return res;
}

static ETH_TimeStampTypeDef halve(ETH_TimeStampTypeDef *a) {
	uint64_t a_64 = ((uint64_t) a->TimeStampHigh * 1000000000)
			+ (uint64_t) a->TimeStampLow;

	ETH_TimeStampTypeDef res;

	// Integer part
	res.TimeStampHigh = a->TimeStampHigh / 2;

	// Fractional part scaled to nanoseconds
	res.TimeStampLow = a->TimeStampLow / 2;

	printf("[halve] res = %" PRIu32 " . %" PRIu32 " \r\n", res.TimeStampHigh,
			res.TimeStampLow);
	return res;
}

static ETH_TimeStampTypeDef divide(ETH_TimeStampTypeDef *a,
		ETH_TimeStampTypeDef *b) {
	uint64_t a_64 = ((uint64_t) a->TimeStampHigh * 1000000000)
			+ (uint64_t) a->TimeStampLow;
	uint64_t b_64 = ((uint64_t) b->TimeStampHigh * 1000000000)
			+ (uint64_t) b->TimeStampLow;

	ETH_TimeStampTypeDef res;

	// Integer part
	res.TimeStampHigh = (uint32_t) (a_64 / b_64);

	// Fractional part scaled to nanoseconds
	res.TimeStampLow = (uint32_t) (((a_64 % b_64) * 1000000000) / b_64);

	printf("[div] res = %" PRIu32 " . %" PRIu32 " \r\n", res.TimeStampHigh,
			res.TimeStampLow);
	return res;
}

static double divide64(ETH_TimeStampTypeDef *a,
		ETH_TimeStampTypeDef *b) {
	uint64_t a_64 = to_uint64(a);
	uint64_t b_64 = to_uint64(b);

	return (double) a_64 / (double) b_64;

}

//static ETH_TimeStampTypeDef multiply(ETH_TimeStampTypeDef *a,
//		ETH_TimeStampTypeDef *b) {
//	ETH_TimeStampTypeDef res;
//
//	// Convert to total nanoseconds
//	uint64_t a_64 = ((uint64_t) a->TimeStampHigh * 1000000000)
//			+ (uint64_t) a->TimeStampLow;
//	uint64_t b_64 = ((uint64_t) b->TimeStampHigh * 1000000000)
//			+ (uint64_t) b->TimeStampLow;
//
//	// Multiply
//	uint64_t result_ns = a_64 * b_64;  // 64-bit multiplication
//
//	// Convert back to (sec, nsec)
//	res.TimeStampHigh = (uint32_t) (result_ns / 1000000000);     // seconds part
//	res.TimeStampLow = (uint32_t) (result_ns % 1000000000);  // nanoseconds part
//
//	printf("[mul] res = %" PRIu32 " . %" PRIu32 " \r\n", res.TimeStampHigh,
//			res.TimeStampLow);
//
//	return res;
//}

static ETH_TimeStampTypeDef divide_scalar(ETH_TimeStampTypeDef *a,
		uint64_t scalar) {
	uint64_t a_64 = to_uint64(a);
	uint64_t res_64 = a_64 / scalar;

	ETH_TimeStampTypeDef res;

	// Integer part
	res.TimeStampHigh = (uint32_t) (res_64 / 1000000000);

	// Fractional part scaled to nanoseconds
	res.TimeStampLow = (uint32_t) (res_64 % 1000000000);

	printf("[div] res = %" PRIu32 " . %" PRIu32 " \r\n", res.TimeStampHigh,
			res.TimeStampLow);
	return res;
}

static ETH_TimeStampTypeDef multiply_scalar(ETH_TimeStampTypeDef *a,
		double scalar) {
	ETH_TimeStampTypeDef res;

	// Convert to total nanoseconds
	uint64_t a_64 = to_uint64(a);

	// Multiply
	uint64_t result_ns = a_64 * scalar;  // 64-bit multiplication

	// Convert back to (sec, nsec)
	res.TimeStampHigh = (uint32_t) (result_ns / 1000000000);     // seconds part
	res.TimeStampLow = (uint32_t) (result_ns % 1000000000);  // nanoseconds part

	printf("[mul] res = %" PRIu32 " . %" PRIu32 " \r\n", res.TimeStampHigh,
			res.TimeStampLow);

	return res;
}


static double compute_skew(struct ts *curr, struct ts *prev)
{
	ETH_TimeStampTypeDef t2_diff = sub(&curr->t2, &prev->t2);
	ETH_TimeStampTypeDef t1_diff = sub(&curr->t1, &prev->t1);
	return divide64(&t2_diff, &t1_diff);
}

static ETH_TimeStampTypeDef compute_path_delay(struct ts *curr, struct ts *prev, double skew_double)
{
	ETH_TimeStampTypeDef t1_x_skew = multiply_scalar(&curr->t1, skew_double);
	ETH_TimeStampTypeDef t4_x_skew = multiply_scalar(&curr->t4, skew_double);

	int is_sec_faster_t2 = compare_ETH_TimeStamps(&curr->t2, &t1_x_skew); // TODO : handle negatives!
	int is_sec_faster_t3 = compare_ETH_TimeStamps(&curr->t3, &t4_x_skew);

	// I believe these two should have the same result
	if (is_sec_faster_t2  != is_sec_faster_t3 )
	{
		printf("I think this is wrong..\r\n");
	}

	ETH_TimeStampTypeDef pathdelay_sum;

	if (is_sec_faster_t2 == 1)
	{
		ETH_TimeStampTypeDef t2_sub_t1 = sub(&curr->t2, &t1_x_skew);
		ETH_TimeStampTypeDef t3_sub_t4 = sub(&curr->t3, &t4_x_skew); // invert to get positive!
		pathdelay_sum = sub(&t2_sub_t1, &t3_sub_t4);
	}

	if (is_sec_faster_t2 == -1)
	{
		ETH_TimeStampTypeDef t1_sub_t2 = sub(&t1_x_skew, &curr->t2);  // invert to get positive!
		ETH_TimeStampTypeDef t4_sub_t3 = sub(&t4_x_skew, &curr->t3);
		pathdelay_sum = sub(&t4_sub_t3, &t1_sub_t2);
	}

	if (is_sec_faster_t2 == 0)
	{
		printf("TODO: handle here \r\n");
	}

	return halve(&pathdelay_sum);
}

static void compute_metrics(struct ts *curr, struct ts *prev) {
	// compute skew
	ETH_TimeStampTypeDef t2_diff = sub(&curr->t2, &prev->t2);
	ETH_TimeStampTypeDef t1_diff = sub(&curr->t1, &prev->t1);
	ETH_TimeStampTypeDef skew = divide(&t2_diff, &t1_diff);
	double skew_double = divide64(&t2_diff, &t1_diff);
	printf("[compute_metrics] skew = %" PRIu32 " . %" PRIu32 " \r\n",
			skew.TimeStampHigh, skew.TimeStampLow);
	printf("[compute_metrics] skew double = %f \r\n", skew_double);

	// compute pathdelay
	ETH_TimeStampTypeDef t1_x_skew = multiply_scalar(&curr->t1, skew_double);
	printf("[compute_metrics] t1_x_skew = %" PRIu32 " . %" PRIu32 " \r\n",
			t1_x_skew.TimeStampHigh, t1_x_skew.TimeStampLow);

	ETH_TimeStampTypeDef t4_x_skew = multiply_scalar(&curr->t4, skew_double);
	ETH_TimeStampTypeDef t2_sub_t1 = sub(&curr->t2, &t1_x_skew);
	// ETH_TimeStampTypeDef t4_sub_t3 = sub(&t4_x_skew, &curr->t3);
	ETH_TimeStampTypeDef t4_sub_t3 = sub(&curr->t3, &t4_x_skew); // altered (negative!)
	ETH_TimeStampTypeDef pathdelay_sum = sub(&t2_sub_t1, &t4_sub_t3); // altered (negative!)
	ETH_TimeStampTypeDef pathdelay = halve(&pathdelay_sum);
	printf("[compute_metrics] pathdelay = %" PRIu32 " . %" PRIu32 " \r\n",
			pathdelay.TimeStampHigh, pathdelay.TimeStampLow);

	// compute offset (Henrik document)
	ETH_TimeStampTypeDef t2_sub_t1_o = sub(&curr->t2, &curr->t1);
	ETH_TimeStampTypeDef offset = sub(&t2_sub_t1_o, &pathdelay);
	printf("[compute_metrics] offset Henrik = %" PRIu32 " . %" PRIu32 " \r\n",
			offset.TimeStampHigh, offset.TimeStampLow);

	// compute offset (website)
	ETH_TimeStampTypeDef offset_sub = sub(&t2_sub_t1, &t4_sub_t3);
	offset = halve(&offset_sub);
	printf("[compute_metrics] offset website = %" PRIu32 " . %" PRIu32 " \r\n",
			offset.TimeStampHigh, offset.TimeStampLow);

}

static void handle_end_ptp_exchange() {
	if (prev_ptp_ts == NULL) {

		int is_valid = (compare_ETH_TimeStamps(&ptp_ts.t1, &ptp_ts.t4) == -1)
				&& (compare_ETH_TimeStamps(&ptp_ts.t2, &ptp_ts.t3) == -1);
		printf("[first time] is PTP valid? %d \r\n", is_valid);

		prev_ptp_ts = malloc(sizeof(struct ts));
		*prev_ptp_ts = ptp_ts;
		return;
	}

	int is_valid = (compare_ETH_TimeStamps(&ptp_ts.t1, &ptp_ts.t4) == -1)
			&& (compare_ETH_TimeStamps(&ptp_ts.t2, &ptp_ts.t3) == -1);
	printf("is PTP valid? %d \r\n", is_valid);

	// uint32_t skew = (ptp_ts.t2 - ptp_ts.prev.t2) / (ptp_ts.t1 - ptp_ts.prev.t1);

	//	ETH_TimeStampTypeDef t1;
	//	t1.TimeStampHigh = 5;
	//	t1.TimeStampLow = 455000000;
	//	ETH_TimeStampTypeDef t2;
	//	t2.TimeStampHigh = 2;
	//	t2.TimeStampLow = 790000000;
	//	sub(&t1, &t2);
	//	divide(&t1, &t2);

	printf("[handle] t1 = %" PRIu32 " . %" PRIu32 " \r\n",
			ptp_ts.t1.TimeStampHigh, ptp_ts.t1.TimeStampLow);

	printf("[handle] prev t1 = %" PRIu32 " . %" PRIu32 " \r\n",
			prev_ptp_ts->t1.TimeStampHigh, prev_ptp_ts->t1.TimeStampLow);
//
//	ETH_TimeStampTypeDef t2_diff = sub(&ptp_ts.t2, &prev_ptp_ts->t2);
//	ETH_TimeStampTypeDef t1_diff = sub(&ptp_ts.t1, &prev_ptp_ts->t1);
//	ETH_TimeStampTypeDef skew = divide(&t2_diff, &t1_diff);
//
//	// pathdelay = ((t2 - t1 * skew) + (skew * t4 - t3)) / 2
//	ETH_TimeStampTypeDef t1_x_skew = multiply(&ptp_ts.t1, &skew);
//	ETH_TimeStampTypeDef t4_x_skew = multiply(&ptp_ts.t4, &skew);
//	ETH_TimeStampTypeDef t2_sub_t1 = sub(&ptp_ts.t2, &t1_x_skew);
//	ETH_TimeStampTypeDef t4_sub_t3 = sub(&t4_x_skew, &ptp_ts.t3);
//	ETH_TimeStampTypeDef pathdelay_sum = add(&t2_sub_t1, &t4_sub_t3);
//	ETH_TimeStampTypeDef pathdelay = halve(&pathdelay_sum);
//
//	// offset = (t2 - t1) - pathDelay
//	ETH_TimeStampTypeDef t2_sub_t1_o = sub(&ptp_ts.t2, &ptp_ts.t1);
//	ETH_TimeStampTypeDef offset = sub(&t2_sub_t1_o, &pathdelay);
//
//	printf("skew = %u . %u\r\n", skew.TimeStampHigh, skew.TimeStampLow);
//	printf("pathdelay= %u . %u\r\n", pathdelay.TimeStampHigh,
//			pathdelay.TimeStampLow);
//	printf("offset= %u . %u\r\n", offset.TimeStampHigh, offset.TimeStampLow);
//
//	ETH_TimeStampTypeDef primary_t3 = sub(&ptp_ts.t3, &offset);
//	printf("primary_t4 = %u . %u\r\n", primary_t3.TimeStampHigh,
//			primary_t3.TimeStampLow);

	//	printf("t1_diff = %u . %u\r\n", t1_diff.TimeStampHigh,
	//			t1_diff.TimeStampLow);

	// uint32_t pathdelay = (ptp_ts.t2 - ptp_ts.prev.t2) / (ptp_ts.t1 - ptp_ts.prev.t1);

	*prev_ptp_ts = ptp_ts;
}

static void convert_to_ETH_TimeStamp(timeTypeDef *timestamp,
		ETH_TimeStampTypeDef *converted) {
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
	print_timestamp(&(send_p->timestamp),
			"[send_delay_req] finished sending DELAY REQ ((t3))");
	ptp_ts.t3 = send_p->timestamp;
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
		print_timestamp(&(p->timestamp),
				"[handle_udp_ptp_sync] received SYNC ((t2))");
		ptp_ts.t2 = p->timestamp;

	}

	pbuf_free(p);

	ETH_TimeStampTypeDef prev_t1 = { .TimeStampHigh = 9, .TimeStampLow = 0 };
	ETH_TimeStampTypeDef prev_t2 = { .TimeStampHigh = 9, .TimeStampLow = 11 };
	ETH_TimeStampTypeDef prev_t3 = { .TimeStampHigh = 9, .TimeStampLow = 12 };
	ETH_TimeStampTypeDef prev_t4 = { .TimeStampHigh = 9, .TimeStampLow = 03 };

	ETH_TimeStampTypeDef curr_t1 = { .TimeStampHigh = 9, .TimeStampLow = 8 };
	ETH_TimeStampTypeDef curr_t2 = { .TimeStampHigh = 9, .TimeStampLow = 19 };
	ETH_TimeStampTypeDef curr_t3 = { .TimeStampHigh = 9, .TimeStampLow = 20 };
	ETH_TimeStampTypeDef curr_t4 = { .TimeStampHigh = 9, .TimeStampLow = 11 };

	struct ts curr_test_ts;
	curr_test_ts.t1 = curr_t1;
	curr_test_ts.t2 = curr_t2;
	curr_test_ts.t3 = curr_t3;
	curr_test_ts.t4 = curr_t4;

	struct ts prev_test_ts;
	prev_test_ts.t1 = prev_t1;
	prev_test_ts.t2 = prev_t2;
	prev_test_ts.t3 = prev_t3;
	prev_test_ts.t4 = prev_t4;

	compute_metrics(&curr_test_ts, &prev_test_ts);

	example();

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
		print_timestamp(&(ptp_ts.t1),
				"[handle_udp_ptp] received FOLLOW Up ((t1))");

		send_delay_req(upcb, addr, port);
	} else if ((data[0] & 0xf) == 9) {
		timeTypeDef msgtime = get_time_from_msg(34, data);
		convert_to_ETH_TimeStamp(&msgtime, &ptp_ts.t4);
		// print_time(&msgtime, "[handle_udp_ptp DELAY RESPONSEP]");
		print_timestamp(&(ptp_ts.t1),
				"[handle_udp_ptp] received DELAY RESPONSE ((t4))");

		handle_end_ptp_exchange();

	} else {
		printf("[handle_udp_ptp] shouldn't be here");
	}

	pbuf_free(p);

}

/**
 * Returns:
 * -  1 if t1  > t2
 * -  0 if t1 == t2
 * - -1 if t1  < t2
 */

#endif /* SRC_UDP_PTP_H_ */
