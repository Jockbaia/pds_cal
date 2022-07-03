#include "calendar_manager.h"

CalendarManager::CalendarManager(QObject *parent)
    : QObject{parent}
{

}

CalendarManager::CalendarManager(const CalendarManager &other){
    synch_timer.setInterval(other.synch_timer.interval());
    synch_timer.setSingleShot(other.synch_timer.isSingleShot());
    user = other.user;
    password = other.password;

    calendars = other.calendars;
    selected_todo = other.selected_todo;
    selected_cal_name = other.selected_cal_name;


}
