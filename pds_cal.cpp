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

#define SECONDS 10


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::pds_cal)
{

    // First GUI setup

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
    ui->checkCompleted->setChecked(false);
    ui->successEdit->hide();
    ui->errorEdit->hide();
    ui->successDelete->hide();
    ui->errorDelete->hide();
    ui->loading_start->hide();
    ui->parsing_alert->show();
    ui->success_login->hide();
    ui->deleteTodoButton->setEnabled(false);
    ui->editTodoButton->setEnabled(false);
    ui->delete_calendar_btn->setEnabled(false);
    ui->share_calendar_btn->setEnabled(false);
    ui->todo_due->setDate(QDate::currentDate());
    ui->start_date_time->setDateTime(QDateTime::currentDateTime());
    ui->end_date_time->setDateTime(QDateTime::currentDateTime());
    ui->stackedWidget->setCurrentIndex(0);
    ui->tabWidget->setCurrentIndex(0);
    ui->create_cal_success->hide();
    ui->create_cal_error->hide();
    ui->share_cal_success->hide();
    ui->share_cal_error->hide();
    ui->editButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
    ui->login_error->hide();
    ui->cloud_changes->hide();
    ui->cloud_changes_2->hide();
    ui->cloud_changes_3->hide();
    ui->cloud_changes_4->hide();
    ui->cloud_changes_5->hide();
    ui->cloud_changes_6->hide();
    ui->cloud_changes_7->hide();
    ui->cloud_changes_8->hide();
    ui->cloud_changes_9->hide();
    ui->cloud_changes_10->hide();
    
    connect(ui->displayedCalendar, SIGNAL(QCalendarWidget::activated(QDate)),
            this, SLOT(MainWindow::on_displayedCalendar_clicked(QDate)));
    
    synch_timer = new QTimer(this);
    synch_timer->setInterval(SECONDS * 1000);
    synch_timer->setSingleShot(true);
    cal_man.is_logged = false;
    
    connect(synch_timer, &QTimer::timeout,
            this, &MainWindow::startSynchronization);
    
}

MainWindow::~MainWindow()
{
    delete ui;
}


/*
 * ******* PARSING *********
*/

