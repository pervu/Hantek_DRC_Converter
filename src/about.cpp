#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::About)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

    ui->label_Link->setText("<a href=\"https://pervu.github.io/\">pervu.github.io</a>");
    ui->label_Link->setTextFormat(Qt::RichText);
    ui->label_Link->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->label_Link->setOpenExternalLinks(true);
}

About::~About()
{
    delete ui;
}

void About::on_pushButton_clicked()
{
    this->close();
}

