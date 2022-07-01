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

class Calendar {
  public:
    std::string name;
    std::string color;
    std::map<std::string, Event> events;
    std::map<std::string, Todo> todos;
};

std::map<std::string, Calendar> calendars;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::pds_cal)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->success_create_event -> hide();
    ui->error_create_event -> hide();

    ui->selectedDate->setAlignment(Qt::Alignment(Qt::AlignHCenter));
    ui->selectedDate->setText("Select a day");
    ui->displayedCalendar->setSelectedDate(QDate::currentDate());
    ui->displayedCalendar->setSelectionMode(QCalendarWidget::SingleSelection);
    ui->TODO_list->setSortingEnabled(true);
    ui->TODO_list->sortItems(1, Qt::SortOrder::AscendingOrder);
    ui->TODO_list->header()->resizeSection(0, 400);

    // TODO fix this connect, the slot is not called
    connect(ui->displayedCalendar, SIGNAL(QCalendarWidget::activated(QDate)),
                     this, SLOT(MainWindow::on_displayedCalendar_clicked(QDate)));
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
        getAllEvents(login_user, password, "test-todo");
    } else {
        // TODO inserire messaggio di errore se USER / PWD sbagliata
    }
}

void MainWindow::showEventsOnDate(QDate date){
    ui->selectedDate->setText(date.toString());
    QString toShow = eventsListToString(getEventsOnDate(date));
    if (toShow.isEmpty()){
        ui->eventsList->setPlainText("No events to show for this date");
        return;
    }
    ui->eventsList->setPlainText(toShow);
    //ui->eventsList->insertPlainText(eventsListToString(getEventsOnDate(date)));
}

QList<Event> MainWindow::getEventsOnDate(QDate date){
    QList<Event> toReturn;
    for(auto const& cal : calendars){
        std::map<std::string, Event> events = cal.second.events;
        for (auto const& ev : events){
            Event e = ev.second;
            if (e.timestamp_start.date() == date ||
                    e.timestamp_end.date() == date){
                // the event either starts or ends (or both) on the requested date
                toReturn.append(e);
            }
        }
    }
    return toReturn;
}