void MainWindow::parse_vcalendar(QString data) {

    bool is_empty = false;
    std::string s = data.toStdString();
    std::string cal_data;
    std::string delimiter = "\r\nBEGIN:";
    std::vector<std::string> calendar_data;

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);

        s.erase(0, pos + delimiter.length());
        calendar_data.push_back(token);
    }

    if(!s.empty()) calendar_data.push_back(s);


    // Creating calendar locally

    if (calendar_data.empty()) {
        is_empty = true;
        cal_data = data.toStdString();
    } // Calendar with no events
    else {cal_data = calendar_data[0];} // Calendar with events
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
    std::string display_name;

    display_name = cal_name;

    for (auto cal : cal_man.calendars){
        if(cal.second.display_name == cal_name) {
            cal_name = cal.second.name;
        }
    }

    // Some events have a set color

    std::string cal_color;
    if(!cal_man.is_new) cal_color = current_cal[5].substr(current_cal[5].find(":")).erase(0,1);
    else cal_color = "#b33b3b";

    // Check extra nextcloud-driven settings

    int big_header_data = 0;

    std::string a1 = "BEGIN:VTIMEZONE";
    if (data.toStdString().find(a1) != std::string::npos) {
        big_header_data++;
    }

    std::string a2 = "BEGIN:DAYLIGHT";
    if (data.toStdString().find(a2) != std::string::npos) {
        big_header_data++;
    }

    std::string a3 = "BEGIN:STANDARD";
    if (data.toStdString().find(a3) != std::string::npos) {
        big_header_data++;
    }

    // Inserting events on local calendar

    if(!is_empty) {
        cal_man.is_new = false;
        if(calendar_data.size()>1 && calendar_data[1 + big_header_data].substr(0,4) == "VEVE") {

            for(int i=1 + big_header_data; i<calendar_data.size(); i++) {

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

                int end_first;
                int first;
                int last;

                QString myUID;
                QString myDTSTART;
                QString myDTEND;
                QString mySUMMARY;
                QString myCREATED;


                for (auto str : current_event){
                    if (str.substr(0,3) == "UID") {
                        first = str.find(":");
                        end_first = first + 1;
                        last = str.length();
                        myUID = QString::fromStdString(str.substr(end_first,last - end_first));
                    } else if (str.substr(0,7) == "DTSTART") {
                        first = str.find(":");
                        end_first = first + 1;
                        last = str.length();
                        myDTSTART = QString::fromStdString(str.substr(end_first,last - end_first));
                    } else if (str.substr(0,5) == "DTEND") {
                        first = str.find(":");
                        end_first = first + 1;
                        last = str.length();
                        myDTEND = QString::fromStdString(str.substr(end_first,last - end_first));
                    } else if (str.substr(0,7) == "SUMMARY") {
                        first = str.find(":");
                        end_first = first + 1;
                        last = str.length();
                        mySUMMARY = QString::fromStdString(str.substr(end_first,last - end_first));
                    } else if (str.substr(0,7) == "CREATED") {
                        first = str.find(":");
                        end_first = first + 1;
                        last = str.length();
                        myCREATED = QString::fromStdString(str.substr(end_first,last - end_first));
                    }
                }

                my_event.UID = myUID;
                my_event.summary = mySUMMARY;
                my_event.timestamp_start = QDateTime::fromString(myDTSTART,"yyyyMMddTHHmmss");
                my_event.timestamp_end = QDateTime::fromString(myDTEND,"yyyyMMddTHHmmss");
                my_event.creation_date = QDateTime::fromString(myCREATED,"yyyyMMddTHHmmssZ");

                cal_man.calendars[cal_name].events[my_event.UID.toStdString()] = my_event;

            }
        }
        else if(calendar_data.size()>1 && calendar_data[1 + big_header_data].substr(0,4) == "VTOD") {

            cal_man.calendars[cal_name].is_todo = true;

            for(int i=1 + big_header_data; i<calendar_data.size(); i++) {

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

                std::string s2 = "COMPLETED";
                if (calendar_data[i].find(s2) != std::string::npos) {
                    my_todo.completed = true;
                } else my_todo.completed = false;

                cal_man.calendars[cal_name].todos[my_todo.UID.toStdString()] = my_todo;

                // Putting task on TODO

                QTreeWidgetItem *newItem = new QTreeWidgetItem();
                newItem->setText(0,my_todo.summary);
                newItem->setText(1,my_todo.due_to.toString("yyyy-MM-dd"));
                newItem->setText(2,QString::fromStdString(display_name));
                newItem->setText(4,my_todo.UID);

                if(my_todo.completed == false) newItem->setText(3, "Due");
                else newItem->setText(3, "Completed");

                ui->TODO_list->addTopLevelItem(newItem);
            }

        }
    }

    cal_man.calendars[cal_name].color = cal_color;
    cal_man.calendars[cal_name].is_shown = true;
    cal_man.calendars[cal_name].name = cal_name;
    cal_man.calendars[cal_name].display_name = display_name;

    // Inserting calendar on list

    QTreeWidgetItem *newItem = new QTreeWidgetItem();
    newItem->setText(0,QString::fromStdString(display_name));

    if(cal_man.is_new) { // Created event
        if(ui->create_cal_type->currentText() == "Calendar") {
            ui->vevent_list->addItem(QString::fromStdString(display_name));
            newItem->setText(1,"Calendar");
        } else {
            ui->vtodo_list->addItem(QString::fromStdString(display_name));
            newItem->setText(1,"Tasks");
        }
    } else {

        // Imported event

        if(cal_man.calendars[cal_name].is_todo) {
            newItem->setText(1,"Tasks");
            ui->vtodo_list->addItem(QString::fromStdString(display_name));
        } else {
            newItem->setText(1,"Calendar");
            ui->vevent_list->addItem(QString::fromStdString(display_name));
        }
        if(cal_man.calendars[cal_name].is_shown) newItem->setText(2,"Show");
        else newItem->setText(2,"Hide");
        ui->cal_list->addTopLevelItem(newItem);
    }

    if(cal_man.calendars[cal_name].is_shown) newItem->setText(2,"Show");
    else newItem->setText(2,"Hide");
    ui->cal_list->addTopLevelItem(newItem);
    cal_man.is_new = false;

    // Coming from a sync? Hide alerts!

    ui->cloud_changes->hide();
    ui->cloud_changes_2->hide();
    ui->cloud_changes_3->hide();
    ui->cloud_changes_4->hide();
    ui->cloud_changes_5->hide();
    ui->cloud_changes_6->hide();
    ui->cloud_changes_7->hide();
    ui->cloud_changes_8->hide();
    ui->cloud_changes_9->hide();
    ui->cloud_changes_10->hide();

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

    for(int i=2; i<request_data.size(); i++) { // i=1 -> Skip header

        std::cout << request_data[i] << std::endl;

        // Getting display_name
        unsigned first = request_data[i].find("<d:displayname>");
        unsigned end_first = first + 15;
        unsigned last = request_data[i].find("</d:displayname>");
        std::string display_name = request_data[i].substr(end_first,last - end_first);

        // Getting c_tag
        first = request_data[i].find("<cs:getctag>");
        end_first = first + 12;
        last = request_data[i].find("</cs:getctag>");
        std::string ctag = request_data[i].substr(end_first,last - end_first);

        // Getting calendar_name
        first = request_data[i].find("<d:href>/remote.php/dav/calendars/");
        end_first = first + 34;
        last = request_data[i].find("/</d:href>");
        std::string user_plus_cal = request_data[i].substr(end_first,last - end_first);

        first = user_plus_cal.find("/");
        end_first = first + 1;
        last = user_plus_cal.length();
        std::string calendar_name = user_plus_cal.substr(end_first,last - end_first);

        if(calendar_name != "inbox" && calendar_name != "outbox") { // Disabling nextcloud default inbox/outbox
            my_calendar.display_name = display_name;
            my_calendar.ctag = ctag;
            my_calendar.name = calendar_name;
            cal_man.calendars[calendar_name] = my_calendar;
            getAllEvents(ui->username_login->text(), ui->password_login->text(), QString::fromStdString(calendar_name));
        }

    }

    // End of first loading: opening first window

    ui->stackedWidget->setCurrentIndex(1);

}

