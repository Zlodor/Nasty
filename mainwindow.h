#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>

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

    QVector<double> process_data(QVector<double> data);

private:
    Ui::MainWindow *ui;

    QFile file;
    QList<QVector<double>> input_list;
    QList<QVector<double>> output_list;
};
#endif // MAINWINDOW_H
