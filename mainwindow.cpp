#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "udp.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->Plot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    ui->Plot->axisRect()->setupFullAxesBox(true);
    ui->Plot->xAxis->setLabel("x");
    ui->Plot->yAxis->setLabel("y");


    ui->Plot_2->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    ui->Plot_2->axisRect()->setupFullAxesBox(true);
    ui->Plot_2->xAxis->setLabel("x");
    ui->Plot_2->yAxis->setLabel("y");

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openFile_btn_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Choose file");
    QFile file(filename);
    timer.start();
    count_data = file.size() / (sizeof(double) * 2);
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    for (i = 0; i < count_data; ++i)
    {
        double real, imag;
        stream >> real >> imag;
        std::complex<double> z(real, imag);
        buffer.append(z);
        //qDebug() << "buffer = " << buffer[i].real() << " + " << buffer[i].imag() << "i";
    }
    graphUpdate_raw(graphID);
    UDP client;
    client.SendDataUDP(buffer);
    buffer_receive_complex = client.readyRead();
    //qDebug() << "Прошло: " << (double)timer.elapsed() / 1000;
    FILE* fp_2;
    fopen_s(&fp_2, "D:\\Programming\\C++\\Qt\\CSA\\data.txt", "w");
    for (int i = 0; i < 2048; ++i)
    {
        for (int j = 0; j < 320; ++j)
        {
            int ii = 2048 * j + i;
            fprintf(fp_2, "i = %d  j = %d  ", i, j);
            fprintf(fp_2, "% .20f + %.20fi\n", buffer_receive_complex[ii].real(), buffer_receive_complex[ii].imag());

            //fprintf(fp_2, "i = %d j = %d %.16f \n", i, j, R0_RCMC[j]);
        }
    }
    fclose(fp_2);
    graphUpdate_image(buffer_receive_complex);
}

void MainWindow::graphUpdate_image(QVector<std::complex<double>> buffer_receive_complex)
{
    QCPColorMap *colorMap = new QCPColorMap(ui->Plot_2->xAxis, ui->Plot_2->yAxis);
    int nx = 2048;
    int ny = 320;
    colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
    colorMap->data()->setRange(QCPRange(0, 2048), QCPRange(0, 320)); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions
    // now we assign some data, by accessing the QCPColorMapData instance of the color map:
    double x, y;
    for (int xIndex=0; xIndex<nx; ++xIndex)
    {
      for (int yIndex=0; yIndex<ny; ++yIndex)
      {
        colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);

        z = abs(buffer_receive_complex[yIndex * 2048 + xIndex]);
        graph_label = "Image";
        colorMap->data()->setCell(xIndex, yIndex, z);
      }
    }
    QCPColorScale *colorScale = new QCPColorScale(ui->Plot_2);
    ui->Plot_2->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale
    // set the color gradient of the color map to one of the presets:
    colorMap->setGradient(QCPColorGradient::gpCold);
    // we could have also created a QCPColorGradient instance and added own colors to
    // the gradient, see the documentation of QCPColorGradient for what's possible.

    // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
    colorMap->rescaleDataRange();

    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->Plot_2);
    ui->Plot_2->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    QCPItemText *textLabel = new QCPItemText(ui->Plot_2);
    textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
    textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel->position->setCoords(0.5, 0); // place position at center/top of axis rect
    textLabel->setText(graph_label);
    textLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
    // rescale the key (x) and value (y) axes so the whole color map is visible:
    ui->Plot_2->rescaleAxes();
    ui->Plot_2->replot();
}

void MainWindow::graphUpdate_raw(int graphID)
{
    QCPColorMap *colorMap = new QCPColorMap(ui->Plot->xAxis, ui->Plot->yAxis);
    int nx = 2048;
    int ny = 320;
    colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
    colorMap->data()->setRange(QCPRange(0, 2048), QCPRange(0, 320)); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions
    // now we assign some data, by accessing the QCPColorMapData instance of the color map:
    double x, y;
    for (int xIndex=0; xIndex<nx; ++xIndex)
    {
      for (int yIndex=0; yIndex<ny; ++yIndex)
      {
        colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
        if (graphID == 0)
        {
        z = buffer[yIndex * 2048 + xIndex].real();
        graph_label = "Real part";
        }
        else if (graphID == 1)
        {
        z = buffer[yIndex * 2048 + xIndex].imag();
        graph_label = "Im part";
        }
        else if (graphID == 2)
        {
        z = abs(buffer[yIndex * 2048 + xIndex]);
        graph_label = "Amplitude";
        }
        else
        {
        z = arg(buffer[yIndex * 2048 + xIndex]);
        graph_label = "Phase";
        }
        colorMap->data()->setCell(xIndex, yIndex, z);
      }
    }
    QCPColorScale *colorScale = new QCPColorScale(ui->Plot);
    ui->Plot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale
    // set the color gradient of the color map to one of the presets:
    colorMap->setGradient(QCPColorGradient::gpHues);
    // we could have also created a QCPColorGradient instance and added own colors to
    // the gradient, see the documentation of QCPColorGradient for what's possible.

    // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
    colorMap->rescaleDataRange();

    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->Plot);
    ui->Plot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    QCPItemText *textLabel = new QCPItemText(ui->Plot);
    textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
    textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    textLabel->position->setCoords(0.5, 0); // place position at center/top of axis rect
    textLabel->setText(graph_label);
    textLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
    // rescale the key (x) and value (y) axes so the whole color map is visible:
    ui->Plot->rescaleAxes();
    ui->Plot->replot();
}

void MainWindow::on_nextGraph_btn_clicked()
{
    graphID++;
    if (graphID > 3)
    {
        graphID = 0;
    }
    graphUpdate_raw(graphID);
}

void MainWindow::on_prevGraph_btn_clicked()
{
    graphID--;
    if (graphID < 0)
    {
        graphID = 3;
    }
    graphUpdate_raw(graphID);
}