/*
 * ******* AUTH *********
*/

bool MainWindow::login(std::string usr, std::string pwd) {
    QString _usr = QString::fromStdString(usr);
    QString _pwd = QString::fromStdString(pwd);
    QNetworkAccessManager *manager = new QNetworkAccessManager();

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(login_slot(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    QNetworkRequest request;

    std::string myUrl_string = "https://cloud.mackers.dev/remote.php/dav/calendars/" + usr ;
    QString my_qurl = QString::fromStdString(myUrl_string);

    request.setUrl(QUrl(my_qurl));

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

/*
 * ******* QREPLYS *********
*/

void MainWindow::do_authentication(QNetworkReply *, QAuthenticator *q) {
    q->setUser(ui->username_login->text());
    q->setPassword(ui->password_login->text());
}

void MainWindow::login_slot(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {

        qDebug() << "[Logged]";
        QString strReply = (QString)reply->readAll();
        cal_man.is_logged = true;
        cal_man.user = ui->username_login->text();
        cal_man.password = ui->password_login->text();
        ui->login_error->hide();
        ui->loading_start->hide();
        ui->success_login->show();
        get_calendars(cal_man.user.toStdString(), cal_man.password.toStdString());

    }
    else {

        qDebug() << "[Failure]" << reply -> errorString();
        delete reply;
        cal_man.is_logged = false;
        ui->login_error->show();
        ui->loading_start->hide();
        ui->success_login->hide();

    }
}

void MainWindow::getCalendars_slot(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "[Getting calendars]";
        QString strReply = (QString)reply->readAll();
        parse_request(strReply);

        // First activation of the timer

        synch_timer->start();

    }
    else {
        qDebug() << "[Failure]" << reply -> errorString();
        delete reply;
    }
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

void MainWindow::report_getAllEvents(QNetworkReply* reply) {

    if (reply->error() == QNetworkReply::NoError) {
        QString strReply = (QString)reply->readAll();
        qDebug() << "[Getting all events]";
        qDebug() << strReply;
        parse_vcalendar(strReply);
        ui->parsing_alert->hide();
        ui->parsing_alert_2->hide();
        ui->parsing_alert_3->hide();
    }
    else {
        qDebug() << "[Failure]" << reply->errorString();
        delete reply;
    }
}

void MainWindow::createCalendar_slot(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "[Created]";
        QString strReply = (QString)reply->readAll();
        getAllEvents(ui->username_login->text(), ui->password_login->text(), ui->create_cal_name->toPlainText());
    }
    else {
        qDebug() << "[Failure]" << reply -> errorString();
        delete reply;
    }
}

void MainWindow::deleteCalendar_slot(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "[Deleted]";
        QString strReply = (QString)reply->readAll();
        delete ui->cal_list->currentItem();

        if (cal_man.selected_cal.is_todo) {
            ui->vtodo_list->removeItem(ui->vtodo_list->findText(QString::fromStdString(cal_man.selected_cal.name), Qt::MatchExactly));
            refresh_todos();
        } else {
            ui->vevent_list->removeItem(ui->vevent_list->findText(QString::fromStdString(cal_man.selected_cal.name), Qt::MatchExactly));
        }

        ui->delete_calendar_btn->setEnabled(false);

        cal_man.calendars.erase(cal_man.selected_cal.name);
    }
    else {
        qDebug() << "[Failure]" << reply -> errorString();
        delete reply;
    }
}

void MainWindow::shareCalendar_slot(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "[Shared]";
        QString strReply = (QString)reply->readAll();
    }
    else {
        qDebug() << "[Failure]" << reply -> errorString();
        delete reply;
    }
}

/*
 * ******* EVENTS *********
*/

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

    // Adding event locally

    Event my_event;
    my_event.UID = uid;
    my_event.summary = summary;
    my_event.timestamp_start = start_date_time;
    my_event.timestamp_end = end_date_time;

    cal_man.calendars[calendar_name.toStdString()].events[uid.toStdString()] = my_event;

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

    Event *e = &cal_man.calendars[calendar_name.toStdString()].events[uid.toStdString()];

    e->UID = uid;
    e->summary = summary;
    e->timestamp_start = start_date_time;
    e->timestamp_end = end_date_time;

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
    qDebug() << "[Deleting Event] " << reply;

    // Deleting event locally

    cal_man.calendars[calendar_name.toStdString()].events.erase(uid.toStdString());

}