QString MainWindow::eventsListToString(QList<Event> eventsList){
    QString formatted;
    for (auto event : eventsList){
        formatted = formatted + event.toString() + "\n";
    }
    return formatted;
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

void MainWindow::on_createTodoButton_clicked()
{

    QString user = ui->username_login->text(); // UID evento di prova
    QString summary = ui->todo_summary->toPlainText();
    QDateTime dueDate = ui->todo_due->dateTime();

    // TODO: aggiungere calendario
    createTODO(user, "test-todo", summary, dueDate);

}

void MainWindow::on_deleteTodoButton_clicked()
{

    // TODO da finire

}



void MainWindow::on_getButton_clicked()
{

    QString user = "progetto-pds";
    QString password = "progetto-pds";
    QString calendar_name = "test-1";
    QString uid = "20220619-1506-0011-0000-202206101306"; // UID evento di prova
    QString summary = "TEST TODO";
    QDate start_date(2022, 06, 10);
    QTime start_time(13, 30);
    QTime end_time(15,00);

    QDateTime endDateTime(start_date, end_time);

    login(user.toStdString(), password.toStdString());
    createTODO(user, calendar_name, summary, endDateTime);
    // getAllEvents(user, password, calendar_name);
    // deleteEvent(user, password, calendar_name, uid);

}

void MainWindow::traduce(QString data) {

    bool is_empty = false;
    std::string s = data.toStdString();
    std::string cal_data;
    // std::string delimiter = "\r\nBEGIN:VEVENT";
    std::string delimiter = "\r\nBEGIN:";
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

    if (calendar_data.empty()) {
        is_empty = true;
        cal_data = data.toStdString();
    } // calendario senza eventi
    else {cal_data = calendar_data[0];} // calendario con eventi
    delimiter = "\r\n";
    std::vector<std::string> current_cal;
    size_t cal_pos = 0;
    std::string cal_token;

    while ((cal_pos = cal_data.find(delimiter)) != std::string::npos) {
            cal_token = cal_data.substr(0, cal_pos);
            cal_data.erase(0, cal_pos + delimiter.length());
            current_cal.push_back(cal_token);
    }

    my_calendar.name = current_cal[4].substr(current_cal[4].find(":")).erase(0,1);
    my_calendar.color = current_cal[5].substr(current_cal[5].find(":")).erase(0,1);

    // inserimento eventi calendario locale
if(!is_empty) {
    if(calendar_data.size()>1 && calendar_data[1].substr(0,4) == "VEVE") {

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

        my_event.UID = QString::fromStdString(current_event[1].substr(current_event[1].find(":")).erase(0,1));
        my_event.summary = QString::fromStdString(current_event[2].substr(current_event[2].find(":")).erase(0,1));
        my_event.timestamp_start = QDateTime::fromString(ts_st,"yyyyMMddTHHmmss");
        my_event.timestamp_end = QDateTime::fromString(ts_en,"yyyyMMddTHHmmss");
        my_event.creation_date = QDateTime::fromString(ts_da,"yyyyMMddTHHmmssZ");
        my_calendar.events[my_event.UID.toStdString()] = my_event;

    }} else if(calendar_data.size()>1 && calendar_data[1].substr(0,4) == "VTOD") {

        for(int i=1; i<calendar_data.size(); i++) {

            Todo my_todo;
            std::string s = calendar_data[i];
            std::vector<std::string> current_todo;

            size_t pos = 0;
            std::string token;
            while ((pos = s.find(delimiter)) != std::string::npos) {
                token = s.substr(0, pos);
                s.erase(0, pos + delimiter.length());
                current_todo.push_back(token);
                std::cout << token << std::endl;
            }

            QString ts_due = QString::fromStdString(current_todo[3].substr(current_todo[3].find(":")).erase(0,1));
            QString ts_tst = QString::fromStdString(current_todo[4].substr(current_todo[4].find(":")).erase(0,1));

            my_todo.UID = QString::fromStdString(current_todo[1].substr(current_todo[1].find(":")).erase(0,1));
            my_todo.summary = QString::fromStdString(current_todo[2].substr(current_todo[2].find(":")).erase(0,1));
            my_todo.due_to = QDateTime::fromString(ts_due,"yyyyMMdd");
            my_todo.creation_date = QDateTime::fromString(ts_tst,"yyyyMMddTHHmmss");
            my_calendar.todos[my_todo.UID.toStdString()] = my_todo;

            // Putting task on TODO

            QTreeWidgetItem *newItem = new QTreeWidgetItem();
            newItem->setText(0,my_todo.summary);
            newItem->setText(1,my_todo.due_to.toString("yyyy-MM-dd"));
            newItem->setText(2,my_todo.UID);
            ui->TODO_list->addTopLevelItem(newItem);
    }

    }
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
        qDebug() << strReply;
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

    // Elimino evento localmente

    calendars[calendar_name.toStdString()].events.erase(uid.toStdString());

}

void MainWindow::deleteTODO(QString user, QString pass, QString calendar_name, QString uid) {

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

    // Elimino evento localmente

    calendars[calendar_name.toStdString()].todos.erase(uid.toStdString());

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
    my_event.UID = uid;
    my_event.summary = summary;
    my_event.timestamp_start = startDateTime;
    my_event.timestamp_end = endDateTime;

    calendars[calendar_name.toStdString()].events[uid.toStdString()] = my_event;

}

void MainWindow::createTODO(QString user, QString calendar_name, QString summary, QDateTime end_date) {

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QDateTime current_datetime = QDateTime::currentDateTime();
    QString uid = current_datetime.toString("yyyyMMdd-HHMM-00ss"); + "-0000-" + end_date.toString("yyyyMMddHHMM");

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
            "BEGIN:VTODO\n"
            "UID:" + uid + "\n"
            "SUMMARY:" + summary + "\n"
            "DUE;VALUE=DATE:" + end_date.toString("yyyyMMdd") + "\n"
            "DTSTAMP:" + current_datetime.toString("yyyyMMddTHHmmssZ") + "\n"
            "END:VTODO\n"
            "END:VCALENDAR\n";

    QByteArray converted_report = request_report.toUtf8();

    QNetworkReply *reply = manager->put(request, converted_report);
    QString response = reply->readAll();
    qDebug() << "[Add Event] " << reply;

    // Aggiungo todo localmente

    Todo my_todo;
    my_todo.UID = uid;
    my_todo.summary = summary;
    my_todo.due_to = end_date;
    my_todo.creation_date = current_datetime;

    calendars[calendar_name.toStdString()].todos[uid.toStdString()] = my_todo;

    // Putting task on TODO

    QTreeWidgetItem *newItem = new QTreeWidgetItem();
    newItem->setText(0,my_todo.summary);
    newItem->setText(1,my_todo.due_to.toString("yyyy-MM-dd"));
    newItem->setText(2,my_todo.UID);
    ui->TODO_list->addTopLevelItem(newItem);

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



void MainWindow::on_displayedCalendar_clicked(const QDate &date)
{
    showEventsOnDate(date);
}

