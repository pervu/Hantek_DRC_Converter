#ifndef JSONDRC_H
#define JSONDRC_H

#include <QObject>
#include <QWidget>
#include <QFileInfo>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>

class JsonDRC : public QWidget
{
    Q_OBJECT
public:
    JsonDRC();
    ~JsonDRC();
    bool createJsonFile(const QString &jsonFileName, const QString &drcFilePath,
                        int headerSize,
                        quint64 timeDIV,
                        int channelQuantity,
                        const QList<int> &chVoltages);

    bool readJsonFromFile(const QString &filePath);

    int getChQnty();
    QList<int> getChVoltages();
    int getHeaderSize();
    uint64_t getTimeDiv();

private:
    QJsonDocument doc;
};

#endif // JSONDRC_H
