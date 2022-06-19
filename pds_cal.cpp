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
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_getButton_clicked()
{
    login("progetto-pds", "progetto-pds");

    // VARIABILI DA PASSARE ALL'ESTERNO
    // per ora le metto qui in modo da pulire i singoli metodi

    QString user = "progetto-pds";
    QString password = "progetto-pds";
    QString calendar_name = "test";
    QString uid = "20220619-1506-0011-0000-202206101306"; // UID evento di prova
    QString summary = "ESAME PDS";
    QDate start_date(2022, 06, 10);
    QTime start_time(13, 30);
    QTime end_time(15,00);


    // createEvent(user, calendar_name, summary, start_date, start_time, end_time);
    // getAllEvents(user, password, calendar_name);
    deleteEvent(user, password, calendar_name, uid);

    // DA FIXARE
    // updateEvent(user, calendar_name, uid, summary, start_date, start_time, end_time);

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

void MainWindow::getAllEvents(QString user, QString pass, QString calendar_name) {

    // SETTING UP REQUEST
    QString concatenated = user + ":" + pass;
    QByteArray user_pass = concatenated.toLocal8Bit().toBase64();
    QString header = "Basic " + user_pass;
    QString baseUrl = "https://cloud.mackers.dev/remote.php/dav/calendars/" + user + "/" + calendar_name + "?export";

    QNetworkRequest request = QNetworkRequest(baseUrl);
    request.setRawHeader("Authorization", header.toLocal8Bit());

    // CONNECTION
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(report_function(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));
    manager->get(request);

}

void MainWindow::deleteEvent(QString user, QString pass, QString calendar_name, QString uid) {

    // CONNECTION
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(report_function(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    // SETTING UP REQUEST
    QString concatenated = user + ":" + pass;
    QByteArray user_pass = concatenated.toLocal8Bit().toBase64();
    QString header = "Basic " + user_pass;
    QString filename = uid + ".ics"; // event that we added with Qt - seems to work
    QString baseUrl = "https://cloud.mackers.dev/remote.php/dav/calendars/" + user + "/" + calendar_name + "/";

    QNetworkRequest request;
    request.setUrl(QUrl(baseUrl + filename));
    request.setRawHeader("User-Agent", "CalendarClient_CalDAV");
    request.setRawHeader("Authorization", header.toUtf8());
    request.setRawHeader("Depth", "0");
    request.setRawHeader("Prefer", "return-minimal");
    request.setRawHeader("Content-Type", "text/calendar; charset=utf-8");
    request.setRawHeader("Content-Length", 0);

    qDebug() << "Deleting" << request.url();
    QNetworkReply *reply = manager->deleteResource(request);
    QString response = reply->readAll();
    qDebug() << "[Deleting Event] " << reply;
}

void MainWindow::createEvent(QString user, QString calendar_name, QString summary, QDate start_date, QTime start_time, QTime end_time) {

    // CONNECTION
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(report_function(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));
    QDateTime endDateTime(start_date, end_time);
    QDateTime startDateTime(start_date, start_time);

    // UID
    QString uid = QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") + "-0000-" + startDateTime.toString("yyyyMMddHHMM");
    QNetworkRequest request;

    // SETTING HEADER
    QString myUrl = "https://cloud.mackers.dev/remote.php/dav/calendars/" + user + "/" + calendar_name + "/" + uid + ".ics";
    request.setUrl(QUrl(myUrl));
    request.setRawHeader("Depth", "1");
    request.setRawHeader("Prefer", "return-minimal");
    request.setRawHeader("If-None-Match", "*");
    request.setRawHeader("Content-Type", "application/xml; charset=utf-8");

    QString request_report = "BEGIN:VCALENDAR\n"
            "BEGIN:VEVENT\n"
            "UID:" + uid + "\n"
            "SUMMARY:" + summary + "\n"
            "DTSTART:" + startDateTime.toString("yyyyMMddTHHmmss") + "\r\n"
            "DTEND:" + endDateTime.toString("yyyyMMddTHHmmss") + "\r\n"
            "END:VEVENT\n"
            "END:VCALENDAR\n";

    QByteArray converted_report = request_report.toUtf8();

    QNetworkReply *reply = manager->put(request, converted_report);
    QString response = reply->readAll();
    qDebug() << "[Add Event] " << reply;
}

void MainWindow::updateEvent(QString user, QString calendar_name, QString uid, QString summary, QDate start_date, QTime start_time, QTime end_time) {

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(report_function(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    QDateTime endDateTime(start_date, end_time);
    QDateTime startDateTime(start_date, start_time);
    QNetworkRequest request;

    // SETTING HEADER
    QString myUrl = "https://cloud.mackers.dev/remote.php/dav/calendars/" + user + "/" + calendar_name + "/" + uid + ".ics";
    request.setUrl(QUrl(myUrl));
    request.setRawHeader("Depth", "1");
    request.setRawHeader("Prefer", "return-minimal");
    request.setRawHeader("If-None-Match", "*");
    request.setRawHeader("Content-Type", "application/xml; charset=utf-8");

    QString request_report = "BEGIN:VCALENDAR\n"
            "BEGIN:VEVENT\n"
            "UID:" + uid + "\n"
            "SUMMARY:" + summary + "\n"
            "DTSTART:" + startDateTime.toString("yyyyMMddTHHmmss") + "\r\n"
            "DTEND:" + endDateTime.toString("yyyyMMddTHHmmss") + "\r\n"
            "END:VEVENT\n"
            "END:VCALENDAR\n";

    QByteArray converted_report = request_report.toUtf8();

    QNetworkReply *reply = manager->put(request, converted_report);
    QString response = reply->readAll();
    qDebug() << "[Edit Event] " << reply;
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


