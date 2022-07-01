#include "calendar.h"

Calendar::Calendar(QObject *parent)
    : QObject{parent}
{

}

Calendar::Calendar(const Calendar &other){
    name = other.name;
    color = other.color;
    ctag = other.ctag;
    display_name = other.display_name;
    is_shown = other.is_shown;
    is_todo = other.is_todo;

    for (auto x : other.events){
        events[x.first] = x.second;
    }
    for (auto x : other.todos){
        todos[x.first] = x.second;
    }
}

Calendar& Calendar::operator=(const Calendar& other){
    name = other.name;
    color = other.color;
    ctag = other.ctag;
    display_name = other.display_name;
    is_shown = other.is_shown;
    is_todo = other.is_todo;

    for (auto x : other.events){
        events[x.first] = x.second;
    }
    for (auto x : other.todos){
        todos[x.first] = x.second;
    }
    return *this;
}
