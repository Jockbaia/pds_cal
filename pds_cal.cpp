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

std::map<std::string, Calendar> calendars;
Todo selected_todo;
QString selected_cal_name;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::pds_cal)
{
    ui->setupUi(this);
    ui->success_create_event -> hide();
    ui->error_create_event -> hide();

    ui->selectedDate->setAlignment(Qt::Alignment(Qt::AlignHCenter));
    ui->selectedDate->setText("Select a day");
    ui->displayedCalendar->setSelectedDate(QDate::currentDate());

    ui->TODO_list->setSortingEnabled(true);
    ui->TODO_list->sortItems(1, Qt::SortOrder::AscendingOrder);
    ui->TODO_list->header()->resizeSection(0, 400);
    ui->error_TODO_create->hide();
    ui->error_TODO_edit->hide();

    ui->successEdit->hide();
    ui->errorEdit->hide();
    ui->successDelete->hide();
    ui->errorDelete->hide();
    ui->loading_start->hide();

    ui->deleteTodoButton->setEnabled(false);
    ui->editTodoButton->setEnabled(false);
    ui->todo_due->setDate(QDate::currentDate());
    ui->start_date_time->setDateTime(QDateTime::currentDateTime());
    ui->end_date_time->setDateTime(QDateTime::currentDateTime());
    ui->stackedWidget->setCurrentIndex(0);
    ui->tabWidget->setCurrentIndex(0);

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
        ui->loading_start->show();
        get_calendars(login_user.toStdString(), password.toStdString());
    } else {
        // TODO inserire messaggio di errore se USER / PWD sbagliata
    }
}

void MainWindow::showEventsOnDate(QDate date){
    ui->selectedDate->setText(date.toString());
    ui->listOfEvents->clear();
    QList<Event> toShow = getEventsOnDate(date);
    /*if (toShow.isEmpty()){
        // ui->eventsList->setPlainText("No events to show for this date");
        return;
    }*/
    for (Event e : toShow){
        ui->listOfEvents->addItem(e.toString());
    }

    // ui->eventsList->setPlainText(toShow);
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
    QDateTime startDateTime = ui -> start_date_time -> dateTime();
    QDateTime endDateTime = ui -> end_date_time -> dateTime();

    // TODO: Inserire selezione calendario

    if (summary.isEmpty() || startDateTime > endDateTime) {
        ui -> success_create_event -> hide();
        ui -> error_create_event -> show();
    } else {
        createEvent(user, ui->vevent_list->currentText(), summary, startDateTime, endDateTime);
        ui -> success_create_event -> show();
        ui -> error_create_event -> hide();
    }

}

void MainWindow::on_createTodoButton_clicked()
{

    QString user = ui->username_login->text(); // UID evento di prova
    QString summary = ui->todo_summary->toPlainText();
    QDateTime dueDate = ui->todo_due->dateTime();

    if(summary.isEmpty()) {
        ui->error_TODO_create->show();
    } else {
        createTODO(user, ui->vtodo_list->currentText(), summary, dueDate);
        ui->error_TODO_create->hide();
    }


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

void MainWindow::parse_vcalendar(QString data) {

    bool is_empty = false;
    std::string s = data.toStdString();
    std::string cal_data;
    // std::string delimiter = "\r\nBEGIN:VEVENT";
    std::string delimiter = "\r\nBEGIN:";
    std::vector<std::string> calendar_data;
    // Calendar my_calendar;

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

    std::string cal_name = current_cal[4].substr(current_cal[4].find(":")).erase(0,1);
    std::string cal_color = current_cal[5].substr(current_cal[5].find(":")).erase(0,1);

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

            calendars[cal_name].events[my_event.UID.toStdString()] = my_event;

            }
        }
        else if(calendar_data.size()>1 && calendar_data[1].substr(0,4) == "VTOD") {

            calendars[cal_name].is_todo = true;

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

            calendars[cal_name].todos[my_todo.UID.toStdString()] = my_todo;

            // Putting task on TODO

            QTreeWidgetItem *newItem = new QTreeWidgetItem();
            newItem->setText(0,my_todo.summary);
            newItem->setText(1,my_todo.due_to.toString("yyyy-MM-dd"));
            newItem->setText(2,QString::fromStdString(cal_name));
            newItem->setText(3,my_todo.UID);
            ui->TODO_list->addTopLevelItem(newItem);
    }

    }
}

    calendars[cal_name].color = cal_color;
    calendars[cal_name].is_shown = true;

    // Putting calendar in list

    QTreeWidgetItem *newItem = new QTreeWidgetItem();
    newItem->setText(0,QString::fromStdString(cal_name));
    if(calendars[cal_name].is_todo) {
        newItem->setText(1,"Tasks");
        ui->vtodo_list->addItem(QString::fromStdString(cal_name));
    } else {
        newItem->setText(1,"Calendar");
        ui->vevent_list->addItem(QString::fromStdString(cal_name));
    }
    if(calendars[cal_name].is_shown) newItem->setText(2,"Show");
    else newItem->setText(2,"Hide");
    ui->cal_list->addTopLevelItem(newItem);

}

