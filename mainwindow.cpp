#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QApplication>
#include <QString>
#include <QDebug>
#include <QLineSeries>
#include <QtCharts>
#include <QChartView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Построить ряд как источник данных диаграммы
    QLineSeries *series = new QLineSeries();
    series->append(0, 5);
    series->append(4, 10);
    series->append(8, 6);
    *series << QPointF(13, 5) << QPointF(17, 6) << QPointF(20, 2);

    QLineSeries *series2 = new QLineSeries();
    series2->append(0, 5);
    series2->append(4, 10);
    series2->append(8, 6);
//    *series2 << QPointF(13, 5) << QPointF(17, 6) << QPointF(20, 2);

    // Построить график
    QChart *chart = new QChart();
    chart->legend()->hide();  // скрыть легенду
    chart->addSeries(series);  // добавить серию на график
    chart->createDefaultAxes();  // Создать ось на основе серии, добавленной к диаграмме
    chart->setTitle("Raw data");  // Устанавливаем заголовок графика

    QChart *chart2 = new QChart();
    chart2->legend()->hide();  // скрыть легенду
    chart2->addSeries(series2);  // добавить серию на график
    chart2->createDefaultAxes();  // Создать ось на основе серии, добавленной к диаграмме
    chart2->setTitle("Processed data");  // Устанавливаем заголовок графика

    this->ui->ChartView->setChart(chart);
    this->ui->ChartView->setRenderHint(QPainter::Antialiasing);
    this->ui->ChartView_2->setChart(chart2);
    this->ui->ChartView_2->setRenderHint(QPainter::Antialiasing);
}

MainWindow::~MainWindow()
{
    if(this->file.isOpen()) this->file.close();
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(0, "Open File", ".", "*.csv");
    qDebug()<<fileName;
    try
    {
        this->file.setFileName(fileName);
        this->file.open(QIODevice::ReadOnly | QIODevice::Text);
        this->input_list.clear();
        qDebug()<<"Чтение файла...";
        QList<QByteArray> tmp = file.readLine().split(',');
        for(int i=0; i<tmp.length(); i++)
            this->input_list.append(QVector<double>{tmp[i].toDouble()});
        while(!file.atEnd())
        {
            tmp = file.readLine().split(',');
            auto list_point = this->input_list.begin();
            for(auto number = tmp.begin(); number!=tmp.end(); number++, list_point++)
                list_point->append(number->toDouble());
        }
        qDebug()<<"Чтение файла завершено."<<"Прочитано:"<<this->input_list[0].length()<<"строк.";
    }
    catch (...)
    {
        this->statusBar()->showMessage("Ошибка при открытии файла", 4000);
    }
}


QVector<double> MainWindow::process_data(QVector<double> data)
{
    QVector<double> output;

    return output;
}

