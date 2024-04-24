#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "oscplot.h"
#include "drcconverter.h"
#include <QFileInfo>
#include <QFileDialog>
#include "about.h"

#define GRAPHS_DATA_FILE "graphs.dat"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setupCustomPlot(QCustomPlot *customPlot);
    void readData(QString filePath);
    void createDynamicPlots(int plotCount);

public slots:
    void updateProgressBar(quint16 progress);
    void updateReplot();

private slots:
    void on_pushButtonOpenDrc_clicked();

    void on_pushButtonConvert_clicked();

    void on_pushButtonGraphs_clicked();

    void on_checkBoxLockPos_stateChanged(int arg1);

    void on_pushButtonSave_clicked();

    void on_pushButtonAbout_clicked();

    void on_checkBoxOnePlot_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;

    OSCplot *plot = nullptr;
    DRCconverter *converter = nullptr;

    QString _drcFilePath;
};
#endif // MAINWINDOW_H
