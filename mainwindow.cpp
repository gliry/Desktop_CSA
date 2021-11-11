#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->Plot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    ui->Plot->axisRect()->setupFullAxesBox(true);
    ui->Plot->xAxis->setLabel("x");
    ui->Plot->yAxis->setLabel("y");
    connect(this, &MainWindow::signalFromFile, this, &MainWindow::slotGraphUpdate);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openFile_btn_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Choose file");
    QFile file(filename);
    count_data = file.size() / (sizeof(double) * 2);
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    for (i = 0; i < count_data; ++i) // вычислить размер файла
    {
        double real, imag;
        stream >> real >> imag;
        std::complex<double> z(real, imag);
        buffer.append(z);
        //qDebug() << "buffer = " << buffer[i].real() << " + " << buffer[i].imag() << "i";
    }

    emit signalFromFile(graphID);
}

void MainWindow::slotGraphUpdate(int graphID)
{
    QCPColorMap *colorMap = new QCPColorMap(ui->Plot->xAxis, ui->Plot->yAxis);
    int nx = 1024;
    int ny = 320;
    colorMap->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
    colorMap->data()->setRange(QCPRange(0, 320), QCPRange(0, 1024)); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions
    // now we assign some data, by accessing the QCPColorMapData instance of the color map:
    double x, y;
    for (int xIndex=0; xIndex<nx; ++xIndex)
    {
      for (int yIndex=0; yIndex<ny; ++yIndex)
      {
        colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
        if (graphID == 0)
        {
        z = buffer[yIndex * 1024 + xIndex].real();
        graph_label = "Real part";
        }
        else if (graphID == 1)
        {
        z = buffer[yIndex * 1024 + xIndex].imag();
        graph_label = "Im part";
        }
        else if (graphID == 2)
        {
        z = abs(buffer[yIndex * 1024 + xIndex]);
        graph_label = "Amplitude";
        }
        else
        {
        z = arg(buffer[yIndex * 1024 + xIndex]);
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
    emit signalFromFile(graphID);
}

void MainWindow::on_prevGraph_btn_clicked()
{
    graphID--;
    if (graphID < 0)
    {
        graphID = 3;
    }
    emit signalFromFile(graphID);
}