void MainWindow::parse_request(QString data) {

    bool is_empty = false;
    std::string s = data.toStdString();
    std::string cal_data;
    std::string delimiter = "<d:response>";
    std::vector<std::string> request_data;
    Calendar my_calendar;

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);

        s.erase(0, pos + delimiter.length());
        request_data.push_back(token);
    }

    for(int i=2; i<request_data.size(); i++) { // i=1 -> skip header

        std::cout << request_data[i] << std::endl;

        // display_name
        unsigned first = request_data[i].find("<d:displayname>");
        unsigned end_first = first + 15;
        unsigned last = request_data[i].find("</d:displayname>");
        std::string display_name = request_data[i].substr(end_first,last - end_first);

        // c_tag
        first = request_data[i].find("<cs:getctag>");
        end_first = first + 12;
        last = request_data[i].find("</cs:getctag>");
        std::string ctag = request_data[i].substr(end_first,last - end_first);

        // calendar_name
        first = request_data[i].find("<d:href>/remote.php/dav/calendars/");
        end_first = first + 34;
        last = request_data[i].find("/</d:href>");
        std::string user_plus_cal = request_data[i].substr(end_first,last - end_first);

        first = user_plus_cal.find("/");
        end_first = first + 1;
        last = user_plus_cal.length();
        std::string calendar_name = user_plus_cal.substr(end_first,last - end_first);

        if(calendar_name != "inbox" && calendar_name != "outbox") { // disable nextcloud default inbox/outbox
            my_calendar.display_name = display_name;
            my_calendar.ctag = ctag;
            my_calendar.name = calendar_name;
            calendars[calendar_name] = my_calendar;
            getAllEvents(ui->username_login->text(), ui->password_login->text(), QString::fromStdString(calendar_name));
        }




    }

    // end first loading: opening first window
    ui->stackedWidget->setCurrentIndex(1);

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

