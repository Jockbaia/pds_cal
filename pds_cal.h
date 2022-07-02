#ifndef PDS_CAL_H
#define PDS_CAL_H

#include <QMainWindow>
#include <QListWidget>
#include <QTreeWidgetItem>
#include <QtNetwork/QNetworkAccessManager>
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

    // auth

    bool login(std::string usr, std::string pwd);
    bool get_calendars(std::string usr, std::string pwd);

    // qReplys

    void do_authentication(QNetworkReply *, QAuthenticator *q);
    void login_slot(QNetworkReply* reply);
    void getCalendars_slot(QNetworkReply* reply);
    void report_function(QNetworkReply* reply);
    void report_getAllEvents(QNetworkReply* reply);

    // events

    void getAllEvents(QString user, QString pass, QString calendar_name);
    void createEvent(QString user, QString calendar_name, QString summary, QDateTime start_date_time, QDateTime end_date_time);
    void editEvent(QString user, QString uid, QString calendar_name, QString summary, QDateTime start_date_time, QDateTime end_date_time);
    void deleteEvent(QString user, QString pass, QString calendar_name, QString uid);

    // todos

    void createTODO(QString user, QString calendar_name, QString summary, QDateTime end_date);
    void deleteTODO(QString user, QString pass, QString calendar_name, QString uid);
    void editTODO(QString user, QString calendar_name, QString summary, QDateTime new_due, QString uid);

    QList<Event> getEventsOnDate(QDate date);
    void showEventsOnDate(QDate date);
    QString eventsListToString(QList<Event>);
    void on_displayedCalendar_clicked(const QDate &date);
    void on_listOfEvents_itemClicked(QListWidgetItem *item);

    void on_TODO_list_itemClicked(QTreeWidgetItem *item, int column);

    void on_editTodoButton_clicked();

    void on_backTODOedit_clicked();

    void on_backSAVEedit_clicked();

protected slots:


private:
    Ui::pds_cal *ui;
    QNetworkAccessManager *mManager;
};
#endif // PDS_CAL_H
