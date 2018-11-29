#ifndef PID_H
#define PID_H

class PID {

    float P, I, D, IMAX, IMIN, sum, last;
public:
    PID(float nP, float nI, float nD, float nIMAX, float nIMIN);

    int GetValue(int value, int goal);

	void reset();

};

#endif