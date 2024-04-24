// OSCplot.cpp
#include "oscplot.h"

OSCplot::OSCplot(QWidget *parent): QCustomPlot(parent){}

OSCplot::~OSCplot(){}

void OSCplot::readData(QString filePath)
{
    JsonDRC *json = new JsonDRC();
    if (!json->readJsonFromFile("data.json"))
    {
        qWarning() << "data.json read err.";
        return;
    }
    int chQnty = json->getChQnty();
    if (!chQnty)
    {
        qWarning() << "Channels qnty is 0.";
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream inStream(&file);

    // Read common settings name
    QString headerStr = inStream.readLine();
    QString dataVars = inStream.readLine();
    QString csvHeader = inStream.readLine();

    quint64 totalSize = file.size();
    quint64 processedSize = 0;

    // Create a pointer to a two-dimensional vector
    dataX = new QVector<QVector<double>>(chQnty);
    dataY = new QVector<QVector<double>>(chQnty);

    // Initialization of each internal vector (by default they are empty)
    for (int i = 0; i < chQnty; ++i) {
        (*dataX)[i] = QVector<double>();
        (*dataY)[i] = QVector<double>();
    }

    int progressPrev = 1000;
    while (!inStream.atEnd()) {
        QString line = inStream.readLine();
        processedSize = file.pos();
        QStringList parts = line.split('\t');
        if (parts.size() == 3) {
            int channel = parts[0].toInt();
            double x = parts[1].toDouble();
            double y = parts[2].toDouble();
            (*dataX)[channel-1].append(x);
            (*dataY)[channel-1].append(y);
        }
        // Calculate progress as a percentage
        int progress = (static_cast<float>(processedSize) / static_cast<float>(totalSize)) * 100;
        if (progress != progressPrev)
        {
            for (int i = 0; i < chQnty; ++i)
            {
                if (!_onePlot)
                {
                    if (customPlot[i])
                        customPlot[i]->graph(0)->addData((*dataX)[i], (*dataY)[i]);
                }
                else
                {
                    if (oneCustomPlot)
                        oneCustomPlot->graph(i)->addData((*dataX)[i], (*dataY)[i]);
                }
                (*dataX)[i].clear();
                (*dataY)[i].clear();
            }
            progressPrev = progress;
            emit progressUpdated(progress);
            emit replotUpdated();
        }
    }
}

void OSCplot::readDataInThread(QString filePath)
{
    // Run readData in a separate thread using a lambda expression
    QFuture<void> future = QtConcurrent::run([this, filePath] {
        this->readData(filePath);
    });
}



void OSCplot::createDynamicPlots(QVBoxLayout *vlayout)
{
    JsonDRC *json = new JsonDRC();
    if (!json->readJsonFromFile("data.json"))
    {
        qWarning() << "data.json read err.";
        return;
    }

    QList<int> maxVoltage;
    QList<int> chVoltages = json->getChVoltages();
    if (chVoltages.isEmpty()) return;
    int chQnty = json->getChQnty();

    for (int v : chVoltages)
    {
        maxVoltage.append(v*8*5);
    }

    if (!_onePlot)
    {
        for (int i = 0; i < chQnty; ++i) {
            QCustomPlot *customPlotTmp = new QCustomPlot(this);
            customPlot.append(customPlotTmp);
            customPlot[i]->yAxis->setRange(-maxVoltage[i], maxVoltage[i]);
            customPlot[i]->yAxis->setLabel("Ch" + QString::number(i+1) + ", mV");
            vlayout->addWidget(customPlot[i]);

            // Pass mouse events back to the parent
            connect(customPlot[i], &QCustomPlot::mousePress, this, [this](QMouseEvent* event) {
                this->mousePressEvent(event);
            });

            // Graph Configuration
            setupCustomPlot(customPlot[i]);
            customPlot[i]->addGraph();
            // Set the color of the chart line
            QPen graphPen;
            graphPen.setColor(colors[i]);
            customPlot[i]->graph(0)->setPen(graphPen);
        }
    }
    else
    {
        oneCustomPlot = new QCustomPlot(this);
        oneCustomPlot->yAxis->setRange(-maxVoltage[0], maxVoltage[0]);
        oneCustomPlot->yAxis->setLabel("Ch, mV");
        vlayout->addWidget(oneCustomPlot);
        // Pass mouse events back to the parent
        connect(oneCustomPlot, &QCustomPlot::mousePress, this, [this](QMouseEvent* event) {
            this->mousePressEvent(event);
        });
        // Graph Configuration
        setupCustomPlot(oneCustomPlot);
        for (int i = 0; i < chQnty; ++i) {
            oneCustomPlot->addGraph();
            // Set the color of the chart line
            QPen graphPen;
            graphPen.setColor(colors[i]);
            oneCustomPlot->graph(i)->setPen(graphPen);
        }
    }
}

void OSCplot::clearLayout(QLayout *layout)
{
    customPlot.clear();
    verticalLines.clear();
    valueLabels.clear();

    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void OSCplot::replotGraphs()
{
    if (!_onePlot)
    {
        for (QCustomPlot *plot : customPlot)
        {
            if (plot)
                plot->replot();
        }
    }
    else
    {
        oneCustomPlot->replot();
    }

}

void OSCplot::lockPos(bool isLocked)
{
    if (customPlot.isEmpty())
    {
        qWarning() << "lockPos(bool isLocked) customPlot is Empty!";
        return;
    }
    if (isLocked)
    {
        for (QCustomPlot *plot : customPlot)
        {
            connect(plot->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), this, &OSCplot::syncAxes);
        }
    }
    else
    {
        for (QCustomPlot *plot : customPlot)
        {
            disconnect(plot->xAxis, static_cast<void (QCPAxis::*)(const QCPRange &)>(&QCPAxis::rangeChanged), this, &OSCplot::syncAxes);
        }
    }
}

void OSCplot::createCheckBoxes(QVBoxLayout *layout, int qnty)
{
    QGridLayout *gridLayout = new QGridLayout();
    layout->addLayout(gridLayout);
    // Create checkboxes and add them to the grid
    for (int i = 0; i < qnty; ++i) {
        QCheckBox *checkBox = new QCheckBox("Ch " + QString::number(i + 1));
        checkBox->setChecked(true);
        // Connect the slot signal via lambda function with capture i
        connect(checkBox, &QCheckBox::stateChanged, this, [this, i](int state) {
            checkBoxChanged(state, i);
        });
        // Calculate row and column for the current checkbox
        int row = i / 2;  // Divide the index by 2 for a new line every 2 elements
        int col = i % 2;  // The remainder of the division determines the column (0 or 1)
        gridLayout->addWidget(checkBox, row, col);
    }
}

void OSCplot::clearCheckBoxes(QVBoxLayout *layout)
{
    QGridLayout *gridLayout = nullptr;
    int layoutIndex = -1;

    // Search for QGridLayout among all QVBoxLayout elements
    for (int i = 0; i < layout->count(); ++i) {
        QLayoutItem* item = layout->itemAt(i);
        if (QGridLayout* g = qobject_cast<QGridLayout*>(item->layout())) {
            gridLayout = g;
            layoutIndex = i;
            break;
        }
    }

    // If not found, nothing to clear, exit.
    if (!gridLayout)
        return;

    // Remove all checkboxes from the grid and the widgets themselves
    while (QLayoutItem* child = gridLayout->takeAt(0)) {
        if (QWidget* widget = child->widget()) {
            delete widget;
        }
        delete child;  // Delete grid element
    }

    // Deleting the grid itself
    layout->takeAt(layoutIndex);  // Remove the grid from the QVBoxLayout, but do not delete it.
    delete gridLayout;  // Remove the grid
}

void OSCplot::setOnePlot(const bool &onePlot)
{
    _onePlot = onePlot;
}

void OSCplot::setupCustomPlot(QCustomPlot *customPlot)
{

    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom );
    // set axes ranges
    customPlot->xAxis->setRange(0, 8000);

    //******* Some ui visuals for the graphs **************

    // customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
    //                             QCP::iSelectLegend | QCP::iSelectPlottables);

    // customPlot->setBackground(QBrush(QColor(43, 43, 43)));
    // // Set graphs colors
    // QPen axisPen(Qt::white, 1); // Color and line tickness
    // customPlot->xAxis->setBasePen(axisPen);
    // customPlot->yAxis->setBasePen(axisPen);
    // customPlot->xAxis->setTickPen(axisPen);
    // customPlot->yAxis->setTickPen(axisPen);
    // customPlot->xAxis->setSubTickPen(axisPen);
    // customPlot->yAxis->setSubTickPen(axisPen);
    // customPlot->xAxis->setTickLabelColor(Qt::white);
    // customPlot->yAxis->setTickLabelColor(Qt::white);
    // customPlot->xAxis->setLabelColor(Qt::white);
    // customPlot->yAxis->setLabelColor(Qt::white);

    // // give the axes some labels:
    // customPlot->xAxis->setLabel("x");
    // customPlot->yAxis->setLabel("y");

    // //Autoscale
    //customPlot->graph(0)->rescaleValueAxis();

    // // Fonts X Y
    // QFont labelFont = QFont("Helvetica", 8);
    // customPlot->xAxis->setLabelFont(labelFont);
    // customPlot->xAxis->setTickLabelFont(labelFont);
    // customPlot->yAxis->setLabelFont(labelFont);
    // customPlot->yAxis->setTickLabelFont(labelFont);

    // // Tick counts X Y
    // customPlot->xAxis->ticker()->setTickCount(12);
    // customPlot->xAxis->ticker()->setTickStepStrategy(QCPAxisTicker::tssReadability);
    // customPlot->yAxis->ticker()->setTickCount(10);
    // customPlot->yAxis->ticker()->setTickStepStrategy(QCPAxisTicker::tssReadability);
}

