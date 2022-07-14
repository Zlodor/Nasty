#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QDebug>
#include <QLineSeries>
#include <QtCharts>
#include <QChartView>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    QVector<double> process_data(QVector<double> _data);

    QVector<double> period(QVector<double> _data);

    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();


    void on_checkBox_clicked();

    void on_checkBox_2_clicked();

    void on_action2_triggered();

    void on_action3_triggered();

    void on_action4_triggered();

    void on_action5_triggered();

    void on_action6_triggered();

    void on_action7_triggered();

    void on_action8_triggered();

    void on_action1_triggered();

private:
    Ui::MainWindow *ui;

    int number_of_canals = 0;
    int number_of_samples = 0;

    QFile file;
    QList<QVector<double>> input_list;
//    QList<QVector<double>> output_list;
    QVector<double> output;
    QVector<double> gramma;
    QVector<double> minses;
    QVector<double> maxses;

    QList<QLineSeries*> list_of_input_series;
    QList<QLineSeries*> processed_series;
    QLineSeries *raw_series;
    QChart *raw_chart, *processed_chart;
};
#endif // MAINWINDOW_H
