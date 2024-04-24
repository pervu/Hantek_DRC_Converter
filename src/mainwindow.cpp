#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->hide();

    converter = new DRCconverter();
    plot = new OSCplot();

    // ProgressBar updates when converting and displaying drc files
    connect(converter, &DRCconverter::progressUpdated, this, &MainWindow::updateProgressBar);
    connect(plot, &OSCplot::progressUpdated, this, &MainWindow::updateProgressBar);
    // Update graphs in the window when adding data
    connect(plot, &OSCplot::replotUpdated, this, &MainWindow::updateReplot);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateProgressBar(quint16 progress)
{
    ui->statusbar->showMessage("Converting...", 500);
    ui->progressBar->setValue(progress);
    if (progress == 0) ui->progressBar->show();
    else if (progress == 100) ui->progressBar->hide();
}

void MainWindow::updateReplot()
{
    plot->replotGraphs();
}

void MainWindow::on_pushButtonOpenDrc_clicked()
{
    QFileInfo fileInfo(GRAPHS_DATA_FILE);
    if (fileInfo.exists()) {
        // Try to delete temp file
        QFile::remove(GRAPHS_DATA_FILE);
    }

    _drcFilePath = QFileDialog::getOpenFileName(
        this,
        tr("Open DRC file"),
        "",
        tr("DRC files (*.drc);; All files (*.*);;")
        );


    // Check if the file has been selected
    if (!_drcFilePath.isEmpty())
    {
        // Clear the window with previous charts
        plot->clearLayout(ui->verticalLayoutPlots);
        plot->clearCheckBoxes(ui->verticalLayoutCheckBoxes);
        ui->label_ChQnty->setText("");

        // Display file name
        QFileInfo fileInfo(_drcFilePath);
        QString fileName = fileInfo.fileName();
        ui->label_FileName->setText(fileName);

        // Calculate parameters of drc file, and save to JSON
        converter->calcHeader(_drcFilePath, "data.json");

        // Display the number of channels
        JsonDRC *json = new JsonDRC();
        json->readJsonFromFile("data.json");
        int chQnty = json->getChQnty();
        ui->label_ChQnty->setText(QString::number(chQnty));
        quint64 timeDiv = json->getTimeDiv();
        ui->label_TimeDiv->setText(QString::number(timeDiv));
    }
    else
    {
        ui->statusbar->showMessage("File not selected.", 2000);
    }
}


void MainWindow::on_pushButtonConvert_clicked()
{
    if (_drcFilePath.isEmpty())
    {
        ui->statusbar->showMessage("Drc file path is empty.", 2000);
        return;
    }
    converter->convertDrcInThread(_drcFilePath, GRAPHS_DATA_FILE, "data.json");
}


void MainWindow::on_pushButtonGraphs_clicked()
{
    // Clear the window with previous graphs
    plot->clearLayout(ui->verticalLayoutPlots);
    plot->clearCheckBoxes(ui->verticalLayoutCheckBoxes);
    // Check if there is a processed file with chart data
    QFileInfo fileInfo(GRAPHS_DATA_FILE);
    if (!fileInfo.exists()) {
        ui->statusbar->showMessage("No data to show", 2000);
        return;
    }
    // All graphs in the same plot, or each on a different plots
    plot->setOnePlot(ui->checkBoxOnePlot->isChecked());
    // Display chekBoxes by number of channels
    plot->createCheckBoxes(ui->verticalLayoutCheckBoxes, ui->label_ChQnty->text().toInt());
    // Draw graphs
    plot->createDynamicPlots(ui->verticalLayoutPlots);
    plot->readDataInThread(GRAPHS_DATA_FILE);
}


void MainWindow::on_checkBoxLockPos_stateChanged(int arg1)
{
    plot->lockPos(arg1);
}

void MainWindow::on_pushButtonSave_clicked()
{
    if (_drcFilePath.isEmpty())
    {
        ui->statusbar->showMessage("Drc file path is empty.", 2000);
        return;
    }
    QString fname = QFileDialog::getSaveFileName(
        nullptr,
        "Save to CSV",
        ".",
        "csv (*.csv);;"
        );
    converter->convertDrcInThread(_drcFilePath, fname, "data.json");
}


void MainWindow::on_pushButtonAbout_clicked()
{
    About *aboutWindow = new About();
    aboutWindow->setWindowTitle("Hantek 6000 Oscilloscope About");
    aboutWindow->show();
}


void MainWindow::on_checkBoxOnePlot_stateChanged(int arg1)
{
    on_pushButtonGraphs_clicked();
}

