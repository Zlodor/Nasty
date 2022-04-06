#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QApplication>
#include <QString>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->raw_chart = new QChart();
    this->processed_chart = new QChart();

    this->raw_chart->legend()->hide();
    this->raw_chart->setTitle("Raw data");

    this->processed_chart->legend()->hide();
    this->processed_chart->setTitle("Processed data");

    this->ui->ChartView->setChart(this->raw_chart);
    this->ui->ChartView->setRenderHint(QPainter::Antialiasing);
    this->ui->ChartView_2->setChart(this->processed_chart);
    this->ui->ChartView_2->setRenderHint(QPainter::Antialiasing);

    this->ui->comboBox->setEnabled(false);
    this->ui->spinBox->setEnabled(false);
    this->ui->spinBox_2->setEnabled(false);
    this->ui->pushButton_2->setEnabled(false);
}

MainWindow::~MainWindow()
{
    if(this->file.isOpen()) this->file.close();
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(0, "Open File", ".", "*.csv");
    try
    {
        this->file.setFileName(fileName);
        this->file.open(QIODevice::ReadOnly | QIODevice::Text);
        this->input_list.clear();
        qDebug()<<"Чтение файла...";
        QList<QByteArray> tmp = file.readLine().split(',');
        this->number_of_canals = tmp.length();
        for(int i=0; i<number_of_canals; i++)
            this->input_list.append(QVector<double>{tmp[i].toDouble()});
        while(!file.atEnd())
        {
            tmp = file.readLine().split(',');
            auto list_point = this->input_list.begin();
            for(auto number = tmp.begin(); number!=tmp.end(); number++, list_point++)
                list_point->append(number->toDouble());
        }

        this->number_of_samples = this->input_list[0].length();

        //Добавим номера каналов в комбобокс
        this->ui->comboBox->clear();
        for(int i=1; i<=this->number_of_canals; i++)
            this->ui->comboBox->addItem(QString::number(i));

        this->ui->spinBox->setMaximum(number_of_samples);
        this->ui->spinBox_2->setMaximum(number_of_samples);
        this->ui->comboBox->setEnabled(true);
        this->ui->spinBox->setEnabled(true);
        this->ui->spinBox_2->setEnabled(true);
        this->ui->pushButton_2->setEnabled(true);
        qDebug()<<"Чтение файла завершено."<<"Прочитано:"<<this->number_of_samples<<"строк.";
    }
    catch (...)
    {
        this->statusBar()->showMessage("Ошибка при открытии файла", 4000);
        this->ui->comboBox->setEnabled(false);
        this->ui->spinBox->setEnabled(false);
        this->ui->spinBox_2->setEnabled(false);
        this->ui->pushButton_2->setEnabled(false);
    }

    //Пока для теста сделаем подсчет и отрисовку графика здесь
//    auto vec = this->process_data(this->input_list[0]);
//    this->processed_series = new QLineSeries();
//    for(int i=0; i<vec.length(); i++)
//        this->processed_series->append(i, vec[i]);
//    this->processed_chart->removeAllSeries();
//    this->processed_chart->addSeries(this->processed_series);
//    this->processed_chart->createDefaultAxes();
//    this->ui->ChartView_2->repaint();
}


QVector<double> MainWindow::process_data(QVector<double> _data)
{
    QVector<double> output;
    QVector<double> average;

    for(int m=1; m<=(_data.length()/2); m++)
    {
        average.clear();
        int p = _data.length() / m;
        double accumulator = 0.0;
        for(int n=0; n<m; n++)
        {
            for(int i=0; i<p; i++)
                accumulator+=_data[n+i*m];

            average.append(accumulator / p);
        }
        double min = *std::min_element(average.begin(), average.end());
        double max = *std::max_element(average.begin(), average.end());
        output.append((max-min) / (2*p));
    }
    return output;
}


void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    QLineSeries *series = new QLineSeries();
    QVector<double> vec = this->input_list[index];
    for(int i=0; i<vec.length(); i++)
        series->append(i, vec[i]);
    this->raw_chart->removeAllSeries();
    this->raw_chart->addSeries(series);
    this->raw_chart->createDefaultAxes();
    this->raw_chart->axes(Qt::Horizontal).back()->setRange(0, number_of_samples);
    this->ui->ChartView->repaint();
}


void MainWindow::on_pushButton_2_clicked()
{
    int from = this->ui->spinBox->value();
    int to = this->ui->spinBox_2->value();
    if(from >= to)
    {
        this->statusBar()->showMessage("Неверный промежуток", 3000);
        return;
    }

    QVector<double> tmp;
    int num_canal = this->ui->comboBox->currentIndex();
    for(int i=from; i<=to; i++)
        tmp.append(this->input_list[num_canal][i]);

    this->output.clear();
    this->output = this->process_data(tmp);

    this->processed_series = new QLineSeries();
    for(int i=0; i<this->output.length(); i++)
        this->processed_series->append(i, this->output[i]);
    this->processed_chart->removeAllSeries();
    this->processed_chart->addSeries(this->processed_series);
    this->processed_chart->createDefaultAxes();
    this->ui->ChartView_2->repaint();
}

