#ifndef TODO_H
#define TODO_H

#include <QObject>
#include <QDateTime>
#include <QString>

class Todo : public QObject
{
    Q_OBJECT
public:
    explicit Todo(QObject *parent = nullptr);
    Todo(const Todo &other);

    Todo& operator=(const Todo& other);

    QString UID;
    QString summary;
    QDateTime due_to;
    QDateTime creation_date;

    const QString toString();

signals:


};

#endif // TODO_H