QList<Event> MainWindow::getEventsOnDate(QDate date){
    QList<Event> toReturn;
    for(auto const& cal : cal_man.calendars){
        std::map<std::string, Event> events = cal.second.events;
        for (auto const& ev : events){
            Event e = ev.second;
            if (e.timestamp_start.date() == date ||
                    e.timestamp_end.date() == date){
                // The event either starts or ends (or both) on the requested date
                toReturn.append(e);
            }
        }
    }
    return toReturn;
}

void MainWindow::showEventsOnDate(QDate date){
    ui->selectedDate->setText(date.toString());
    ui->listOfEvents->clear();
    QList<Event> toShow = getEventsOnDate(date);

    for (Event e : toShow){
        ui->listOfEvents->addItem(e.toString());
    }
}

QString MainWindow::eventsListToString(QList<Event> eventsList){
    QString formatted;
    for (auto event : eventsList){
        formatted = formatted + event.toString() + "\n";
    }
    return formatted;
}

/*
 * ******* TODOS *********
*/

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

    // Adding todo locally

    Todo my_todo;
    my_todo.UID = uid;
    my_todo.summary = summary;
    my_todo.due_to = end_date;
    my_todo.creation_date = current_datetime;
    my_todo.completed = false; // Makes no sense to create a todo when it's already done
    cal_man.calendars[calendar_name.toStdString()].todos[uid.toStdString()] = my_todo;
    QString display_name = QString::fromStdString(cal_man.calendars[calendar_name.toStdString()].display_name);

    // Putting task on TODO

    QTreeWidgetItem *newItem = new QTreeWidgetItem();
    newItem->setText(0, my_todo.summary);
    newItem->setText(1, my_todo.due_to.toString("yyyy-MM-dd"));
    newItem->setText(2, display_name);
    newItem->setText(4, my_todo.UID);
    newItem->setText(3, "Due");
    ui->TODO_list->addTopLevelItem(newItem);

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

    // Deleting event locally

    cal_man.calendars[calendar_name.toStdString()].todos.erase(uid.toStdString());

}

