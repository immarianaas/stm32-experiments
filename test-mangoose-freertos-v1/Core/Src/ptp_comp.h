/*
 * ptp_comp.h
 *
 *  Created on: Nov 16, 2025
 *      Author: mar
 */

#ifndef SRC_PTP_COMP_H_
#define SRC_PTP_COMP_H_

#define NANO 1000000000

#define SHOULD_PRINT_PTP 1

typedef int64_t DeltaTimeType; // in Nano seconds; can be pos or neg

struct deltatime_ts {
	DeltaTimeType t1;
	DeltaTimeType t2;
	DeltaTimeType t3;
	DeltaTimeType t4;
};

struct ptp_metrics {
	double skew;
	DeltaTimeType offset;
} result_ptp;

static void print_deltatime_ts(struct deltatime_ts *dt_ts)
{
	if (!SHOULD_PRINT_PTP) return;

	int32_t seconds = (int32_t) (dt_ts->t1/ NANO);
	int32_t nanoseconds = (int32_t) (dt_ts->t1 % NANO);
	printf("t1: %" PRId32 ".%09" PRId32 "\r\n", seconds, nanoseconds);

	seconds = (int32_t) (dt_ts->t2/ NANO);
	nanoseconds = (int32_t) (dt_ts->t2 % NANO);
	printf("t2: %" PRId32 ".%09" PRId32 "\r\n", seconds, nanoseconds);

	seconds = (int32_t) (dt_ts->t3/ NANO);
	nanoseconds = (int32_t) (dt_ts->t3 % NANO);
	printf("t3: %" PRId32 ".%09" PRId32 "\r\n", seconds, nanoseconds);

	seconds = (int32_t) (dt_ts->t4/ NANO);
	nanoseconds = (int32_t) (dt_ts->t4 % NANO);
	printf("t4: %" PRId32 ".%09" PRId32 "\r\n", seconds, nanoseconds);

}
static void print_deltatime(DeltaTimeType value, char *add_str) {
	if (!SHOULD_PRINT_PTP) return;

	char sign = (value < 0) ? '-' : '+';
	if (value < 0) value = -value;   // make positive for printing

	int32_t seconds = (int32_t) (value / NANO);
	int32_t nanoseconds = (int32_t) (value % NANO);

	printf("%s: deltatime = %c%" PRId32 ".%09" PRId32 "\r\n", add_str, sign, seconds, nanoseconds);
}


static DeltaTimeType toDeltaTimeType(ETH_TimeStampTypeDef *a) {

	return (DeltaTimeType) ((uint64_t) a->TimeStampHigh * 1000000)
			+ (uint64_t) a->TimeStampLow;
	// 1 ms = 1000000 ns
}


static double compute_skew_deltatime(struct deltatime_ts *curr, struct deltatime_ts *prev)
{
	DeltaTimeType t2_diff = curr->t2 - prev->t2;
	DeltaTimeType t1_diff = curr->t1 - prev->t1;

	return (double) (((double)t2_diff) / ((double)t1_diff));
}

static DeltaTimeType compute_pathdelay_deltatime(struct deltatime_ts *curr, double skew)
{
	DeltaTimeType t1_x_skew = curr->t1 * skew;
	DeltaTimeType t4_x_skew = curr->t4 * skew;

	DeltaTimeType sum_res = (curr->t2 - t1_x_skew) + (t4_x_skew - curr->t3);
	return sum_res / 2;
}

static DeltaTimeType compute_offset_Henrik_deltatime(struct deltatime_ts *curr, DeltaTimeType pathdelay)
{
	return curr->t2 - curr->t1 - pathdelay;
}


static DeltaTimeType compute_offset_website_deltatime(struct deltatime_ts *curr)
{
	DeltaTimeType sub_res = curr->t2 - curr->t1 - (curr->t4 - curr->t3);
	return sub_res / 2;

}

static DeltaTimeType compute_t1(struct deltatime_ts *curr, DeltaTimeType offset, DeltaTimeType pathdelay)
{
	DeltaTimeType calc_t1 = curr->t2 - offset - pathdelay;
	return calc_t1;

}

static int is_deltatime_ts_valid(struct deltatime_ts *curr)
{
	return (curr->t1 < curr->t4) && (curr->t2 < curr->t3);
}

static void example() {
	/** TESTING ONLY **/
	struct deltatime_ts ts_prev = {
			.t1 = NANO * 9.0,
			.t2 = NANO * 9.11,
			.t3 = NANO * 9.12,
			.t4 = NANO * 9.03
	};

	struct deltatime_ts ts_curr = {
			.t1 = NANO * 9.08,
			.t2 = NANO * 9.19,
			.t3 = NANO * 9.20,
			.t4 = NANO * 9.11
	};

	double skew = compute_skew_deltatime(&ts_curr, &ts_prev);
	printf("[new example]: skew = %f\r\n", skew);

	DeltaTimeType pathdelay = compute_pathdelay_deltatime(&ts_curr, skew);
	print_deltatime(pathdelay, "[New example]: pathdelay");

	DeltaTimeType offset_Henrik = compute_offset_Henrik_deltatime(&ts_curr, pathdelay);

	print_deltatime(offset_Henrik, "[New example]: offset Henrik");

	DeltaTimeType offset_website = compute_offset_website_deltatime(&ts_curr);
	print_deltatime(offset_website, "[New example]: offset website");

}

static void compute_all_metrics(struct deltatime_ts *ts_curr, struct deltatime_ts *ts_prev)
{


	print_deltatime_ts(ts_curr);
	print_deltatime_ts(ts_prev);

	double skew = compute_skew_deltatime(ts_curr, ts_prev);
	if (SHOULD_PRINT_PTP)
		printf("[compute_all_metrics]: skew = %f\r\n", skew);

	DeltaTimeType pathdelay = compute_pathdelay_deltatime(ts_curr, skew);
	print_deltatime(pathdelay, "[compute_all_metrics]: pathdelay");

	DeltaTimeType offset_Henrik = compute_offset_Henrik_deltatime(ts_curr, pathdelay);

	print_deltatime(offset_Henrik, "[compute_all_metrics]: offset Henrik");

	DeltaTimeType offset_website = compute_offset_website_deltatime(ts_curr);
	print_deltatime(offset_website, "[compute_all_metrics]: offset website");

	DeltaTimeType c_t1 = compute_t1(ts_curr, offset_Henrik, pathdelay);
	print_deltatime(c_t1, "[... henrik]: c_t1");

	c_t1 = compute_t1(ts_curr, offset_website, pathdelay);
	print_deltatime(c_t1, "[... website]: c_t1");

	result_ptp.skew = skew;
	result_ptp.offset = offset_Henrik;

}



#endif /* SRC_PTP_COMP_H_ */
