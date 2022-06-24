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

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <regex>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::pds_cal)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui -> success_create_event -> hide();
    ui -> error_create_event -> hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loginButton_clicked() {

    QString login_user = ui->username_login->text();
    QString password = ui->password_login->text();

    if (login(login_user.toStdString(), password.toStdString())) {
        ui->stackedWidget->setCurrentIndex(1);
    } else {
        // TODO inserire messaggio di errore se USER / PWD sbagliata
    }
}

void MainWindow::on_createEventButton_clicked() {

    QString user = ui->username_login->text();
    QString summary = ui-> event_title -> toPlainText();
    QDate start_date = ui -> start_date -> date();
    QTime start_time = ui -> start_time -> time();
    QTime end_time = ui -> end_time -> time();

    // TODO: Inserire selezione calendario

    if (summary.isEmpty() || start_time > end_time) {
        ui -> success_create_event -> hide();
        ui -> error_create_event -> show();
    } else {
        createEvent(user, "test", summary, start_date, start_time, end_time);
        ui -> success_create_event -> show();
        ui -> error_create_event -> hide();
    }




}

void MainWindow::on_getButton_clicked()
{
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

    login(user.toStdString(), password.toStdString());
    // createEvent(user, calendar_name, summary, start_date, start_time, end_time);
    getAllEvents(user, password, calendar_name);
    // deleteEvent(user, password, calendar_name, uid);

    // DA FIXARE
    // updateEvent(user, calendar_name, uid, summary, start_date, start_time, end_time);

    // DA FINIRE

    QString data = "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nCALSCALE:GREGORIAN\r\nPRODID:-//SabreDAV//SabreDAV//EN\r\nX-WR-CALNAME:test\r\nX-APPLE-CALENDAR-COLOR:#499AA2\r\nREFRESH-INTERVAL;VALUE=DURATION:PT4H\r\nX-PUBLISHED-TTL:PT4H\r\nBEGIN:VEVENT\r\nCREATED:20220516T164427Z\r\nDTSTAMP:20220516T164443Z\r\nLAST-MODIFIED:20220516T164443Z\r\nSEQUENCE:4\r\nUID:7f449f88-1b1b-411b-ac73-0fa2befcba50\r\nDTSTART;VALUE=DATE:20220607\r\nDTEND;VALUE=DATE:20220612\r\nSUMMARY:Festa di prova\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nCREATED:20220516T164445Z\r\nDTSTAMP:20220516T164515Z\r\nLAST-MODIFIED:20220516T164515Z\r\nSEQUENCE:3\r\nUID:98a23dfe-54bb-4b61-81d1-0072f0b50a4c\r\nDTSTART;VALUE=DATE:20220611\r\nDTEND;VALUE=DATE:20220612\r\nCOLOR:burlywood\r\nSUMMARY:Giorno speciale\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:test123\r\nSUMMARY:Test Event2\r\nDTSTART:20220526T080000Z\r\nDTEND:20220526T170000Z\r\nDTSTAMP:20220522T203519Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:test1234\r\nSUMMARY:Test Event2\r\nDTSTART:20220526T080000Z\r\nDTEND:20220526T170000Z\r\nDTSTAMP:20220522T203633Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:PLEASEFUNZIONA\r\nSUMMARY:Test Event2\r\nDTSTART:20220527T080000Z\r\nDTEND:20220527T170000Z\r\nDTSTAMP:20220522T213945Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:asdfghjkl\r\nSUMMARY:EVENTO PROVA\r\nDTSTART:20220527T080000Z\r\nDTEND:20220527T170000Z\r\nDTSTAMP:20220522T214111Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220522-2305-0042-0000-202205211405\r\nSUMMARY:EVENTO PROVA\r\nDTSTART:20220521T143000\r\nDTEND:20220521T183000\r\nDTSTAMP:20220522T214443Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220522-2305-0000-0000-202205211405\r\nSUMMARY:XXXXX\r\nDTSTART:20220521T143000\r\nDTEND:20220521T183000\r\nDTSTAMP:20220522T215002Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220524-1505-0055-0000-202205251805\r\nSUMMARY:XXXXX\r\nDTSTART:20220525T183000\r\nDTEND:20220525T233000\r\nDTSTAMP:20220524T125557Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220522-2305-0026-0000-202205211405\r\nSUMMARY:EDITTTSSSSS\r\nDTSTART:20220525T183000\r\nDTEND:20220525T233000\r\nDTSTAMP:20220617T212451Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220522-2305-0026-0000-202341405\r\nSUMMARY:EDITED2\r\nDTSTART:20220525T183000\r\nDTEND:20220525T233000\r\nDTSTAMP:20220619T125750Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220624-1706-0020-0000-202206250806\r\nSUMMARY:Titolo\r\nDTSTART:20220625T080000\r\nDTEND:20220625T130000\r\nDTSTAMP:20220624T152322Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220624-1706-0010-0000-199912312312\r\nSUMMARY:\r\nDTSTART:19991231T230000\r\nDTEND:19991231T230000\r\nDTSTAMP:20220624T153412Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220624-1706-0049-0000-202206301406\r\nSUMMARY:Portineria\r\nDTSTART:20220630T140000\r\nDTEND:20220630T180000\r\nDTSTAMP:20220624T153551Z\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n";
    cal_translator(data);
}

void cal_translator(QString data) {



}

bool MainWindow::login(std::string usr, std::string pwd) {
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

    return true;

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