void MainWindow::editTODO(QString user, QString calendar_name, QString summary, QDateTime new_due, QString uid, bool completed) {

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(report_function(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    QNetworkRequest request;

    QString myUrl = "https://cloud.mackers.dev/remote.php/dav/calendars/" + user + "/" + calendar_name + "/" + uid + ".ics";
    request.setUrl(QUrl(myUrl));
    request.setRawHeader("Depth", "1");
    request.setRawHeader("Prefer", "return-minimal");
    request.setRawHeader("Content-Type", "application/xml; charset=utf-8");

    QString request_report = "BEGIN:VCALENDAR\n"
                             "BEGIN:VTODO\n"
                             "UID:" + uid + "\n"
                                            "SUMMARY:" + summary + "\n"
                                                                   "DUE;VALUE=DATE:" + new_due.toString("yyyyMMdd");
    if (completed){
        request_report.append("\nCOMPLETED:");
        request_report.append(QDateTime::currentDateTime().toString("yyyyMMddTHHmmss"));
    }
    request_report.append("\nEND:VTODO\nEND:VCALENDAR\n");

    QByteArray converted_report = request_report.toUtf8();

    QNetworkReply *reply = manager->put(request, converted_report);
    QString response = reply->readAll();
    qDebug() << "[Add Event] " << reply;

    // Editing event locally

    cal_man.calendars[calendar_name.toStdString()].todos[uid.toStdString()].due_to = new_due;
    cal_man.calendars[calendar_name.toStdString()].todos[uid.toStdString()].summary = summary;
    if(completed) cal_man.calendars[calendar_name.toStdString()].todos[uid.toStdString()].completed = true;

}

/*
 * ******* CALENDAR *********
*/

void MainWindow::create_calendar(std::string usr, std::string pwd, std::string calendar_name) {

    QString _usr = QString::fromStdString(usr);
    QString _pwd = QString::fromStdString(pwd);
    QNetworkAccessManager *manager = new QNetworkAccessManager();

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(createCalendar_slot(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    QNetworkRequest request;

    std::string myUrl_string = "https://cloud.mackers.dev/remote.php/dav/calendars/" + usr + "/" + calendar_name;
    QString my_qurl = QString::fromStdString(myUrl_string);

    request.setUrl(QUrl(my_qurl));
    request.setRawHeader("Depth", "1");
    request.setRawHeader("Prefer", "return-minimal");
    request.setRawHeader("Content-Type", "application/xml; charset=utf-8");
    std::string cal_type;

    std::string req_mkcalendar = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
                                 "   <C:mkcalendar xmlns:D=\"DAV:\"\n"
                                 "                 xmlns:C=\"urn:ietf:params:xml:ns:caldav\">\n"
                                 "     <D:set>\n"
                                 "       <D:prop>\n"
                                 "         <D:displayname>" + calendar_name + "</D:displayname>\n"
                                                                              "       </D:prop>\n"
                                                                              "     </D:set>\n"
                                                                              "   </C:mkcalendar>";

    QByteArray request_mk;
    request_mk.append(req_mkcalendar);

    QNetworkReply *reply = manager->sendCustomRequest(request,"MKCALENDAR", request_mk);
    QString response = reply->readAll();
    qDebug() << "[Creation calendar] " << reply;


}

void MainWindow::delete_calendar(std::string usr, std::string pwd, std::string calendar_name) {

    QString _usr = QString::fromStdString(usr);
    QString _pwd = QString::fromStdString(pwd);
    QNetworkAccessManager *manager = new QNetworkAccessManager();

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(deleteCalendar_slot(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    QNetworkRequest request;

    std::string myUrl_string = "https://cloud.mackers.dev/remote.php/dav/calendars/" + usr + "/" + calendar_name;
    QString my_qurl = QString::fromStdString(myUrl_string);

    request.setUrl(QUrl(my_qurl));
    request.setRawHeader("Depth", "1");
    request.setRawHeader("Prefer", "return-minimal");
    request.setRawHeader("Content-Type", "application/xml; charset=utf-8");
    std::string cal_type;

    std::string delete_cal = "";

    QByteArray delete_req;
    delete_req.append(delete_cal);

    QNetworkReply *reply = manager->sendCustomRequest(request,"DELETE", delete_req);
    QString response = reply->readAll();
    qDebug() << "[Delete calendar] " << reply;


}

void MainWindow::share_calendar(std::string usr, std::string pwd, std::string calendar_name, std::string mail) {

    QString _usr = QString::fromStdString(usr);
    QString _pwd = QString::fromStdString(pwd);
    QNetworkAccessManager *manager = new QNetworkAccessManager();

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(shareCalendar_slot(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    QNetworkRequest request;

    std::string myUrl_string = "https://cloud.mackers.dev/remote.php/dav/calendars/" + usr + "/" + calendar_name;
    QString my_qurl = QString::fromStdString(myUrl_string);

    request.setUrl(QUrl(my_qurl));
    request.setRawHeader("Depth", "1");
    request.setRawHeader("Prefer", "return-minimal");
    request.setRawHeader("Content-Type", "application/davsharing+xml; charset=utf-8");
    std::string cal_type;

    std::string req_share = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
                            "<D:share-resource xmlns:D=\"DAV:\">\n"
                            "    <D:sharee>\n"
                            "        <D:href>mailto:" + mail + "</D:href>\n"
                                                               "        <D:prop>\n"
                                                               "            <D:displayname>" + calendar_name + "</D:displayname>\n"
                                                                                                               "        </D:prop>\n"
                                                                                                               "        <D:share-access>\n"
                                                                                                               "            <D:read-write />\n"
                                                                                                               "        </D:share-access>\n"
                                                                                                               "    </D:sharee>\n"
                                                                                                               "</D:share-resource>";

    QByteArray request_share;
    request_share.append(req_share);

    QNetworkReply *reply = manager->sendCustomRequest(request,"POST", request_share);
    QString response = reply->readAll();
    qDebug() << "[Sharing calendar] " << reply;
}

/*
 * ******* TIMER *********
*/

void MainWindow::startSynchronization(){

    QNetworkAccessManager *manager = new QNetworkAccessManager();

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handle_synch_reply(QNetworkReply*)));
    connect(manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)), this, SLOT(do_authentication(QNetworkReply *, QAuthenticator *)));

    QNetworkRequest request;

    QString my_qurl = "https://cloud.mackers.dev/remote.php/dav/calendars/" + cal_man.user;

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
}

void MainWindow::handle_synch_reply(QNetworkReply *reply){

    QString replyData = reply -> readAll();
    QString partial_reply;
    qsizetype first_cal = replyData.indexOf("<d:response>");
    replyData.remove(0, first_cal + 12);    // remove header

    auto responses = replyData.split("<d:response>");

    responses.removeLast(); // removing Deck, unused

    QList<QString> cal_to_update;

    for(int i=1; i< responses.size() - 2; i++) {

        partial_reply = responses[i];

        // Getting displayname
        std::string this_reply = partial_reply.toStdString();
        unsigned first = this_reply.find("<d:displayname>");
        unsigned end_first = first + 15;
        unsigned last = this_reply.find("</d:displayname>");
        std::string display_name = this_reply.substr(end_first,last - end_first);

        std::cout << display_name << std::endl;

        // Getting ctag
        first = this_reply.find("<cs:getctag>");
        end_first = first + 12;
        last = this_reply.find("</cs:getctag>");
        std::string c_tag = this_reply.substr(end_first,last - end_first);

        std::cout << c_tag << std::endl;

        // Getting cal_name
        first = this_reply.find("<d:href>/remote.php/dav/calendars/");
        end_first = first + 32;
        last = this_reply.find("/</d:href>");
        std::string cal_slice = this_reply.substr(end_first,last - end_first);

        cal_slice.erase(0,2);
        first = cal_slice.find("/");
        end_first = first + 1;
        last = cal_slice.length();
        std::string cal_name = cal_slice.substr(end_first,last - end_first);

        std::cout << cal_name << std::endl;

        QString old_ctag;

        if(cal_name != "inbox" && cal_name != "outbox" && cal_name != "") {

            // new calendar

            if (cal_man.calendars.find(cal_name) == cal_man.calendars.end()){
                getAllEvents(cal_man.user, cal_man.password, QString::fromStdString(cal_name));
            } else {
                old_ctag = QString::fromStdString(cal_man.calendars[cal_name].ctag);
                if (old_ctag != QString::fromStdString(c_tag)){

                    // showing alerts

                    ui->cloud_changes->show();
                    ui->cloud_changes_2->show();
                    ui->cloud_changes_3->show();
                    ui->cloud_changes_4->show();
                    ui->cloud_changes_5->show();
                    ui->cloud_changes_6->show();
                    ui->cloud_changes_7->show();
                    ui->cloud_changes_8->show();
                    ui->cloud_changes_9->show();
                    ui->cloud_changes_10->show();

                    clear_selected_todo(display_name);
                    clear_selected_cal(display_name);
                    cal_man.calendars[cal_name].eraseContent(); // Erase content from old calendar
                    cal_man.calendars[cal_name].ctag = c_tag;
                    cal_to_update.append(QString::fromStdString(cal_name));

                }
            }
        }
    }

    for (auto cal : cal_to_update){
        getAllEvents(cal_man.user, cal_man.password, cal);
    }

    // Restart timer

    ui->cloud_changes->hide();
    synch_timer->start();
}

void MainWindow::clear_selected_todo(std::string display_name){

    ui->vtodo_list->removeItem(ui->vtodo_list->findText(QString::fromStdString(display_name), Qt::MatchExactly));
    QList<QTreeWidgetItem*> clist = ui->TODO_list->findItems(QString::fromStdString(display_name), Qt::MatchContains, 2);
    foreach(QTreeWidgetItem* item, clist)
    {
        qDebug() << item->text(2);
        delete item;
    }

}

void MainWindow::clear_selected_cal(std::string display_name){

    ui->vevent_list->removeItem(ui->vevent_list->findText(QString::fromStdString(display_name), Qt::MatchExactly));
    QList<QTreeWidgetItem*> clist = ui->cal_list->findItems(QString::fromStdString(display_name), Qt::MatchContains, 0);
    foreach(QTreeWidgetItem* item, clist)
    {
        qDebug() << item->text(2);
        delete item;
    }

    /* ui->cal_list->clear();
    // show them all again
    for (auto calendar : cal_man.calendars){
        Calendar cal = calendar.second;
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, QString::fromStdString(cal.name));
        if (cal.is_todo){
            item->setText(1, QString::fromStdString("Tasks"));
        }
        else {
            item->setText(1, QString::fromStdString("Calendar"));
        }
        if (cal.is_shown){
            item->setText(2, QString::fromStdString("Show"));
        }
        else{
            item->setText(2, QString::fromStdString("Hide"));
        }
    }*/

}

void MainWindow::refresh_todos() {

    ui->TODO_list->clear(); // clear todos shown

    for (auto cal : cal_man.calendars){
        if (cal.second.is_todo){
            for (auto task : cal.second.todos){
                Todo todo = task.second;
                QTreeWidgetItem *newItem = new QTreeWidgetItem();
                newItem->setText(0,todo.summary);
                newItem->setText(1,todo.due_to.toString("yyyy-MM-dd"));
                newItem->setText(2,QString::fromStdString(cal.second.display_name));
                newItem->setText(4,todo.UID);
                if (todo.completed){
                    newItem->setText(3, "Completed");
                }
                else {
                    newItem->setText(3, "Due");
                }
                ui->TODO_list->addTopLevelItem(newItem);
            }
        }
    }

}

/*
 * ******* GUI/BUTTONS *********
*/

void MainWindow::on_loginButton_clicked() {
    
    QString login_user = ui->username_login->text();
    QString password = ui->password_login->text();
    ui->loading_start->show();
    ui->login_error->hide();
    
    login(login_user.toStdString(), password.toStdString());
}

void MainWindow::on_createEventButton_clicked() {
    
    QString user = ui->username_login->text();
    QString summary = ui-> event_title -> toPlainText();
    QDateTime startDateTime = ui -> start_date_time -> dateTime();
    QDateTime endDateTime = ui -> end_date_time -> dateTime();
    QString cal_name;
    QString cal_chosen = ui->vevent_list->currentText();
    
    for (auto cal : cal_man.calendars){
        if(cal.second.display_name == cal_chosen.toStdString()) {
            cal_name = QString::fromStdString(cal.second.name);
        }
    }
    
    if (summary.isEmpty() || startDateTime > endDateTime || cal_name.isEmpty()) {
        ui -> success_create_event -> hide();
        ui -> error_create_event -> show();
    } else {
        createEvent(user, cal_name, summary, startDateTime, endDateTime);
        ui -> success_create_event -> show();
        ui -> error_create_event -> hide();
    }
    
}

void MainWindow::on_createTodoButton_clicked()
{
    
    QString user = ui->username_login->text();
    QString summary = ui->todo_summary->toPlainText();
    QDateTime dueDate = ui->todo_due->dateTime();
    QString cal_name;
    QString cal_chosen = ui->vtodo_list->currentText();
    
    for (auto cal : cal_man.calendars){
        if(cal.second.display_name == cal_chosen.toStdString()) {
            cal_name = QString::fromStdString(cal.second.name);
        }
    }
    
    if(summary.isEmpty() || cal_name.isEmpty()) {
        ui->error_TODO_create->show();
    } else {
        createTODO(user, cal_name, summary, dueDate);
        ui->error_TODO_create->hide();
    }
    
}

void MainWindow::on_displayedCalendar_clicked(const QDate &date)
{
    showEventsOnDate(date);
}

void MainWindow::on_editButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    ui->editButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
    
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
    s.erase(s.length() - 1); // Erase \n
    QString uid = QString::fromStdString(s);
    QString cal_name = "";
    QString display_name = "";
    
    for (auto cal : cal_man.calendars){
        Calendar c = cal.second;
        if (c.events.find(s) != c.events.end()){
            cal_name = QString::fromStdString(c.name);
            display_name = QString::fromStdString(c.display_name);
        }
    }
    
    if (summary.isEmpty() || start_date_time > end_date_time) {
        ui -> successEdit -> hide();
        ui -> errorEdit -> show();
    } else {
        editEvent(user, uid, cal_name, summary, start_date_time, end_date_time);
        showEventsOnDate(ui->displayedCalendar->selectedDate());
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
    ui->editButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
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
    QString cal_name = "";
    QString display_name = "";
    
    for (auto cal : cal_man.calendars){
        Calendar c = cal.second;
        if (c.events.find(s) != c.events.end()){
            cal_name = QString::fromStdString(c.name);
            display_name = QString::fromStdString(c.display_name);
        }
    }
    
    deleteEvent(user, password, cal_name, uid);
    showEventsOnDate(ui->displayedCalendar->selectedDate());
    ui -> successDelete -> show();
    ui -> errorDelete -> hide();
    
}

void MainWindow::on_listOfEvents_itemClicked(QListWidgetItem *item)
{
    QString event_data = item->text();
    QString summary = event_data.section("\n", 0, 0);
    QString start = event_data.section("\n", 1, 1);
    QString end = event_data.section("\n", 2, 2);
    
    qsizetype pos = 0;
    start.remove(pos, 7);
    end.remove(pos, 5);
    
    ui->editButton->setEnabled(true);
    ui->deleteButton->setEnabled(true);
    ui->titleEdit->setText(summary);
    ui->startDateTimeEdit->setDateTime(QDateTime::fromString(start, "dd.MM.yyyy hh:mm"));
    ui->endDateTimeEdit->setDateTime(QDateTime::fromString(end, "dd.MM.yyyy hh:mm"));
    
}

void MainWindow::on_TODO_list_itemClicked(QTreeWidgetItem *item, int column)
{
    
    cal_man.selected_todo.summary = item->text(0);
    cal_man.selected_todo.due_to = QDateTime::fromString(item->text(1),"yyyy-MM-dd");
    cal_man.selected_cal_name = item->text(2);
    cal_man.selected_todo.UID = item->text(4);
    cal_man.selected_todo.completed = (item->text(3) != "Due");
    ui->textTODOedit->setText(item->text(0));
    auto s = item->text(1).toStdString();
    auto date = QDate::fromString(QString::fromStdString(s),"yyyy-MM-dd");
    
    ui->dateTODOedit->setDate(date);
    ui->checkCompleted->setChecked(cal_man.selected_todo.completed);
    ui->editTodoButton->setEnabled(true);
    ui->deleteTodoButton->setEnabled(true);
    ui->error_TODO_edit->hide();
    ui->success_TODO_edit->hide();
}

void MainWindow::on_deleteTodoButton_clicked()
{
    
    deleteEvent(ui->username_login->text(), ui->username_login->text(), cal_man.selected_cal_name, cal_man.selected_todo.UID);
    cal_man.calendars[cal_man.selected_cal_name.toStdString()].todos.erase(cal_man.selected_todo.UID.toStdString());
    ui->editTodoButton->setEnabled(false);
    ui->deleteTodoButton->setEnabled(false);
    delete ui->TODO_list->currentItem();
    
}

void MainWindow::on_editTodoButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(4);
    ui->checkCompleted->setChecked(false);
}

void MainWindow::on_backTODOedit_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_backSAVEedit_clicked()
{
    
    QString cal_name;
    QString display_name;
    
    QString myTitle = ui->textTODOedit->toPlainText();
    
    for (auto cal : cal_man.calendars){
        if(cal.second.display_name == cal_man.selected_cal_name.toStdString()) {
            cal_name = QString::fromStdString(cal.second.name);
            display_name = QString::fromStdString(cal.second.display_name);
        }
    }
    
    if(myTitle.isEmpty() || cal_name.isEmpty()) {
        ui->error_TODO_edit->show();
        ui->success_TODO_edit->hide();
    } else {
        editTODO(ui->username_login->text(), cal_name, ui->textTODOedit->toPlainText(),
                 ui->dateTODOedit->dateTime(), cal_man.selected_todo.UID, ui->checkCompleted->isChecked());
        
        // Replacing task with new version

        delete ui->TODO_list->currentItem();
        QTreeWidgetItem *newItem = new QTreeWidgetItem();
        newItem->setText(0,ui->textTODOedit->toPlainText());
        newItem->setText(1,ui->dateTODOedit->dateTime().toString("yyyy-MM-dd"));
        newItem->setText(2,display_name);
        newItem->setText(4,cal_man.selected_todo.UID);
        if(ui->checkCompleted->isChecked()){
            newItem->setText(3, "Completed");
        }
        else{
            newItem->setText(3, "Due");
        }
        ui->TODO_list->addTopLevelItem(newItem);
        
        ui->success_TODO_edit->show();
        ui->error_TODO_edit->hide();
        
    }
    
    
}

void MainWindow::on_new_calendar_btn_clicked()
{
    ui->stackedWidget->setCurrentIndex(5);
}

void MainWindow::on_create_cal_goback_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->create_cal_success->hide();
    ui->create_cal_error->hide();
}

void MainWindow::on_create_cal_go_clicked()
{
    QString myTitle = ui->create_cal_name->toPlainText();
    if(myTitle.isEmpty()) {
        ui->create_cal_error->show();
        ui->create_cal_success->hide();
    } else {
        
        cal_man.is_new = true;
        create_calendar(ui->username_login->text().toStdString(), ui->password_login->text().toStdString(), ui->create_cal_name->toPlainText().toStdString());
        ui->create_cal_success->show();
        ui->create_cal_error->hide();
        
    }
    
}

void MainWindow::on_cal_list_itemClicked(QTreeWidgetItem *item, int column)
{
    ui->delete_calendar_btn->setEnabled(true);
    ui->share_calendar_btn->setEnabled(true);
    cal_man.selected_cal.name = item->text(0).toStdString();
    cal_man.selected_cal.display_name = item->text(0).toStdString();
    
    std::string curTyp = item->text(1).toStdString();
    
    if (curTyp == "Calendar") {
        cal_man.selected_cal.is_todo = false;
    } else {
        cal_man.selected_cal.is_todo = true;
    }
}

void MainWindow::on_delete_calendar_btn_clicked()
{
    
    QString cal_name;
    
    for (auto cal : cal_man.calendars){
        if(cal.second.display_name == cal_man.selected_cal.display_name) {
            cal_name = QString::fromStdString(cal.second.name);
        }
    }
    
    delete_calendar(ui->username_login->text().toStdString(), ui->password_login->text().toStdString(), cal_name.toStdString());
}

void MainWindow::on_share_calendar_btn_clicked()
{
    ui->stackedWidget->setCurrentIndex(6);
}

void MainWindow::on_share_cal_goback_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->share_cal_success->hide();
    ui->share_cal_error->hide();
}

void MainWindow::on_share_cal_go_clicked()
{
    
    QString myTitle = ui->share_cal_name->toPlainText();
    if(myTitle.isEmpty()) {
        ui->share_cal_error->show();
        ui->share_cal_success->hide();
    } else {
        share_calendar(ui->username_login->text().toStdString(), ui->password_login->text().toStdString(), cal_man.selected_cal.name, ui->share_cal_name->toPlainText().toStdString());
        ui->share_cal_success->show();
        ui->share_cal_error->hide();
        
    }
}

void MainWindow::on_newEventShortcut_clicked()
{
    ui->tabWidget->setCurrentIndex(2);
}


void MainWindow::on_newCalendarShortcut_clicked()
{
    ui->stackedWidget->setCurrentIndex(5);
}
