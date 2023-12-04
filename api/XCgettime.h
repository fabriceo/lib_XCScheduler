/*
 * XC_gettime.h
 *
 *  Created on: 27 oct. 2023
 *      Author: fabriceo
 */

#ifndef XCGETTIME_H_
#define XCGETTIME_H_

//basic helper function to read time from XCore global timer
static inline int XC_GET_TIME()            { int time; asm volatile("gettime %0":"=r"(time)); return time; }
static inline int XC_SET_TIME(const int x) { int time = XC_GET_TIME() + x; return time;}
static inline int XC_END_TIME(const int t) { int time = XC_GET_TIME() - t; return ( time >= 0 ); }

typedef __attribute__((aligned(8))) union XCtime_u {
    long long ll;
    unsigned long long ull;
    struct lh_s  {  long lo; long hi; } lh;
    struct ulh_s {  unsigned lo; unsigned hi; } ulh;
} XCtime_t;

#ifdef __cplusplus
extern "C" {
#endif

//used to compute a 64bit timer (10ns per increments)
long long XCgetTime();
//XCmicros provides a version of the 64bit timer divided by 100 for micro seconds
long long XCmicros();
//XCmicros provides a version of the 64bit timer divided by 100 000 for milli seconds
long long XCmillis();

#ifdef __cplusplus
}
#endif

#endif /* XCGETTIME_H_ */
