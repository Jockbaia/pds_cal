#ifndef PDS_CAL_H
#define PDS_CAL_H

#include <QMainWindow>
#include <QListWidget>
#include <QTreeWidgetItem>
#include <QtNetwork/QNetworkAccessManager>
#include <QTimer>
#include "calendar_manager.h"
#include "event.h"
#include "todo.h"
#include "calendar.h"

QT_BEGIN_NAMESPACE
namespace Ui { class pds_cal; }
QT_END_NAMESPACE

class QNetworkAccessManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:

    // parsing

    void parse_vcalendar(QString data);
    void parse_request(QString data);

    // auth

    bool login(std::string usr, std::string pwd);
    bool get_calendars(std::string usr, std::string pwd);

    // qReplys

    void do_authentication(QNetworkReply *, QAuthenticator *q);
    void login_slot(QNetworkReply* reply);
    void getCalendars_slot(QNetworkReply* reply);
    void report_function(QNetworkReply* reply);
    void report_getAllEvents(QNetworkReply* reply);
    void createCalendar_slot(QNetworkReply* reply);
    void deleteCalendar_slot(QNetworkReply* reply);
    void shareCalendar_slot(QNetworkReply* reply);

    // events

    void getAllEvents(QString user, QString pass, QString calendar_name);
    void createEvent(QString user, QString calendar_name, QString summary, QDateTime start_date_time, QDateTime end_date_time);
    void editEvent(QString user, QString uid, QString calendar_name, QString summary, QDateTime start_date_time, QDateTime end_date_time);
    void deleteEvent(QString user, QString pass, QString calendar_name, QString uid);

    // todos

    void createTODO(QString user, QString calendar_name, QString summary, QDateTime end_date);
    void deleteTODO(QString user, QString pass, QString calendar_name, QString uid);
    void editTODO(QString user, QString calendar_name, QString summary, QDateTime new_due, QString uid, bool comp);

    // calendar

    void create_calendar(std::string usr, std::string pwd, std::string calendar_name);
    void delete_calendar(std::string usr, std::string pwd, std::string calendar_name);
    void share_calendar(std::string usr, std::string pwd, std::string calendar_name, std::string mail);

    QList<Event> getEventsOnDate(QDate date);
    void showEventsOnDate(QDate date);
    QString eventsListToString(QList<Event>);

    // timer

    void startSynchronization();
    void handle_synch_reply(QNetworkReply*);

    // clicking buttons

    void on_getButton_clicked();
    void on_loginButton_clicked();
    void on_createEventButton_clicked();
    void on_createTodoButton_clicked();
    void on_deleteTodoButton_clicked();
    void on_editButton_clicked();
    void on_closeEditButton_clicked();
    void on_confirmEditButton_clicked();
    void on_cancelEditButton_clicked();
    void on_deleteButton_clicked();
    void on_goBackButton_clicked();
    void on_confirmDelete_clicked();
    void on_new_calendar_btn_clicked();
    void on_create_cal_goback_clicked();
    void on_create_cal_go_clicked();
    void on_cal_list_itemClicked(QTreeWidgetItem *item, int column);
    void on_delete_calendar_btn_clicked();
    void on_displayedCalendar_clicked(const QDate &date);
    void on_listOfEvents_itemClicked(QListWidgetItem *item);
    void on_TODO_list_itemClicked(QTreeWidgetItem *item, int column);
    void on_editTodoButton_clicked();
    void on_backTODOedit_clicked();
    void on_backSAVEedit_clicked();
    void on_share_calendar_btn_clicked();
    void on_share_cal_goback_clicked();
    void on_share_cal_go_clicked();
    void on_newEventShortcut_clicked();
    void on_newCalendarShortcut_clicked();

protected slots:
    void clear_and_show_todos_again();
    void clear_and_show_calendars_again();

private:
    Ui::pds_cal *ui;
    QNetworkAccessManager *mManager;
    CalendarManager cal_man;
    QTimer* synch_timer;
};

#endif // PDS_CAL_H
