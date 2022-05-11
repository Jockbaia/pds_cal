#include "pds_cal.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    pds_cal w;

    w.show();
    return a.exec();
}
