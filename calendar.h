#ifndef CALENDAR_H
#define CALENDAR_H

#include <QObject>

#include "event.h"
#include "todo.h"

class Calendar : public QObject
{
    Q_OBJECT
public:
    explicit Calendar(QObject *parent = nullptr);
    Calendar(const Calendar &other);

    Calendar& operator=(const Calendar& other);


    std::string name;
    std::string display_name;
    std::string color;
    std::string ctag;
    bool is_shown; // show calendar in GUI
    bool is_todo; // flag calendar as task list
    std::map<std::string, Event> events;
    std::map<std::string, Todo> todos;

signals:

};

#endif // CALENDAR_H
