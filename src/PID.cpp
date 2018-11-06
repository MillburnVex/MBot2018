#include "../include/main.h"
#include "PID.h"

PID::PID(float nP, float nI, float nD, float nIMAX, float nIMIN) :
	P(nP), I(nI), D(nD), IMAX(nIMAX), IMIN(nIMIN), sum(0), last(0)
{
}

int PID::GetValue(int value, int goal) {
	int error = goal - value;
	int pVal = error * P;
	sum += error;
	if (sum > IMAX) sum = IMAX;
	if (sum < IMIN) sum = IMIN;
	int iVal = I * sum;
	int dVal = D * (value - last);
	last = value;
	int newval = pVal + iVal - dVal;

	//if(P == 3.0f && goal!=-1) printf("goal: %d, current: %d, P: %d, I:%d D: %d\n", goal,
	//	                         value, pVal, iVal, dVal);
	return newval;
}