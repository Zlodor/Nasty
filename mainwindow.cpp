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

    //Добавляем последовательности для каждого алгоритма обработки
    this->processed_series.append(new QLineSeries());
    this->processed_series.append(new QLineSeries());
    this->processed_chart->addSeries((*processed_series.begin()));
    this->processed_chart->addSeries((*(processed_series.begin()+1)));
    this->processed_chart->createDefaultAxes();

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
            accumulator = 0;
        }
        double min = *std::min_element(average.begin(), average.end());
        double max = *std::max_element(average.begin(), average.end());
        output.append((max-min) / (2*p));
    }
    return output;
}

//QVector<double> MainWindow::period(QVector<double> _data)
//{
//    QVector<double> a, b;

//    auto Mean = [](QVector<double> _inf)
//    {
//        double mean=0.0;
//        for(auto &izm: _inf) mean+=izm;
//        return mean/_inf.length();
//    };
//    a.append(Mean(_data));
//    b.append(0.0);

//    const int N = _data.length() / 2;
//    const double PI = 3.1415;
//    for(int i = 1; i <= N; i++)
//    {
//        double p = 0, q = 0;
//        for(int j = 1; j <= N; j++)
//        {
//            p += _data[j-1] * cos(2 * PI * i * j / N);
//            q += _data[j-1] * sin(2 * PI * i * j / N);
//        }
//        a.append(2.0 / N * p);
//        b.append(2.0 / N * q);
//    }

//    QVector<double> gramma;
//    for(int i=0; i<a.length(); i++)
//        gramma.append((pow(a[i],2) + pow(b[i],2)) * N / 2);

//    return gramma;
//}


QVector<double> MainWindow::period(QVector<double> _data)
{
    QVector<double> a, b;
    QVector<double> gramma;

    int N = _data.length();
    if((N % 2) == 1) N--;
    int n = N / 2;

    const double PI = 3.1415;
    for(int k = 1; k <= n-1; k++)
    {
        double fk = static_cast<double>(k) / N;
        double p = 0, q = 0;
        for(int i = 0; i < _data.length(); i++)
        {
            p += _data[i] * cos(2 * PI * i * fk);
            q += _data[i] * sin(2 * PI * i * fk);
        }
        a.append(2.0 / N * p);
        b.append(2.0 / N * q);
    }


    for(int i=0; i<a.length(); i++)
        gramma.append(pow(a[i],2) + pow(b[i],2));

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

    //Запомним минимумы и максимумы
    this->minses.clear();
    this->maxses.clear();
    this->minses.append(*std::min_element(output.begin(), output.end()));
    this->minses.append(*std::min_element(gramma.begin(), gramma.end()));
    this->maxses.append(*std::max_element(output.begin(), output.end()));
    this->maxses.append(*std::max_element(gramma.begin(), gramma.end()));

    auto first_alg = this->processed_series.begin();
    auto secend_alg = this->processed_series.begin()+1;
    (*first_alg)->clear();
    (*secend_alg)->clear();

    for(int i=0; i<this->output.length(); i++)
        (*first_alg)->append(i, this->output[i]);
    for(int i=0; i<this->gramma.length(); i++)
        (*secend_alg)->append(i, gramma[i]);
    this->processed_chart->axes(Qt::Horizontal).back()->setRange(0, this->output.length());
    this->ui->ChartView_2->repaint();
    this->on_checkBox_2_clicked();  //Для перерисовки оси Y
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


void MainWindow::on_checkBox_clicked()
{
    double min=0, max=0;
    if(this->ui->checkBox->isChecked())
    {
        (*this->processed_series.begin())->setVisible(true);
        if(this->ui->checkBox_2->isChecked())
        {
            min = this->minses[0] > this->minses[1] ? minses[1] : minses[0];
            max = this->maxses[0] > this->maxses[1] ? maxses[0] : maxses[1];
        }
        else
        {
            min = this->minses[0];
            max = this->maxses[0];
        }
    }
    else
    {
        (*this->processed_series.begin())->setVisible(false);
        min = this->minses[1];
        max = this->maxses[1];
    }
    this->processed_chart->axes(Qt::Vertical).back()->setRange(min, max);
}


void MainWindow::on_checkBox_2_clicked()
{
    double min=0, max=0;
    if(this->ui->checkBox_2->isChecked())
    {
        (*(this->processed_series.begin()+1))->setVisible(true);
        if(this->ui->checkBox->isChecked())
        {
            min = this->minses[0] > this->minses[1] ? minses[1] : minses[0];
            max = this->maxses[0] > this->maxses[1] ? maxses[0] : maxses[1];
        }
        else
        {
            min = this->minses[1];
            max = this->maxses[1];
        }
    }
    else
    {
        (*(this->processed_series.begin()+1))->setVisible(false);
        min = this->minses[0];
        max = this->maxses[0];
    }
    this->processed_chart->axes(Qt::Vertical).back()->setRange(min, max);
}


void MainWindow::on_action2_triggered()
{
    this->raw_chart->setTheme(QChart::ChartThemeLight);
    this->processed_chart->setTheme(QChart::ChartThemeLight);
}


void MainWindow::on_action3_triggered()
{
    this->raw_chart->setTheme(QChart::ChartThemeBlueCerulean);
    this->processed_chart->setTheme(QChart::ChartThemeBlueCerulean);
}


void MainWindow::on_action4_triggered()
{
    this->raw_chart->setTheme(QChart::ChartThemeDark);
    this->processed_chart->setTheme(QChart::ChartThemeDark);
}


void MainWindow::on_action5_triggered()
{
    this->raw_chart->setTheme(QChart::ChartThemeBrownSand);
    this->processed_chart->setTheme(QChart::ChartThemeBrownSand);
}


void MainWindow::on_action6_triggered()
{
    this->raw_chart->setTheme(QChart::ChartThemeBlueNcs);
    this->processed_chart->setTheme(QChart::ChartThemeBlueNcs);
}


void MainWindow::on_action7_triggered()
{
    this->raw_chart->setTheme(QChart::ChartThemeHighContrast);
    this->processed_chart->setTheme(QChart::ChartThemeHighContrast);
}


void MainWindow::on_action8_triggered()
{
    this->raw_chart->setTheme(QChart::ChartThemeBlueIcy);
    this->processed_chart->setTheme(QChart::ChartThemeBlueIcy);
}


void MainWindow::on_action1_triggered()
{
    this->raw_chart->setTheme(QChart::ChartThemeQt);
    this->processed_chart->setTheme(QChart::ChartThemeQt);
}

