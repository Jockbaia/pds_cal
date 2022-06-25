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
#include <QString>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <regex>



class Event {
  public:
    std::string UID;
    std::string summary;
    QDateTime timestamp_start;
    QDateTime timestamp_end;
    QDateTime creation_date;
};


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

    QString data = "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nCALSCALE:GREGORIAN\r\nPRODID:-//SabreDAV//SabreDAV//EN\r\nX-WR-CALNAME:test\r\nX-APPLE-CALENDAR-COLOR:#499AA2\r\nREFRESH-INTERVAL;VALUE=DURATION:PT4H\r\nX-PUBLISHED-TTL:PT4H\r\nBEGIN:VEVENT\r\nUID:test123\r\nSUMMARY:Test Event2\r\nDTSTART:20220526T080000Z\r\nDTEND:20220526T170000Z\r\nDTSTAMP:20220522T203519Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:test1234\r\nSUMMARY:Test Event2\r\nDTSTART:20220526T080000Z\r\nDTEND:20220526T170000Z\r\nDTSTAMP:20220522T203633Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:PLEASEFUNZIONA\r\nSUMMARY:Test Event2\r\nDTSTART:20220527T080000Z\r\nDTEND:20220527T170000Z\r\nDTSTAMP:20220522T213945Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:asdfghjkl\r\nSUMMARY:EVENTO PROVA\r\nDTSTART:20220527T080000Z\r\nDTEND:20220527T170000Z\r\nDTSTAMP:20220522T214111Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220522-2305-0042-0000-202205211405\r\nSUMMARY:EVENTO PROVA\r\nDTSTART:20220521T143000\r\nDTEND:20220521T183000\r\nDTSTAMP:20220522T214443Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220522-2305-0000-0000-202205211405\r\nSUMMARY:XXXXX\r\nDTSTART:20220521T143000\r\nDTEND:20220521T183000\r\nDTSTAMP:20220522T215002Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220524-1505-0055-0000-202205251805\r\nSUMMARY:XXXXX\r\nDTSTART:20220525T183000\r\nDTEND:20220525T233000\r\nDTSTAMP:20220524T125557Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220522-2305-0026-0000-202205211405\r\nSUMMARY:EDITTTSSSSS\r\nDTSTART:20220525T183000\r\nDTEND:20220525T233000\r\nDTSTAMP:20220617T212451Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220522-2305-0026-0000-202341405\r\nSUMMARY:EDITED2\r\nDTSTART:20220525T183000\r\nDTEND:20220525T233000\r\nDTSTAMP:20220619T125750Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220624-1706-0020-0000-202206250806\r\nSUMMARY:Titolo\r\nDTSTART:20220625T080000\r\nDTEND:20220625T130000\r\nDTSTAMP:20220624T152322Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220624-1706-0010-0000-199912312312\r\nSUMMARY:\r\nDTSTART:19991231T230000\r\nDTEND:19991231T230000\r\nDTSTAMP:20220624T153412Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220624-1706-0049-0000-202206301406\r\nSUMMARY:Portineria\r\nDTSTART:20220630T140000\r\nDTEND:20220630T180000\r\nDTSTAMP:20220624T153551Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:20220625-0006-0003-0000-202206251306\r\nSUMMARY:PORTINERIAAAAAAA\r\nDTSTART:20220625T130000\r\nDTEND:20220625T220000\r\nDTSTAMP:20220624T223106Z\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n";
    traduce(data);
}

void MainWindow::traduce(QString data) {

    std::string s = data.toStdString();
    std::string delimiter = "\r\nBEGIN:VEVENT";
    std::vector<std::string> calendar_data;
    std::vector<Event> events;

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);

        s.erase(0, pos + delimiter.length());
        calendar_data.push_back(token);
    }

    // TODO: OPERAZIONI SUL CALENDARIO PRINCIPALE
    // calendar_data[0]

    for(int i=1; i<calendar_data.size(); i++) {

        Event my_event;
        std::string s = calendar_data[i];
        std::string delimiter = "\r\n";
        std::vector<std::string> current_event;

        size_t pos = 0;
        std::string token;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            s.erase(0, pos + delimiter.length());
            current_event.push_back(token);
            std::cout << token << std::endl;
        }

        QString ts_st = QString::fromStdString(current_event[3].substr(current_event[3].find(":")).erase(0,1));
        QString ts_en = QString::fromStdString(current_event[4].substr(current_event[4].find(":")).erase(0,1));
        QString ts_da = QString::fromStdString(current_event[5].substr(current_event[5].find(":")).erase(0,1));

        // current_event[0] -> ""
        my_event.UID = current_event[1].substr(current_event[1].find(":")).erase(0,1);
        my_event.summary = current_event[2].substr(current_event[2].find(":")).erase(0,1);
        my_event.timestamp_start = QDateTime::fromString(ts_st,"yyyyMMddTHHmmssZ");
        my_event.timestamp_end = QDateTime::fromString(ts_en,"yyyyMMddTHHmmssZ");
        my_event.creation_date = QDateTime::fromString(ts_da,"yyyyMMddTHHmmssZ");
        events.push_back(my_event);


    }



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


