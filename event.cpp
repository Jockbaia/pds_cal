#include "event.h"

Event::Event(QObject *parent)
    : QObject{parent}
{

}

Event::Event(const Event &other){
    UID = other.UID;
    summary = other.summary;
    timestamp_start = other.timestamp_start;
    timestamp_end = other.timestamp_end;
    creation_date = other.creation_date;
}

Event& Event::operator=(const Event &other){
    UID = other.UID;
    summary = other.summary;
    timestamp_start = other.timestamp_start;
    timestamp_end = other.timestamp_end;
    creation_date = other.creation_date;

    return *this;
}

const QString Event::toString(){
    QString res;
    res = res + summary + "\n";
    res = res + "START: " + timestamp_start.toString("dd.MM.yyyy hh:mm") + "\n";
    res = res + "END: " + timestamp_end.toString("dd.MM.yyyy hh:mm") + "\n";
    res = res + "UID: " + UID + "\n";
    return res;
}
