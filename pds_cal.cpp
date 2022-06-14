#include "pds_cal.h"
#include "./ui_pds_cal.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <iostream>
#include <windows.h>

#include <QDebug>
#include <QBuffer>
#include <QDateTime>
#include <QTimer>
#include <QAuthenticator>
#include <QAuthenticator>
#include <QObject>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::pds_cal)
{
    ui->setupUi(this);
    /*mManager = new QNetworkAccessManager(this);
    connect(mManager, &QNetworkAccessManager::finished, this, [&](QNetworkReply *reply){
        QByteArray data = reply->readAll();
        QString str = QString::fromLatin1(data);
        ui->plainTextEdit->setPlainText(str);
    });*/
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_getButton_clicked()
{
    login("progetto-pds", "progetto-pds");
    // saveNewEvent();
    getAllEvents();
    //deleteEvent();
}

void MainWindow::login(std::string usr, std::string pwd) {
    QString _usr = QString::fromStdString(usr);
    QString _pwd = QString::fromStdString(pwd);
    QNetworkAccessManager *manager = new QNetworkAccessManager();

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(login_slot(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    QNetworkRequest request;

    request.setUrl(QUrl("https://cloud.mackers.dev/remote.php/dav/calendars/progetto-pds"));
    request.setRawHeader("Depth", "0");
    request.setRawHeader("Prefer", "return-minimal");
    request.setRawHeader("Content-Type", "application/xml; charset=utf-8");

    QByteArray req_propfind = "<d:propfind xmlns:d=\"DAV:\">\n"
                              "  <d:prop>\n"
                              "     <d:current-user-principal />\n"
                              "  </d:prop>\n"
                              "</d:propfind>";

    QNetworkReply *reply = manager->sendCustomRequest(request,"PROPFIND", req_propfind);
    QString response = reply->readAll();
    qDebug() << "[Login] " << reply;

}

void MainWindow::login_slot(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "[Logged]";
        QString strReply = (QString)reply->readAll();
    }
    else {
        qDebug() << "[Failure]" << reply -> errorString();
        delete reply;
    }
}

void MainWindow::do_authentication(QNetworkReply *, QAuthenticator *q) {
    q->setUser(QString::fromStdString("progetto-pds"));
    q->setPassword(QString::fromStdString("progetto-pds"));
}

void MainWindow::getAllEvents() {

    // SETTING UP REQUEST
    QString concatenated = "progetto-pds:progetto-pds";
    QByteArray user_pass = concatenated.toLocal8Bit().toBase64();
    QString header = "Basic " + user_pass;
    QString baseUrl = "https://cloud.mackers.dev/remote.php/dav/calendars/progetto-pds/test?export";

    QNetworkRequest request = QNetworkRequest(baseUrl);

    request.setRawHeader("Authorization", header.toLocal8Bit());

    // CONNECTION
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(report_function(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    manager->get(request);
    // The following code is useless, because the reply can only be handled in the slot (report_function)
    // QString response = reply->readAll();
    // qDebug() << "[Read all events] " << response;
}

void MainWindow::deleteEvent() {

    // CONNECTION
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(report_function(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    // SETTING UP REQUEST
    QString concatenated = "progetto-pds:progetto-pds";
    QByteArray user_pass = concatenated.toLocal8Bit().toBase64();
    QString header = "Basic " + user_pass;

    // QString filename = "7f449f88-1b1b-411b-ac73-0fa2befcba50.ics"; // not found
    // QString filename = "7f449f88-1b1b-411b-ac73-0fa2befcba50"; // not found
    // QString filename = "test123"; // not found
    QString filename = "20220522-2305-0026-0000-202205211405.ics"; // event that we added with Qt - seems to work

    QString baseUrl = "https://cloud.mackers.dev/remote.php/dav/calendars/progetto-pds/test/";
    //QString baseUrl = "https://cloud.mackers.dev/remote.php/dav/calendars/";

    // le righe commentate (prese da altre richieste) sono alternative a quelle non commentate (dal progetto vecchio)
    QNetworkRequest request;
    request.setUrl(QUrl(baseUrl + filename));
    request.setRawHeader("User-Agent", "CalendarClient_CalDAV");
    request.setRawHeader("Authorization", header.toUtf8());
    request.setRawHeader("Depth", "0");
    //request.setRawHeader("Depth", "1");
    request.setRawHeader("Prefer", "return-minimal");
    request.setRawHeader("Content-Type", "text/calendar; charset=utf-8");
    //request.setRawHeader("Content-Type", "application/xml; charset=utf-8");
    request.setRawHeader("Content-Length", 0);
    // request.setRawHeader("Content-Length", "XXXX");

    // request.setRawHeader("If-None-Match", "*");

    qDebug() << "Deleting" << request.url();


    /*QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);*/

    QNetworkReply *reply = manager->deleteResource(request);
    QString response = reply->readAll();
    qDebug() << "[Deleting Event] " << reply;

    /*if (NULL != m_pUploadReply)
    {
      connect(m_pUploadReply, SIGNAL(error(QNetworkReply::NetworkError)),
              this, SLOT(handleUploadHTTPError()));

      connect(m_pUploadReply, SIGNAL(finished()),
              this, SLOT(handleUploadFinished()));

      m_UploadRequestTimeoutTimer.start(m_RequestTimeoutMS);
    }
    else
    {
      QDEBUG << m_DisplayName << ": " << "ERROR: Invalid reply pointer when requesting URL.";
      emit error("Invalid reply pointer when requesting URL.");
    }*/
}

void MainWindow::saveNewEvent() {

    // CONNECTION
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(report_function(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    // TIMESTAMPS
    QDate start_date(2022, 05, 25);
    QTime start_time(18, 30);
    QTime end_time(23,30);
    QDateTime endDateTime(start_date, end_time);
    QDateTime startDateTime(start_date, start_time);

    // UID
    QString uid = QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") + "-0000-" + startDateTime.toString("yyyyMMddHHMM");
    QString filename = uid + ".ics";
    QNetworkRequest request;

    // SETTING HEADER
    QString myUrl = "https://cloud.mackers.dev/remote.php/dav/calendars/progetto-pds/test/" + uid + ".ics";
    request.setUrl(QUrl(myUrl));
    request.setRawHeader("Depth", "1");
    request.setRawHeader("Prefer", "return-minimal");
    request.setRawHeader("If-None-Match", "*");
    request.setRawHeader("Content-Type", "application/xml; charset=utf-8");

    QString request_report = "BEGIN:VCALENDAR\n"
            "BEGIN:VEVENT\n"
            "UID:" + uid + "\n"
            "SUMMARY:XXXXX\n"
            "DTSTART:" + startDateTime.toString("yyyyMMddTHHmmss") + "\r\n"
            "DTEND:" + endDateTime.toString("yyyyMMddTHHmmss") + "\r\n"
            "END:VEVENT\n"
            "END:VCALENDAR\n";

    QByteArray converted_report = request_report.toUtf8();

    QNetworkReply *reply = manager->put(request, converted_report);
    QString response = reply->readAll();
    qDebug() << "[Add Event] " << reply;
}

void MainWindow::report_function(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QString strReply = (QString)reply->readAll();
        qDebug() << "[OK]";
        qDebug() << strReply;
    }
    else {
        qDebug() << "[Failure]" << reply->errorString();
        delete reply;
    }
}


