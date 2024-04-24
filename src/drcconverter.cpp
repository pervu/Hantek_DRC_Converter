#include "drcconverter.h"

DRCconverter::DRCconverter()
{
    json = new JsonDRC();
}

DRCconverter::~DRCconverter(){}

void DRCconverter::convertDrc(const QString &inputFilePath, const QString &outputFilePath, const QString &settingsFilePath)
{
    _blocksQnty = 0;

    if (json == nullptr)
    {
        qWarning() << "nullptr json.";
        return;
    }
    if (!json->readJsonFromFile(settingsFilePath))
    {
        qWarning() << "Can't read json.";
        return;
    }


    //int headerSize = calcHeader(inputFilePath);
    int headerSize = json->getHeaderSize();
    int chQnty = json->getChQnty();
    QList<int> chVoltages = json->getChVoltages();
    QList<int> maxVoltages;
    for (int v: chVoltages)
    {
        int maxV = 5*8*v;
        maxVoltages.append(maxV);
    }
    if (!headerSize)
    {
        qWarning() << "Can't find header.";
        return;
    }

    QFile inputFile(inputFilePath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open input file:" << inputFile.errorString();
        return;
    }
    inputFile.seek(0);
    QDataStream inStream(&inputFile);
    inStream.setByteOrder(QDataStream::LittleEndian);

    QFile outputFile(outputFilePath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open output file:" << outputFile.errorString();
        return;
    }
    QTextStream outStream(&outputFile);

    // Total file size
    quint64 totalSize = inputFile.size();
    // Processed file size
    quint64 processedSize = 0;

    // Write the name of the DRC file
    QFileInfo fileInfo(inputFilePath);
    QString fileName = fileInfo.fileName();
    outStream << "DRC_File: " << fileName << "\n";
    outStream << "Channels_qnty: " << chQnty << " Header_size: " << headerSize << " Time_DIV: " << json->getTimeDiv() << "\n";
    outStream << "CH \t Index \t Value \n";

    // Skip the common header
    for (int i = 0; i < headerSize; ++i) {
        quint8 dummy;
        inStream >> dummy;
        // Increase the processed size by the size of the missing element
        processedSize += sizeof(dummy);
    }

    // Initialize null indexes for all variables
    std::vector<quint64> index;
    index.resize(chQnty, 0);

    // Read blocks
    int channelNumber = 0;
    int progressPrev = 1000;
    while (!inStream.atEnd()) {
        // Read header of the block
        quint8 header[8];
        for (int i = 0; i < 8; ++i) {
            inStream >> header[i];
            processedSize += sizeof(header[i]);
        }
        // Size of the block
        quint16 blockSize = 4096;
        // In blocks, the channels run consecutively one after another
        if (channelNumber == chQnty) channelNumber = 0;
            ++channelNumber;

        // Data block
        for (int i = 0; i < blockSize; ++i) {
            short dataWord;
            inStream >> dataWord;
            double mvolts = static_cast<double>(dataWord) / 65535 * maxVoltages[channelNumber-1] * 2;
            outStream <<  QString::number(channelNumber) << "\t" << QString::number(index[channelNumber-1]) << "\t" << QString::number(mvolts) << "\n";
            index[channelNumber-1]++;
            processedSize += sizeof(dataWord);
        }

        // Increase block index
        ++_blocksQnty;

        // Calculate progress as a percentage
        int progress = (static_cast<float>(processedSize) / static_cast<float>(totalSize)) * 100;
        if (progress != progressPrev)
        {
            progressPrev = progress;
            emit progressUpdated(progress);
        }
    }
}

void DRCconverter::convertDrcInThread(const QString &inputFilePath, const QString &outputFilePath, const QString &settingsFilePath)
{
    // Run readData in a separate thread using a lambda expression
    QFuture<void> future = QtConcurrent::run([this, inputFilePath, outputFilePath, settingsFilePath] {
        this->convertDrc(inputFilePath, outputFilePath, settingsFilePath);
    });
}

int DRCconverter::calcHeader(const QString &drcFilePath, const QString &jsonFilePath)
{
    QFile *file = new QFile(drcFilePath);
    if (!file->open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << file->errorString();
        return false;
    }
    file->seek(0);

    QDataStream inStream(file);
    inStream.setByteOrder(QDataStream::LittleEndian);

    // Expected byte sequence
    QByteArray expectedHeader = QByteArray::fromHex("000002001401a8c000ffffff");
    // Read the first bytes from the file
    QByteArray fileHeader(expectedHeader.size(), '\0');
    inStream.readRawData(fileHeader.data(), expectedHeader.size());
    // Check if the first part of the file matches the expected sequence
    if (fileHeader != expectedHeader)
    {
        qWarning() << "Unknown header in the file";
        return 0;
    }
    // Skip 6 bytes
    inStream.skipRawData(6);

    // Read the maximum possible recorded number of channels in the file
    quint16 maxChannels;
    inStream >> maxChannels;
    // There's some shit going on here.
    // It seemed that this word is responsible for the number of active channels,
    // but if you mark channels in the wrong order when saving a drc file,
    // the counter resets to 0
    maxChannels++;
    // So set a fixed number of channels, for Hantec 6000 series it is 4.
    maxChannels = 4;

    quint8 dataWord;
    quint16 passedCh = 0;
    int chQnty = 0;
    QList<int> chVoltages;

    // Watch the channels
    while(passedCh < maxChannels)
    {
        inStream >> dataWord;
        if (dataWord == 1)
        {
            chQnty++;
            quint8 chData[29];
            for (int i=0; i<29; i++)
            {
                inStream >> chData[i];
            }
            chVoltages.append(voltageFromIndex(chData[3]));
            // Graph zero position
            quint16 graphPos = ((quint16)chData[17] << 8) + (quint16)chData[18];
        }
        else if (dataWord == 0)
        {
            inStream.skipRawData(4);
        }
        else
        {
            qWarning() << "Unexpected data";
        }
        ++passedCh;
    }

    int headerSize = 0;
    if (chQnty)
        headerSize = 138 + chQnty * 26;

    int timeDivPos = headerSize - 80;
    inStream.device()->seek(timeDivPos);
    // Read timeDiv
    quint8 timeDivIndex;
    inStream >> timeDivIndex;
    quint64 timeDivNs = timeDivFromIndex(timeDivIndex);

    file->close();
    // Create json file with settings
    json->createJsonFile(jsonFilePath, drcFilePath, headerSize, timeDivNs, chQnty, chVoltages);

    return headerSize;
}

int DRCconverter::voltageFromIndex(int index)
{
    if (index >= 0 && index < (int)voltages.size()) {
        return voltages[index];
    }
    else
    {
        return 0;
    }
}

quint64 DRCconverter::timeDivFromIndex(int index)
{
    if (index >= 0 && index < (int)timeDivsNs.size()) {
        return timeDivsNs[index];
    }
    else
    {
        return 0;
    }
}
