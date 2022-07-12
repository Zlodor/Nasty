#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QApplication>
#include <QString>
#include <cmath>


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
    this->ui->ChartView->setRubberBand(QChartView::HorizontalRubberBand);
    this->ui->ChartView_2->setChart(this->processed_chart);
    this->ui->ChartView_2->setRenderHint(QPainter::Antialiasing);
    this->ui->ChartView_2->setRubberBand(QChartView::HorizontalRubberBand);

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
        QList<QString> tmp = QString(file.readLine()).split(',');
        this->number_of_canals = tmp.length();

        for(int i=0; i<number_of_canals; i++)
            if(tmp[i]!='\n')
                this->input_list.append(QVector<double>{tmp[i].toDouble()});
            else
                this->number_of_canals--;

        while(!file.atEnd())
        {
            tmp = QString(file.readLine()).split(',');
            auto list_point = this->input_list.begin();
            for(auto number = tmp.begin(); number!=tmp.end(); number++, list_point++)
                if(*number != '\n'){
                    list_point->append(number->toDouble());
                }
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

QVector<double> MainWindow::period(QVector<double> _data)
{
    QVector<double> a, b;
    b.append(0.0);
    auto Mean = [](QVector<double> _inf)
    {
        double mean=0.0;
        for(auto &izm: _inf) mean+=izm;
        return mean/_inf.length();
    };
    a.append(Mean(_data));

    const int N = _data.length() / 2;
    const int len = _data.length();
    const double PI = 3.1415;
    for(int i = 1; i < N; i++)
    {
        double p = 0, q = 0;
        for(int j = 1; j < N; j++)
        {
            p += _data[j-1] * cos(2 * PI * i * j / N);
            q += _data[j-1] * sin(2 * PI * i * j / N);
        }
        a.append(2.0 / N * p);
        b.append(2.0 / N * q);
    }

    QVector<double> gramma;
    for(int i=0; i<a.length(); i++)
        gramma.append((pow(a[i],2) + pow(b[i],2)) * N / 2);

    return gramma;
}


void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    if(index == -1) return; //Это нужно т.к. при отчистке комбобокса его индекс устанавливается в -1, и этот слот срабатывает!!

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
    for(int i=from; i<to; i++)
        tmp.append(this->input_list[num_canal][i]);

    this->output.clear();
    this->output = this->process_data(tmp);

    this->gramma = this->period(tmp);
    QLineSeries *gramma_series = new QLineSeries();
    QLineSeries *processed_series = new QLineSeries();

    for(int i=0; i<this->output.length(); i++){
        processed_series->append(i, this->output[i]);
        gramma_series->append(i, gramma[i]);
    }
    this->processed_chart->removeAllSeries();
    this->processed_chart->addSeries(processed_series);
    this->processed_chart->addSeries(gramma_series);
    this->processed_chart->createDefaultAxes();
    this->processed_chart->axes(Qt::Horizontal).back()->setRange(0, this->output.length());
    this->ui->ChartView_2->repaint();
}


void MainWindow::on_pushButton_3_clicked()
{
    if(this->output.isEmpty())
    {
        this->statusBar()->showMessage("Сохранить нечего:(", 3000);
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save File", ".", "*.csv");
    QFile out(fileName + ".csv");

    try
    {
        out.open(QIODevice::WriteOnly);
        QTextStream stream(&out);
        //Чет мне кажется, что в грамм может быть на 1 значение больше. Перестрахуемся.
        int len = output.length() > gramma.length() ? gramma.length() : output.length();
        for(int i=0; i<len; i++)
            stream<<QString::number(output[i])+ ","+ QString::number(gramma[i])<<endl;
        out.close();
        qDebug()<<"Файл сохранён";
    }
    catch (...)
    {
        this->statusBar()->showMessage("Ошибка при сохранении файла", 4000);
    }
}



