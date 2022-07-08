#ifndef CALENDAR_MANAGER_H
#define CALENDAR_MANAGER_H

#include <QObject>
#include <QTimer>
#include "calendar.h"

class CalendarManager : public QObject
{
    Q_OBJECT
public:
    explicit CalendarManager(QObject *parent = nullptr);
    CalendarManager(const CalendarManager& other);

    // AUTH

    QString user;
    QString password;

    // Selectors

    std::map<std::string, Calendar> calendars;
    Todo selected_todo;
    Calendar selected_cal;
    QString selected_cal_name;
    Event event_creation;
    Todo todo_creation;

    // Sync (new calendar from server)

    std::string sync_name;
    bool is_new_sync;

    // various flags

    bool is_new;
    bool is_logged;



signals:

};

#endif // CALENDAR_MANAGER_H
