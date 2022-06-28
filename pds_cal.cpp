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
#include <chrono>
#include <thread>

class Event {
  public:
    std::string UID;
    std::string summary;
    QDateTime timestamp_start;
    QDateTime timestamp_end;
    QDateTime creation_date;
};

class Calendar {
  public:
    std::string name;
    std::string color;
    std::map<std::string, Event> events;
};

std::map<std::string, Calendar> calendars;

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
        // TODO da implementare selezione calendario
        getAllEvents(login_user, password, "test-1");
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
        createEvent(user, "test-1", summary, start_date, start_time, end_time);
        ui -> success_create_event -> show();
        ui -> error_create_event -> hide();
    }

}

void MainWindow::on_getButton_clicked()
{

    QString user = "progetto-pds";
    QString password = "progetto-pds";
    QString calendar_name = "test-1";
    QString uid = "20220619-1506-0011-0000-202206101306"; // UID evento di prova
    QString summary = "ESAME PDS";
    QDate start_date(2022, 06, 10);
    QTime start_time(13, 30);
    QTime end_time(15,00);

    login(user.toStdString(), password.toStdString());
    // createEvent(user, calendar_name, summary, start_date, start_time, end_time);
    getAllEvents(user, password, calendar_name);
    // deleteEvent(user, password, calendar_name, uid);

}

void MainWindow::traduce(QString data) {

    std::string s = data.toStdString();
    std::string cal_data;
    std::string delimiter = "\r\nBEGIN:VEVENT";
    std::vector<std::string> calendar_data;
    Calendar my_calendar;

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);

        s.erase(0, pos + delimiter.length());
        calendar_data.push_back(token);
    }

    // creazione calendario locale

    if (calendar_data.empty()) {cal_data = data.toStdString();} // calendario con eventi
    else {cal_data = calendar_data[0];} // calendario senza eventi
    delimiter = "\r\n";
    std::vector<std::string> current_cal;
    size_t cal_pos = 0;
    std::string cal_token;

    while ((cal_pos = cal_data.find(delimiter)) != std::string::npos) {
            cal_token = cal_data.substr(0, cal_pos);
            cal_data.erase(0, cal_pos + delimiter.length());
            current_cal.push_back(cal_token);
            // std::cout << cal_token << std::endl;
    }

    my_calendar.name = current_cal[4].substr(current_cal[4].find(":")).erase(0,1);
    my_calendar.color = current_cal[5].substr(current_cal[5].find(":")).erase(0,1);

    // inserimento eventi calendario locale

    for(int i=1; i<calendar_data.size(); i++) {

        Event my_event;
        std::string s = calendar_data[i];
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

        my_event.UID = current_event[1].substr(current_event[1].find(":")).erase(0,1);
        my_event.summary = current_event[2].substr(current_event[2].find(":")).erase(0,1);
        my_event.timestamp_start = QDateTime::fromString(ts_st,"yyyyMMddTHHmmss");
        my_event.timestamp_end = QDateTime::fromString(ts_en,"yyyyMMddTHHmmss");
        my_event.creation_date = QDateTime::fromString(ts_da,"yyyyMMddTHHmmssZ");
        my_calendar.events[my_event.UID] = my_event;
    }

    calendars[my_calendar.name] = my_calendar;

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


    QString concatenated = user + ":" + pass;
    QByteArray user_pass = concatenated.toLocal8Bit().toBase64();
    QString header = "Basic " + user_pass;
    QString baseUrl = "https://cloud.mackers.dev/remote.php/dav/calendars/" + user + "/" + calendar_name + "?export";

    QNetworkRequest request = QNetworkRequest(baseUrl);
    request.setRawHeader("Authorization", header.toLocal8Bit());

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(report_getAllEvents(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));
    manager->get(request);

}

void MainWindow::report_getAllEvents(QNetworkReply* reply) {

    if (reply->error() == QNetworkReply::NoError) {
        QString strReply = (QString)reply->readAll();
        qDebug() << "[Getting all events]";
        traduce(strReply);
    }
    else {
        qDebug() << "[Failure]" << reply->errorString();
        delete reply;
    }
}

void MainWindow::deleteEvent(QString user, QString pass, QString calendar_name, QString uid) {

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(report_function(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    QString concatenated = user + ":" + pass;
    QByteArray user_pass = concatenated.toLocal8Bit().toBase64();
    QString header = "Basic " + user_pass;
    QString filename = uid + ".ics";
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

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QDateTime endDateTime(start_date, end_time);
    QDateTime startDateTime(start_date, start_time);
    QString uid = QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") + "-0000-" + startDateTime.toString("yyyyMMddHHMM");

    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(report_function(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    QNetworkRequest request;

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

    // Aggiungo evento localmente
    Event my_event;
    my_event.UID = uid.toStdString();
    my_event.timestamp_start.setTime(start_time);
    my_event.timestamp_end.setTime(end_time);

    // TODO controllare se giusta
    auto x = calendars[calendar_name.toStdString()].events[uid.toStdString()] = my_event;
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


