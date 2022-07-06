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

    QString user;
    QString password;

    std::map<std::string, Calendar> calendars;
    Todo selected_todo;
    Calendar selected_cal;
    QString selected_cal_name;
    bool is_new;


signals:

};

#endif // CALENDAR_MANAGER_H
