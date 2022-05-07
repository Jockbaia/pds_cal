#include "pds_cal.h"
#include "./ui_pds_cal.h"

pds_cal::pds_cal(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::pds_cal)
{
    ui->setupUi(this);
}

pds_cal::~pds_cal()
{
    delete ui;
}