void OSCplot::syncAxes(const QCPRange &newRange) {
    // Traverse all graphs and set a new range for the X axis
    for (QCustomPlot *plot : customPlot) {
        plot->xAxis->setRange(newRange);
        plot->replot(); // Redraw the chart to update the display
    }
}

void OSCplot::checkBoxChanged(int state, int index)
{
    if (customPlot.size() > index)
    {
        if (customPlot[index] != nullptr)
        {
            if (state)
                customPlot[index]->show();
            else
                customPlot[index]->hide();
        }
    }
    if (oneCustomPlot != nullptr)
    {
        if (oneCustomPlot->graphCount() > index)
        {
            oneCustomPlot->graph(index)->setVisible(state);
            oneCustomPlot->replot();
        }
    }
}

void OSCplot::mousePressEvent(QMouseEvent *event)
{
    if (!_onePlot)
    {
    int plotIndex = 0;
    for (QCustomPlot *plot : customPlot) {
        if (plot->rect().contains(event->pos())) {
            double x = plot->xAxis->pixelToCoord(event->pos().x());

            if (verticalLines.size() < customPlot.size()) {
                verticalLines.resize(customPlot.size(), nullptr);
                valueLabels.resize(customPlot.size(), nullptr);
            }

            if (!verticalLines[plotIndex]) {
                verticalLines[plotIndex] = new QCPItemLine(plot);
            }

            verticalLines[plotIndex]->start->setCoords(x, plot->yAxis->range().lower);
            verticalLines[plotIndex]->end->setCoords(x, plot->yAxis->range().upper);

            if (!valueLabels[plotIndex]) {
                valueLabels[plotIndex] = new QCPItemText(plot);
            }

            // Search for chart data at the nearest point to x
            QCPGraphDataContainer::const_iterator it = plot->graph(0)->data()->findBegin(x, true);
            // Use the value if the iterator is not at the end of the container
            double yValue = it != plot->graph(0)->data()->end() ? it->value : 0;

            valueLabels[plotIndex]->position->setCoords(x, plot->yAxis->range().upper);
            valueLabels[plotIndex]->setText(QString::number(yValue));
            valueLabels[plotIndex]->setPositionAlignment(Qt::AlignTop|Qt::AlignLeft);
            valueLabels[plotIndex]->setPadding(QMargins(5, 5, 5, 5));

            plot->replot();
        }
        plotIndex++;
    }
    }
    // Call the base implementation for internal event handling
    QCustomPlot::mousePressEvent(event);
}

