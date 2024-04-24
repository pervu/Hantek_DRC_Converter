#ifndef DRCCONVERTER_H
#define DRCCONVERTER_H

#include <QObject>
#include <QWidget>
#include <QFile>
#include <QtConcurrent/QtConcurrentRun>
#include "jsondrc.h"


class DRCconverter : public QWidget
{
    Q_OBJECT
public:
    DRCconverter();
    ~DRCconverter();
    void convertDrc(const QString &inputFilePath, const QString &outputFilePath, const QString &settingsFilePath);
    void convertDrcInThread(const QString &inputFilePath, const QString &outputFilePath, const QString &settingsFilePath);
    int calcHeader(const QString &drcFilePath, const QString &jsonFilePath);

signals:
    void progressUpdated(quint16 progress);

private:
    int voltageFromIndex(int index);
    quint64 timeDivFromIndex(int index);
    quint64 _blocksQnty;
    // Initialization of the array with stress values
    std::vector<int> voltages = {
        2,    // 2 mV
        5,    // 5 mV
        10,   // 10 mV
        20,   // 20 mV
        50,   // 50 mV
        100,  // 100 mV
        200,  // 200 mV
        500,  // 500 mV
        1000, // 1000 mV
        2000, // 2000 mV
        5000, // 5000 mV
        10000 // 10000 mV
    };
    // Initialization of the array with time/Div values in nanoseconds
    std::vector<quint64> timeDivsNs = {
        2, 5, 10, 20, 50, 100, 200, 500,
        1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000,
        1000000, 2000000, 5000000, 10000000, 20000000, 50000000, 100000000, 200000000, 500000000,
        1000000000, 2000000000, 5000000000, 10000000000, 20000000000, 50000000000, 100000000000, 500000000000, 1000000000000
    };
    JsonDRC *json = nullptr;
};

#endif // DRCCONVERTER_H
