#include "calendar_manager.h"

CalendarManager::CalendarManager(QObject *parent)
    : QObject{parent}
{

}

CalendarManager::CalendarManager(const CalendarManager &other){
    user = other.user;
    password = other.password;

    calendars = other.calendars;
    selected_cal = other.selected_cal;
    selected_cal_name = other.selected_cal_name;
    is_new = other.is_new;
    is_logged = other.is_logged;
    is_new_sync = other.is_new_sync;
    sync_name = other.sync_name;
    event_creation = other.event_creation;
    todo_creation = other.todo_creation;
}
