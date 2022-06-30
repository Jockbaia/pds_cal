#ifndef EVENT_H
#define EVENT_H

#include <QObject>
#include <QDateTime>
#include <QString>

class Event : public QObject
{
    Q_OBJECT
public:
    explicit Event(QObject *parent = nullptr);
    Event(const Event &other);

    Event& operator=(const Event& other);

    QString UID;
    QString summary;
    QDateTime timestamp_start;
    QDateTime timestamp_end;
    QDateTime creation_date;

    const QString toString();

signals:


};

#endif // EVENT_H
