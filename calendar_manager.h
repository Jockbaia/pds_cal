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

    QTimer synch_timer;
    QString user;
    QString password;

    std::map<std::string, Calendar> calendars;
    Todo selected_todo;
    QString selected_cal_name;


signals:

};

#endif // CALENDAR_MANAGER_H
