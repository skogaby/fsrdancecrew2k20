#include "RunningSum.h"
#include <stdlib.h>

RunningSum::RunningSum(int n) {
    _size = n;
    _ar = (int*) malloc(_size * sizeof(int));
    if (_ar == NULL) _size = 0;
    clear();
}

RunningSum::~RunningSum() {
    if (_ar != NULL) free(_ar);
}

void RunningSum::clear() {
    _cnt = 0;
    _idx = 0;
    _sum = 0.0;
    for (int i = 0; i< _size; i++) _ar[i] = 0;
}

void RunningSum::addValue(int f) {
    if (_ar == NULL) return;
    _sum -= _ar[_idx];
    _ar[_idx] = f;
    _sum += f;
    _idx++;
    if (_idx == _size) _idx = 0;
    if (_cnt < _size) _cnt++;
}

int RunningSum::getElement(uint8_t idx) {
    if (idx >=_cnt ) return NAN;
    return _ar[idx];
}

int RunningSum::getLastElement()
{
  // if we haven't inserted at least 1 element, just return 0
  if (_cnt > 0) {
    if (_idx > 0) {
      return _ar[_idx - 1];
    } else {
      return _ar[_size - 1];
    }
  } else {
    return 0;
  }
}

int RunningSum::getLatestDelta()
{
  // if we haven't inserted at least 2 elements, just return 0
  if (_cnt > 1) {
    if (_idx > 0) {
      return _ar[_idx] - _ar[_idx - 1];
    } else {
      return _ar[_idx] - _ar[_size - 1];
    }
  } else {
    return 0;
  }
}
