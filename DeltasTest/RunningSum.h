#ifndef RunningSum_h
#define RunningSum_h

#include "Arduino.h"

class RunningSum
{
public:
    RunningSum(void);
    RunningSum(int);
    ~RunningSum();

    void clear();
    void addValue(int);

    int getElement(uint8_t idx);
    int getLastElement();
    int getLatestDelta();
    uint8_t getSize() { return _size; }
    uint8_t getCount() { return _cnt; }
    int getSum() { return _sum; }

protected:
    uint8_t _size;
    uint8_t _cnt;
    uint8_t _idx;
    int     _sum;
    int * _ar;
};

#endif
