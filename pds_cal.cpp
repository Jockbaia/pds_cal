#include "pds_cal.h"
#include "./ui_pds_cal.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>


pds_cal::pds_cal(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::pds_cal)
{
    ui->setupUi(this);
    mManager = new QNetworkAccessManager(this);
    connect(mManager, &QNetworkAccessManager::finished, this, [&](QNetworkReply *reply){
        QByteArray data = reply->readAll();
        QString str = QString::fromLatin1(data);
        ui->plainTextEdit->setPlainText(str);
    });
}

pds_cal::~pds_cal()
{
    delete ui;
}


void pds_cal::on_getButton_clicked()
{
    // mManager->get(QNetworkRequest(QUrl(ui->urlLineEdit->text())));

    //authentication

    QString username = ui->username_login->text();
    QString pass = ui->password_login->text();

    QString concatenated = username + ":" + pass; //username:password
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;
    QNetworkRequest request=QNetworkRequest(QUrl(ui->urlLineEdit->text()));

    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    mManager->get(request);
}

