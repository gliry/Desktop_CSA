#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <complex>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>
#include <stdio.h>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QList<std::complex<double>> buffer;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void signalFromFile(int graphID);

private slots:
    void on_openFile_btn_clicked();
    void slotGraphUpdate(int graphID);

    void on_nextGraph_btn_clicked();

    void on_prevGraph_btn_clicked();

private:
    Ui::MainWindow *ui;
    int i, j;
    bool b;
    int count_data;
    double z;
    int graphID = 0;
    QString graph_label;
};
#endif // MAINWINDOW_H
