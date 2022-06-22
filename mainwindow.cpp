#include "mainwindow.h"
#include <QtDebug>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSet>
#include <QtCharts/qbarseries.h>
#include <QtCharts/qvalueaxis.h>
#include <QBarCategoryAxis>
#include <QRegExp>
#pragma push_macro("slots")
#undef slots
#include "Python.h"
#pragma pop_macro("slots")

void MainWindow::initTreeWidget() {
    QTreeWidgetItem *new_item;
    new_item = new QTreeWidgetItem();
    new_item->setText(0, "TOP");
    ui->treeWidget->clear();
    ui->treeWidget->insertTopLevelItem(0, new_item);
}

void MainWindow::populateTreeWidget(QString info, QString value, long time) {
    QString item = info;
    QStringList items = item.split(".");
    if (items.first() != "TOP") {
        // QMessageBox::information(NULL, "Error", "Parser error! Code = 0", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    /* items.removeFirst();
    if (items.first() != "XSSimSoC") {
        QMessageBox::information(NULL, "Error", "Parser error! Code = 1", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    items.removeFirst();
    if (items.first() != "soc") {
        QMessageBox::information(NULL, "Error", "Parser error! Code = 2", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }*/
    items.removeFirst();
    QStringListIterator itemIterator(items);
    QTreeWidgetItem *root = ui->treeWidget->topLevelItem(0);
    while (itemIterator.hasNext()) {
        QString node = itemIterator.next();
        int flag = 0;
        for (int i = 0; i < root->childCount(); i++) {
            if (root->child(i)->text(0) == node) {
                root = root->child(i);
                flag = 1;
                break;
            }
        }
        if (flag) continue;
        QTreeWidgetItem *child = new QTreeWidgetItem();

        if (!itemIterator.hasNext()) {
            long valueInt = value.toLong();
            float valueAverage = (float)valueInt / time;

            child->setText(0, node);
            child->setText(1, value);
            child->setText(2, QString::number(valueAverage, 'f', fpDigit));
            child->setTextAlignment(1, Qt::AlignRight);
            child->setTextAlignment(2, Qt::AlignRight);
            child->setTextAlignment(3, Qt::AlignRight);
        } else {
            child->setText(0, node);
        }
        root->insertChild(root->childCount(), child);
        root = child;
    }
}

void MainWindow::diffTreeWidget(char* info, char* value, long time) {
    QString item(info);
    QStringList items = item.split(".");
    if (items.first() != "TOP") {
        // QMessageBox::information(NULL, "Error", "Parser error! Code = 0", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    /*items.removeFirst();
    if (items.first() != "XSSimSoC") {
        QMessageBox::information(NULL, "Error", "Parser error! Code = 1", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    items.removeFirst();
    if (items.first() != "soc") {
        QMessageBox::information(NULL, "Error", "Parser error! Code = 2", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }*/

    items.removeFirst();

    QStringListIterator itemIterator(items);
    QTreeWidgetItem *root = ui->treeWidget->topLevelItem(0);
    while (itemIterator.hasNext()) {
        QString node = itemIterator.next();
        int flag = 0;
        for (int i = 0; i < root->childCount(); i++) {
            if (root->child(i)->text(0) == node) {
                root = root->child(i);
                if (!itemIterator.hasNext()) {
                    if (root->childCount() != 0) {
                        // QMessageBox::information(NULL, "Error", "Trunk node becoming leaf is not allowed", QMessageBox::Ok, QMessageBox::Ok);
                        return;
                    }
                    long diff = root->text(1).toLong() - atol(value);
                    if (diff == 0) {
                        root->setText(3, "=");
                    } else if (diff > 0) {
                        root->setText(3, "+" + QString::number(diff));
                    } else {
                        root->setText(3, QString::number(diff));
                    }
                    return;
                }
                flag = 1;
                break;
            }
        }
        if (flag) continue;
        if (root->childCount() == 0) {  // root used up, but info not
            // QMessageBox::information(NULL, "Error", "Leaf node becoming trunk is not allowed", QMessageBox::Ok, QMessageBox::Ok);
            return;
        }
        if (itemIterator.hasNext()) {  // no child matched && info not used up
            QTreeWidgetItem *child = new QTreeWidgetItem();
            child->setText(0, "#NEW# "+node);
            root->insertChild(root->childCount(), child);
            root = child;
        } else {

        }
    }
}

void MainWindow::dataFetching(QString perfFilename) {
    QFile file;
    file.setFileName(perfFilename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::information(NULL, "Error", "File not found!", QMessageBox::Ok, QMessageBox::Ok);
    }
    Py_Initialize();
    if (!Py_IsInitialized()) {
        QMessageBox::information(NULL, "Error", "Python connection failed!", QMessageBox::Ok, QMessageBox::Ok);
        Py_Finalize();
        return;
    }
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");
    PyObject *pModule = PyImport_ImportModule("perfdiff");
    if (!pModule) {
        QMessageBox::information(NULL, "Error", "Python file not found!", QMessageBox::Ok, QMessageBox::Ok);
        Py_Finalize();
        return;
    }
    PyObject_CallMethod(pModule, "initDB", "");
    while(!file.atEnd()) {
        QByteArray line = file.readLine();
        char* str = line.data();
        if (mainTime == -1) {
            QString time;
            /*
              PyObject* pRet = PyObject_CallMethod(pModule, "parseTime", "s", str);
              PyArg_Parse(pRet, "s", &time);
            */
            QRegExp rx("^*\\[PERF \\]\\[time=\\s*(\\d*)\\]*");
            if (rx.indexIn(str) == -1) {
                qDebug() << "Parse time error!";
                return;
            }
            time = rx.cap(1);
            mainTime = time.toLong();
        }
        QString itemName;
        QString value;
        /*
        PyObject* pRet = PyObject_CallMethod(pModule, "parseLine", "si", str, 1);
        PyArg_ParseTuple(pRet, "ss", &itemName, &value);
        */
        QRegExp rx("^*\\[PERF \\]\\[time=\\s*\\d*\\] ((\\w*(\\.|))*): (\\w*)\\s*,\\s*(\\d*)");
        if (rx.indexIn(str) == -1) {
            qDebug() << "Parse line error!";
            qDebug() << str;
            return;
        } else {
            itemName = rx.cap(1) + "." + rx.cap(4);
            value = rx.cap(5);
        }
        populateTreeWidget(itemName, value, mainTime);
    }
    // PyObject_CallMethod(pModule, "dumpPerf", "");
    // Py_Finalize();
}

void MainWindow::dataDiff(QString diffFilename) {
    QFile file;
    file.setFileName(diffFilename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::information(NULL, "Error", "File not found!", QMessageBox::Ok, QMessageBox::Ok);
    }
    PyObject *pModule = PyImport_ImportModule("perfdiff");
    while(!file.atEnd()) {
        QByteArray line = file.readLine();
        char* str = line.data();
        if (mainTime == -1) {
            char* time;
            PyObject* pRet = PyObject_CallMethod(pModule, "parseTime", "s", str);
            PyArg_Parse(pRet, "s", &time);
            mainTime = atoi(time);
        }
        PyObject* pRet = PyObject_CallMethod(pModule, "parseLine", "si", str, 0);
        char* itemName;
        char* value;
        PyArg_ParseTuple(pRet, "ss", &itemName, &value);
        diffTreeWidget(itemName, value, mainTime);
    }
}

void itemDFS(QTreeWidgetItem* node, long* arg1, int arg2, void* arg3, callback cb) {
    if (node->childCount() == 0) {
        cb(node, arg1, arg2, arg3);
        return;
    } else {
        for (int i = 0; i < node->childCount(); i++) {
            itemDFS(node->child(i), arg1, arg2, arg3, cb);
        }
    }
}

void MainWindow::iterateTreeLeave(callback cb, long* arg1, long arg2, void* arg3) {
    QTreeWidgetItem* root = ui->treeWidget->topLevelItem(0);
    if (root == nullptr) return;
    itemDFS(root, arg1, arg2, arg3, cb);
}

void MainWindow::doResize() {
    ui->treeWidget->setFixedHeight(MainWindow::height() - 200);
    ui->treeWidget->setFixedWidth(MainWindow::width() - 200);
    ui->tabWidget->setFixedHeight(MainWindow::height() - 50);
    ui->tabWidget->setFixedWidth(MainWindow::width() - 50);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    doResize();
    fileDialog = new QFileDialog(this);
    Histogram* histogramDB = new Histogram[200];
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent*) {
    doResize();
}

void loadHistogram(QTreeWidgetItem* node, long* arg1, long arg2, void* arg3) {
    QRegExp rx("^*_\\d+_\\d+");
    if (rx.indexIn(node->text(0)) == -1) {
        return;
    }
    qDebug() << node->text(0);
    QStringList info = node->text(0).split("_");
    int highBound = info.last().toInt();
    info.removeLast();
    int lowBound = info.last().toInt();
    info.removeLast();
    int step = highBound - lowBound;
    if (step <= 0) {
        return;
    }
    QString leafName = info.join("");
    QStringList* fullNameList = new QStringList();
    fullNameList->clear();
    fullNameList->append(leafName);
    QTreeWidgetItem* orphan = node;
    while (orphan->parent()) {
        orphan = orphan->parent();
        fullNameList->append(orphan->text(0));
    }
    QStringList* fullNameListReverse = new QStringList();
    for (int i = fullNameList->size()-1; i >= 0; i--) {
        fullNameListReverse->append(fullNameList->at(i));
    }
    QString histogramName = fullNameListReverse->join(".");

    int index = lowBound / step;
    Histogram* histogramDB = (Histogram*)arg3;
    for (int i = 0; i < 1000; i++) {
        if (histogramDB[i].title == histogramName) {
            histogramDB[i].insert(index, node->text(1).toLong());
            return;
        }
    }
    histogramDB[*arg1].title = histogramName;
    histogramDB[*arg1].setStep(step);
    histogramDB[*arg1].insert(index, node->text(1).toLong());
    *arg1 += 1;
}

void MainWindow::histogramSniff() {
    iterateTreeLeave(loadHistogram, &nHistogram, 0, histogramDB);
    for (int i = 0; i < 200; i++) {
        if (histogramDB[i].title != "") {
            ui->comboBox_2->addItem(histogramDB[i].title);
        }
    }
}

void MainWindow::tmaSniff() {
    PyObject *pModule = PyImport_ImportModule("perfdiff");
    PyObject_CallMethod(pModule, "filterTMA", "");
}

void setFP(QTreeWidgetItem* node, long* arg1, long arg2, void* arg3) {
    long valueInt = node->text(1).toLong();
    float valueAverage = (float)valueInt / arg2;
    node->setText(2, QString::number(valueAverage, 'f', *arg1));
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    fpDigit = index;
    long longIndex = index;
    iterateTreeLeave(setFP, &longIndex, mainTime, nullptr);
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    chartView->resize(chartWidth+(ui->horizontalSlider_2->value()-50)*50, chartHeight+(ui->horizontalSlider->value()-50)*20);
    chartView->show();
}

void MainWindow::on_horizontalSlider_2_valueChanged(int value)
{
    chartView->resize(chartWidth+(ui->horizontalSlider_2->value()-50)*50, chartHeight+(ui->horizontalSlider->value()-50)*20);
    chartView->show();
}

void itemDelDFS(QTreeWidgetItem* node) {
    QRegExp rx("^*_\\d+_\\d+");
    for (int i = 0; i < node->childCount(); i++) {
        if (node->child(i)->childCount() != 0) {
            itemDelDFS(node->child(i));
        } else {  // child is leaf
            if (rx.indexIn(node->child(i)->text(0)) != -1) {
                node->removeChild(node->child(i));
                itemDelDFS(node);
                return;
            }
        }
    }
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    if (!checked) {
        initTreeWidget();
        mainTime = -1;
        dataFetching(perfFilename);
        return;
    }
    if (perfFilename == "") return;
    itemDelDFS(ui->treeWidget->topLevelItem(0));
}

void MainWindow::on_pushButton_clicked()
{
    perfFilename = QFileDialog::getOpenFileName(this, "open file dialog",
                   "~/",
                   "");
    if (perfFilename == "") {
        return;
    }
    ui->textBrowser->setText(perfFilename);
    initTreeWidget();
    mainTime = -1;
    dataFetching(perfFilename);
    histogramSniff();
    // tmaSniff();  /* TODO: do TMA sniff manually in TMA tab */
    ui->checkBox->setChecked(false);
    on_checkBox_clicked(false);
}

void MainWindow::on_pushButton_2_clicked()
{
    if (perfFilename == "") {
        QMessageBox::information(NULL, "Error", "Please assign main perf file first!", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    diffFilename = QFileDialog::getOpenFileName(this, "open file dialog",
                   "~/",
                   "");
    if (diffFilename == "") {
        return;
    }
    ui->textBrowser_2->setText(diffFilename);
    ui->checkBox->setChecked(false);
    on_checkBox_clicked(false);
    dataDiff(diffFilename);

}

void MainWindow::on_comboBox_2_currentTextChanged(const QString &arg1)
{
    if (chartView == nullptr) {
        chartView = new QChartView();
    }
    for (int i = 0; i < 200; i++) {
        if (histogramDB[i].title == arg1) {
            QBarSet* set0 = new QBarSet(arg1);
            long sum = 0;
            for (int j = 0; j <= histogramDB[i].size; j++) {
                sum += histogramDB[i].data[j];
            }
            for (int j = 0; j <= histogramDB[i].size; j++) {
                *set0 << (float)histogramDB[i].data[j]/sum;
            }
            QBarSeries *series = new QBarSeries();
            series->append(set0);
            series->setLabelsFormat("@valueh");
            QChart* chart = new QChart();
            chart->addSeries(series);
            chart->createDefaultAxes();

            QBarCategoryAxis *axisX = new QBarCategoryAxis();
            QStringList strListX;
            for (int k = 0; k <= (histogramDB[i].size+1)*histogramDB[i].step; k += histogramDB[i].step) {
                strListX.append(QString::number(k));
            }
            axisX->append(strListX);
            chart->setAxisX(axisX, series);

            chartView->setChart(chart);
            chartView->resize(chartWidth, chartHeight);
            chartView->move(40, 80);
            chartView->setParent(ui->tab_3);
            chartView->resize(chartWidth+(ui->horizontalSlider_2->value()-50)*50, chartHeight+(ui->horizontalSlider->value()-50)*20);
            chartView->show();
            return;
        }
    }
}


void MainWindow::on_MainWindow_iconSizeChanged(const QSize &iconSize)
{

}
