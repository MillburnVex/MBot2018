#include "../include/main.h"
#include "PID.h"

PID::PID(float nP, float nI, float nD, float nIMAX, float nIMIN) :
        P(nP), I(nI), D(nD), IMAX(nIMAX), IMIN(nIMIN), sum(0), last(0) {
}

double PID::GetValue(double value, double goal) {
    double error = goal - value;
    double pVal = error * P;
    sum += error;
    if (sum > IMAX) sum = IMAX;
    if (sum < IMIN) sum = IMIN;
    double iVal = I * sum;
    double dVal = D * (value - last);
    last = value;
    return pVal + iVal - dVal;
}

void PID::Reset() {
    sum = 0;
    last = 0;
}