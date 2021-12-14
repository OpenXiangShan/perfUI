#include "histogram.h"

Histogram::Histogram()
{
    clear();
}

void Histogram::clear() {
    size = 0;
    for (int i = 0; i < 1000; i++) {
        data[i] = 0;
    }
}

void Histogram::insert(int index, long value) {
    data[index] = value;
    size = (index > size) ? index : size;
}

void Histogram::setStep(int a) {
    step = a;
}
