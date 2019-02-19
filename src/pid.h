#ifndef PID_H
#define PID_H

class PID {

    float P, I, D, IMAX, IMIN, sum, last;
public:
    PID(float nP, float nI, float nD, float nIMAX, float nIMIN);

    double GetValue(double value, double goal);

    void Reset();

};

#endif