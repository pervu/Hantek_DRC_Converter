// OSCplot.h

#ifndef OSCPLOT_H
#define OSCPLOT_H

#include <QObject>
#include <QWidget>
#include <QFile>
#include <QtConcurrent/QtConcurrentRun>
#include "../qcustomplot/qcustomplot.h"
#include "jsondrc.h"


class OSCplot : public QCustomPlot
{
    Q_OBJECT
public:
    OSCplot(QWidget *parent = nullptr);
    ~OSCplot();
    void readData(QString filePath);
    void readDataInThread(QString filePath);
    void createDynamicPlots(QVBoxLayout *vlayout);
    void clearLayout(QLayout *layout);
    void replotGraphs();
    void lockPos(bool isLocked);
    void createCheckBoxes(QVBoxLayout *layout, int qnty);
    void clearCheckBoxes(QVBoxLayout *layout);
    void setOnePlot(const bool &onePlot);

protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void progressUpdated(quint16 progress);
    void replotUpdated();

public slots:
    void syncAxes(const QCPRange &newRange);
    void checkBoxChanged(int state, int index);

private:
    bool _onePlot = false;
    QVector<QVector<double>>* dataX;
    QVector<QVector<double>>* dataY;
    QList<QCustomPlot*> customPlot;
    QCustomPlot *oneCustomPlot = nullptr;
    // Parameters of graphs display
    void setupCustomPlot(QCustomPlot *customPlot);
    QList<QCPItemLine*> verticalLines;  // Pointer to vertical line
    QList<QCPItemText*> valueLabels;    // Text labels for each chart

    QVector<QColor> colors = {
        QColor(Qt::magenta),
        QColor(Qt::blue),
        QColor(Qt::red),
        QColor(Qt::green),
        QColor(Qt::cyan),
        QColor(Qt::yellow)
    };
};

#endif // OSCPLOT_H
