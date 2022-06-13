#ifndef CALENDAR_MANAGER_H
#define CALENDAR_MANAGER_H

#include <QObject>

class Calendar_Manager : public QObject
{
    Q_OBJECT
public:
    explicit Calendar_Manager(QObject *parent = nullptr);

signals:

};

#endif // CALENDAR_MANAGER_H
