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

    this->ui->spinBox_3->setEnabled(false);
    this->ui->spinBox_4->setEnabled(false);

    //Добавляем последовательности для каждого алгоритма обработки
    this->processed_series.append(new QLineSeries());
    this->processed_series.append(new QLineSeries());
    this->processed_chart->addSeries((*processed_series.begin()));
    this->processed_chart->addSeries((*(processed_series.begin()+1)));
    this->processed_chart->createDefaultAxes();




//    connect(this->ui->ChartView_2, &QtCharts::QChartView::mouseMoveEvent, this, [](QMouseEvent *e){qDebug()<<"YES";});

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

    for(int m=1; m<=(_data.length()/2); m++) //Цикл от 1 до N/2
    {
        average.clear();                //Отчищаем массив под ср. знач. столбцов
        int p = _data.length() / m;     //Расчитываем кол-во строк в матрице
        double accumulator = 0.0;       //Здесь накапоивается ср.
        for(int n=0; n<m; n++)          //Цикл по столбцам
        {
            for(int i=0; i<p; i++)      //Цикл по строкам
                accumulator+=_data[n+i*m];  //Суммируем элементы столбцов

            average.append(accumulator / p);    //Добавляем в массив ср. знач. усреднённую сумму
            accumulator = 0;                //Обнуляем накопитель
        }
        double min = *std::min_element(average.begin(), average.end()); //Находим min
        double max = *std::max_element(average.begin(), average.end()); //Находим max
        output.append((max-min) / (m*p));   //Добавляем точку на графике для отрезка длиной m
    }

//    this->ui->spinBox_3->setEnabled(true);
//    this->ui->spinBox_4->setEnabled(true);
//    this->ui->spinBox_3->setMaximum(output.size());
//    this->ui->spinBox_4->setMaximum(output.size());

    return output;
}


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

//    this->ui->spinBox_3->setEnabled(true);
//    this->ui->spinBox_4->setEnabled(true);
//    this->ui->spinBox_3->setMaximum(gramma.size());
//    this->ui->spinBox_4->setMaximum(gramma.size());

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
    this->raw_chart->axes(Qt::Horizontal).back()->setRange(0+1, number_of_samples);
    this->ui->ChartView->repaint();
}


//Верхняя кнопка "Обработать"
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
    {
        double t = static_cast<double>(tmp.length())/(i+1);
        (*secend_alg)->append(t, gramma[i]);
//        (*secend_alg)->append(i, gramma[i]);
    }
    this->processed_chart->axes(Qt::Horizontal).back()->setRange(0+1, this->output.length());
    this->ui->ChartView_2->repaint();
    this->on_checkBox_2_clicked();  //Для перерисовки оси Y

    this->ui->spinBox_3->setEnabled(true);
    this->ui->spinBox_4->setEnabled(true);
    this->ui->spinBox_3->setMaximum(output.size());
    this->ui->spinBox_4->setMaximum(output.size());
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
            stream<<QString::number(i+1) + ',' + QString::number(output[i]) + ','
                    + QString::number(double(gramma.length()*2)/(i+1)) + ',' + QString::number(gramma[i])<<endl;
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


void MainWindow::on_pushButton_4_clicked()
{
    int from = this->ui->spinBox_3->value();
    int to = this->ui->spinBox_4->value();
    if(from >= to)
    {
        this->statusBar()->showMessage("Неверный промежуток", 3000);
        return;
    }

    QVector<double> segment;        //Анализируемый отрезок
    //Вырезаем нужный кусок
    for(int i=from; i<=to; i++)
        segment.push_back(this->gramma[i]);
    int lenght = segment.size();     //Длина отрезка (кол-во элементов)

    //Подсчитаем кол-во попаданий каждого значения в отрезок
    std::map<double, int> counts;
    for(const double& element : segment)
    {
        if(counts.find(element) == counts.end())
            counts[element] = 1;
        else
            counts[element]++;
    }
    double M = 0.0;   //Мат. ожидание
    for(const auto& it : counts)
        M += it.first*it.second;
    M/=lenght;


    double D = 0.0;     //Дисперсия
    double m3 = 0.0;    //Момент 3-го порядка
    double m4 = 0.0;    //Момент 4-го порядка
    for(const auto& it : counts)
    {
       double ver = (double)it.second/lenght;
       D += pow((it.first - M),2) * ver;
       m3 +=pow((it.first - M),3) * ver;
       m4 +=pow((it.first - M),4) * ver;
    }

    double sko = sqrt(D);       //СКО
    double A = m3/pow(sko,3);   //Коэф. асимметрии
    double E = m4/pow(sko,4);   //Коэф. эксцесса

    QString msg = "Мат. ожидание M = " + QString::number(M) +
                    "\nДисперсия D = " + QString::number(D) +
                    "\nСКО S = " + QString::number(sko) +
                    "\nКоэф. асимметрии A = " + QString::number(A) +
                    "\nКоэф. эксцесса E = " + QString::number(E);


    QFont font;
    font.setPointSizeF(12);
    QMessageBox msgBox;
    msgBox.setFont(font);
    msgBox.setText(msg);
    msgBox.setTextInteractionFlags(Qt::TextSelectableByMouse);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