bool MainWindow::get_calendars(std::string usr, std::string pwd) {
    QString _usr = QString::fromStdString(usr);
    QString _pwd = QString::fromStdString(pwd);
    QNetworkAccessManager *manager = new QNetworkAccessManager();

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(getCalendars_slot(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    QNetworkRequest request;

    std::string myUrl_string = "https://cloud.mackers.dev/remote.php/dav/calendars/" + usr ;
    QString my_qurl = QString::fromStdString(myUrl_string);

    request.setUrl(QUrl(my_qurl));
    request.setRawHeader("Depth", "1");
    request.setRawHeader("Prefer", "return-minimal");
    request.setRawHeader("Content-Type", "application/xml; charset=utf-8");

    QByteArray req_propfind = "<d:propfind xmlns:d=\"DAV:\" xmlns:cs=\"http://calendarserver.org/ns/\" xmlns:c=\"urn:ietf:params:icsText:ns:caldav\">\n"
                              "  <d:prop>\n"
                              "     <d:resourcetype />\n"
                              "     <d:displayname />\n"
                              "     <cs:getctag />\n"
                              "     <c:supported-calendar-component-set />\n"
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

void MainWindow::getCalendars_slot(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "[Logged]";
        QString strReply = (QString)reply->readAll();
        parse_request(strReply);
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
        parse_vcalendar(strReply);
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
    // QString response = reply->readAll();
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

void MainWindow::createEvent(QString user, QString calendar_name, QString summary, QDateTime start_date_time, QDateTime end_date_time) {

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QString uid = QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") + "-0000-" + start_date_time.toString("yyyyMMddHHMM");

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
            "DTSTART:" + start_date_time.toString("yyyyMMddTHHmmss") + "\r\n"
            "DTEND:" + end_date_time.toString("yyyyMMddTHHmmss") + "\r\n"
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
    my_event.timestamp_start = start_date_time;
    my_event.timestamp_end = end_date_time;

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
    newItem->setText(2,calendar_name);
    newItem->setText(3, my_todo.UID);
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

void MainWindow::editEvent(QString user, QString uid, QString calendar_name, QString summary, QDateTime start_date_time, QDateTime end_date_time) {
    QNetworkAccessManager *manager = new QNetworkAccessManager();

    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(report_function(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    QNetworkRequest request;

    QString myUrl = "https://cloud.mackers.dev/remote.php/dav/calendars/" + user + "/" + calendar_name + "/" + uid + ".ics";
    request.setUrl(QUrl(myUrl));
    request.setRawHeader("Depth", "1");
    request.setRawHeader("Prefer", "return-minimal");
    // request.setRawHeader("If-None-Match", "*");
    request.setRawHeader("Content-Type", "application/xml; charset=utf-8");

    QString request_report = "BEGIN:VCALENDAR\n"
            "BEGIN:VEVENT\n"
            "UID:" + uid + "\n"
            "SUMMARY:" + summary + "\n"
            "DTSTART:" + start_date_time.toString("yyyyMMddTHHmmss") + "\r\n"
            "DTEND:" + end_date_time.toString("yyyyMMddTHHmmss") + "\r\n"
            "END:VEVENT\n"
            "END:VCALENDAR\n";

    QByteArray converted_report = request_report.toUtf8();

    QNetworkReply *reply = manager->put(request, converted_report);
    QString response = reply->readAll();
    qDebug() << "[Edit Event] " << reply;

    // Modify event locally
    Event *e = &calendars[calendar_name.toStdString()].events[uid.toStdString()];

    e->UID = uid;
    e->summary = summary;
    e->timestamp_start = start_date_time;
    e->timestamp_end = end_date_time;
    e->cal_ptr = &calendars[calendar_name.toStdString()];
}

void MainWindow::on_displayedCalendar_clicked(const QDate &date)
{
    showEventsOnDate(date);
}


void MainWindow::on_editButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);

}


void MainWindow::on_closeEditButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->errorEdit->hide();
    ui->successEdit->hide();
}


void MainWindow::on_confirmEditButton_clicked()
{
    if (ui->listOfEvents->currentItem() == nullptr){
        // If no event is selected, show error message
        ui->errorEdit->show();
        ui->successEdit->hide();
        return;
    }

    QString user = ui->username_login->text();
    QString summary = ui->titleEdit->text();
    QDateTime start_date_time = ui->startDateTimeEdit->dateTime();
    QDateTime end_date_time = ui->endDateTimeEdit->dateTime();

    // Get uid from selected event in list
    QString event_data = ui->listOfEvents->currentItem()->text();
    std::string s = event_data.toStdString();
    std::string delimiter = "UID: ";
    size_t pos = 0;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        s.erase(0, pos + delimiter.length());
    }
    s.erase(s.length() - 1); // erase \n ?
    QString uid = QString::fromStdString(s);


    // TODO: Inserire selezione calendario

    if (summary.isEmpty() || start_date_time > end_date_time) {
        ui -> successEdit -> hide();
        ui -> errorEdit -> show();
    } else {
        editEvent(user, uid, "personal", summary, start_date_time, end_date_time);
        ui -> successEdit -> show();
        ui -> errorEdit -> hide();
    }

}


void MainWindow::on_cancelEditButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->errorEdit->hide();
    ui->successEdit->hide();
}


void MainWindow::on_deleteButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
}


void MainWindow::on_goBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->errorDelete->hide();
    ui->successDelete->hide();
}


void MainWindow::on_confirmDelete_clicked()
{
    if (ui->listOfEvents->currentItem() == nullptr){
        // If no event is selected, show error message
        ui->errorDelete->show();
        ui->successDelete->hide();
        return;
    }

    QString user = ui->username_login->text();
    QString password = ui->password_login->text();

    // Get uid from selected event in list
    QString event_data = ui->listOfEvents->currentItem()->text();
    std::string s = event_data.toStdString();
    std::string delimiter = "UID: ";
    size_t pos = 0;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        s.erase(0, pos + delimiter.length());
    }
    s.erase(s.length() - 1); // erase \n ?
    QString uid = QString::fromStdString(s);


    // TODO: Inserire selezione calendario
    deleteEvent(user, password, "personal", uid);
    ui -> successDelete -> show();
    ui -> errorDelete -> hide();

}


