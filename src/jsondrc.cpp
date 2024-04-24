#include "jsondrc.h"

JsonDRC::JsonDRC()
{

}

JsonDRC::~JsonDRC(){}

bool JsonDRC::createJsonFile(const QString &jsonFileName, const QString &drcFilePath,
                             int headerSize,
                             quint64 timeDIV,
                             int channelQuantity,
                             const QList<int> &chVoltages )
{
    QFile jsonFile(jsonFileName);
    if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open output file:" << jsonFile.errorString();
        return false;
    }
    QTextStream outStream(&jsonFile);

    QFileInfo fileInfo(drcFilePath);
    QString drcFileName = fileInfo.fileName();

    QJsonObject jsonobj;
    jsonobj["FileName"] = drcFileName;
    jsonobj["HeaderSize"] = headerSize;
    jsonobj["ChannelQnty"] = channelQuantity;
    jsonobj["timeDIV"] = QString::number(timeDIV);

    QJsonArray channels;

    for (int i = 0; i < channelQuantity; ++i) {
        QJsonObject channel;
        channel["Name"] = QString("CH%1").arg(i + 1);
        if (i < chVoltages.size()) {
            channel["Voltage"] = chVoltages[i];
        } else {
            // If the voltage is not sufficient, assign 'undefined'
            channel["Voltage"] = "undefined";
        }
        channels.append(channel);
    }

    jsonobj["Channels"] = channels;

    QJsonDocument doc(jsonobj);
    outStream << doc.toJson(QJsonDocument::Indented);

    jsonFile.close();

    return true;
}

bool JsonDRC::readJsonFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Can't open file:" << filePath;
        return false;
    }

    QTextStream in(&file);
    QString jsonData = in.readAll();
    file.close();

    doc = QJsonDocument::fromJson(jsonData.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "JSON read error:" << filePath;
        return false;
    }
    return true;
}

int JsonDRC::getChQnty()
{
    if (!doc.isEmpty()) {
        // Getting chQnty from JSON object
        int chQnty = static_cast<quint16>(doc["ChannelQnty"].toInt());
        return chQnty;
    } else {
        qWarning() << "JSON read err.";
    }
    return 0;
}

QList<int> JsonDRC::getChVoltages()
{
    if (!doc.isEmpty())
    {
        QList<int> chVoltages;
        QJsonArray channels = doc["Channels"].toArray();
        // Iterate over the channel array
        for (const QJsonValue &value : channels) {
            QJsonObject channel = value.toObject();
            //QString name = channel["Name"].toString();
            chVoltages.append(channel["Voltage"].toInt());
        }
        return chVoltages;
    }
    else
    {
        qWarning() << "Error read JSON. QJsonDocument isEmpty()";
    }
    return QList<int>();
}

int JsonDRC::getHeaderSize()
{
    if (!doc.isEmpty()) {
        // Getting headerSize from JSON object
        int headerSize = doc["HeaderSize"].toInt();
        return headerSize;
    } else {
        qWarning() << "JSON read err.";
    }
    return 0;
}

uint64_t JsonDRC::getTimeDiv()
{
    if (!doc.isEmpty()) {
        // Getting timeDiv from JSON object
        QString timeDiv = doc["timeDIV"].toString();
        return timeDiv.toULongLong();
    } else {
        qWarning() << "JSON read err.";
    }
    return 0;
}
