#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QApplication>

class Histogram
{
public:
    Histogram();
    bool empty();
    void insert(int index, long value);
    void clear();
    void setStep(int);
    QString title;
    int size = 0;
    int step = 0;
    long data[1000];
private:
};

#endif // HISTOGRAM_H
