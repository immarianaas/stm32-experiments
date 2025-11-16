/*
 * ptp_comp.h
 *
 *  Created on: Nov 16, 2025
 *      Author: mar
 */

#ifndef SRC_PTP_COMP_H_
#define SRC_PTP_COMP_H_

#define NANO 1000000000

typedef int64_t DeltaTimeType; // in Nano seconds; can be pos or neg

struct deltatime_ts {
	DeltaTimeType t1;
	DeltaTimeType t2;
	DeltaTimeType t3;
	DeltaTimeType t4;
};


static void print_deltatime(DeltaTimeType value, char *add_str) {

	char sign = (value < 0) ? '-' : '+';
	if (value < 0) value = -value;   // make positive for printing

	int32_t seconds = (int32_t) (value / NANO);
	int32_t nanoseconds = (int32_t) (value % NANO);

	printf("%s: deltatime = %c%" PRId32 ".%09" PRId32 "\r\n", add_str, sign, seconds, nanoseconds);
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


static void example() {

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



#endif /* SRC_PTP_COMP_H_ */