void MainWindow::on_listOfEvents_itemClicked(QListWidgetItem *item)
{

    // TODO: Fare in modo che selezionando l'evento
    // si autocompletino i campi nella schermata "Edit event"
    // così se non li cambi restano come sono (ora vanno a 00.00 del 01.01.2000)

}


void MainWindow::on_TODO_list_itemClicked(QTreeWidgetItem *item, int column)
{

    selected_todo.summary = item->text(0);
    selected_todo.due_to = QDateTime::fromString(item->text(1),"yyyy-MM-dd");
    selected_cal_name = item->text(2);
    selected_todo.UID = item->text(3);
    ui->textTODOedit->setText(item->text(0));
    auto s = item->text(1).toStdString();
    auto date = QDate::fromString(QString::fromStdString(s),"yyyy-MM-dd");

    ui->dateTODOedit->setDate(date); // TODO da finire
    ui->editTodoButton->setEnabled(true);
    ui->deleteTodoButton->setEnabled(true);
    ui->error_TODO_edit->hide();
    ui->success_TODO_edit->hide();
}

void MainWindow::on_deleteTodoButton_clicked()
{

    deleteEvent(ui->username_login->text(), ui->username_login->text(), selected_cal_name, selected_todo.UID);
    calendars[selected_cal_name.toStdString()].todos.erase(selected_todo.UID.toStdString());
    ui->editTodoButton->setEnabled(false);
    ui->deleteTodoButton->setEnabled(false);
    delete ui->TODO_list->currentItem();

}

void MainWindow::on_editTodoButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(4);
}

void MainWindow::editTODO(QString user, QString calendar_name, QString summary, QDateTime new_due, QString uid) {

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(report_function(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    QNetworkRequest request;

    QString myUrl = "https://cloud.mackers.dev/remote.php/dav/calendars/" + user + "/" + calendar_name + "/" + uid + ".ics";
    request.setUrl(QUrl(myUrl));
    request.setRawHeader("Depth", "1");
    request.setRawHeader("Prefer", "return-minimal");
    // request.setRawHeader("If-None-Match", "*");
    request.setRawHeader("Content-Type", "application/xml; charset=utf-8");

    QString request_report = "BEGIN:VCALENDAR\n"
            "BEGIN:VTODO\n"
            "UID:" + uid + "\n"
            "SUMMARY:" + summary + "\n"
            "DUE;VALUE=DATE:" + new_due.toString("yyyyMMdd") + "\n"
            "END:VTODO\n"
            "END:VCALENDAR\n";

    QByteArray converted_report = request_report.toUtf8();

    QNetworkReply *reply = manager->put(request, converted_report);
    QString response = reply->readAll();
    qDebug() << "[Add Event] " << reply;

    // Modifico evento localmente

    calendars[calendar_name.toStdString()].todos[uid.toStdString()].due_to = new_due;
    calendars[calendar_name.toStdString()].todos[uid.toStdString()].summary = summary;

}


void MainWindow::on_backTODOedit_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}


void MainWindow::on_backSAVEedit_clicked()
{

    QString myTitle = ui->textTODOedit->toPlainText();
    if(myTitle.isEmpty()) {
        ui->error_TODO_edit->show();
        ui->success_TODO_edit->hide();
    } else {

        editTODO(ui->username_login->text(), selected_cal_name, ui->textTODOedit->toPlainText(), ui->dateTODOedit->dateTime(), selected_todo.UID);

        // replace task with new version
        delete ui->TODO_list->currentItem();
        QTreeWidgetItem *newItem = new QTreeWidgetItem();
        newItem->setText(0,ui->textTODOedit->toPlainText());
        newItem->setText(1,ui->dateTODOedit->dateTime().toString("yyyy-MM-dd"));
        newItem->setText(2,selected_cal_name);
        newItem->setText(3,selected_todo.UID);
        ui->TODO_list->addTopLevelItem(newItem);

        ui->success_TODO_edit->show();
        ui->error_TODO_edit->hide();

    }


}

