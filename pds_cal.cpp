#include "pds_cal.h"
#include "./ui_pds_cal.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <iostream>

#include <QDebug>
#include <QBuffer>
#include <QDateTime>
#include <QTimer>
#include <QAuthenticator>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QObject>

#include <thread>
#include <mutex>
#include <ctime>


MainWindow::MainWindow(QWidget *parent)
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

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_getButton_clicked()
{
    QNetworkRequest request;
    QDate start_date(2022, 05, 21);
    QTime start_time(14, 30);
    QTime end_time(18,30);
    QDateTime endDateTime(start_date, end_time);
    QDateTime startDateTime(start_date, start_time);    // Local Time, non ho trovato un modo facile per scegliere il fuso orario
    QString uid = QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") + "-0000-" + startDateTime.toString("yyyyMMddHHMM");
    QString filename = uid + ".ics";
    request.setUrl(QUrl("https://cloud.mackers.dev/remote.php/dav/calendars/progetto-pds/test/" + filename));
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false); // Fallback to HTTP 1.1
        //"The "If-None-Match: *" request header ensures that the client will not inadvertently overwrite an existing resource
        //if the last path segment turned out to already be used"
    request.setRawHeader("If-None-Match", "*");
    request.setRawHeader("Content-Type", "text/calendar; charset=utf-8");

    QString requestString = "BEGIN:VCALENDAR\r\n"
                               "VERSION:2.0\r\n"
                               "BEGIN:VEVENT\r\n"
                               "UID:" + uid + "\r\n"
                               "DTSTAMP:" + QDateTime::currentDateTime().toString("yyyyMMddTHHmmssZ") + "\r\n"
                               "DTSTART:" + startDateTime.toString("yyyyMMddTHHmmss") + "\r\n"
                               "DTEND:" + endDateTime.toString("yyyyMMddTHHmmss") + "\r\n"
                               "SUMMARY:" + "TEST1-SUMMARY" + "\r\n"
                               "LOCATION:" + "TEST1-LOCATION" + "\r\n"
                               "DESCRIPTION:" + "TEST1-DESCRIPTION" + "\r\n";

    QBuffer* buffer = new QBuffer();
    buffer->open(QIODevice::ReadWrite);
    int bufferSize = buffer->write(requestString.toUtf8());
    buffer->seek(0);
    QByteArray contentLength;
    contentLength.append(QString::number(bufferSize).toStdString());
    request.setRawHeader("Content-Length", contentLength);



    QNetworkAccessManager *_manager;
    QNetworkReply* _reply = _manager->put(request, buffer);

    // When request ends check the status (200 OK or not) and then handle the Reply
    connect(_reply, SIGNAL(finished()), this, SLOT(handleAddingVEventFinished()));  //so we use this
    // If authentication is required, provide credentials
    connect(_manager, &QNetworkAccessManager::authenticationRequired, this, &MainWindow::handleAuthentication);

}

void MainWindow::handleAuthentication(QNetworkReply *reply, QAuthenticator *q) const
{
      q->setUser("progetto-pds");
      q->setPassword("progetto-pds");
}


