#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include "ui_mainwindow.h"
#include "histogram.h"
#include <QtCharts/QChartGlobal>

typedef void(*callback)(QTreeWidgetItem*, long*, long, void*);

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QChartView;
class QChart;
QT_END_NAMESPACE

QT_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_comboBox_currentIndexChanged(int index);

    void on_horizontalSlider_valueChanged(int value);

    void on_horizontalSlider_2_valueChanged(int value);

    void on_checkBox_clicked(bool checked);

    void on_pushButton_2_clicked();

    void on_MainWindow_iconSizeChanged(const QSize &iconSize);

    void on_comboBox_2_currentTextChanged(const QString &arg1);

    void on_comboBox_2_currentIndexChanged(int index);

private:
    void resizeEvent(QResizeEvent*);
    void doResize();

    Ui::MainWindow *ui;
    QFileDialog *fileDialog;
    QString perfFilename = "";
    QString diffFilename = "";
    Histogram histogramDB[1000];
    long nHistogram = 0;
    QChartView *chartView = nullptr;

    int fpDigit = 2;
    int mainTime = -1;
    int chartWidth = 1200;
    int chartHeight = 600;
    void initTreeWidget();
    void dataFetching(QString);
    void dataDiff(QString);
    void histogramSniff();
    void tmaSniff();
    void populateTreeWidget(QString, QString, long);
    void diffTreeWidget(char* info, char* value, long time);
    void iterateTreeLeave(callback, long*, long, void*);
};
#endif // MAINWINDOW_H
