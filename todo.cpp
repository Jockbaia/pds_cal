#include "todo.h"

Todo::Todo(QObject *parent)
    : QObject{parent}
{

}

Todo::Todo(const Todo &other){
    UID = other.UID;
    summary = other.summary;
    creation_date = other.creation_date;
    due_to = other.due_to;
}

Todo& Todo::operator=(const Todo &other){
    UID = other.UID;
    summary = other.summary;
    creation_date = other.creation_date;
    due_to = other.due_to;

    return *this;
}

const QString Todo::toString(){
    QString res;
    res = res + due_to.date().toString("dd.MM.yyyy") + "\t" + summary;
    return res;
}
