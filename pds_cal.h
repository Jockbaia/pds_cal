#ifndef PDS_CAL_H
#define PDS_CAL_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class pds_cal; }
QT_END_NAMESPACE

class pds_cal : public QMainWindow
{
    Q_OBJECT

public:
    pds_cal(QWidget *parent = nullptr);
    ~pds_cal();

private:
    Ui::pds_cal *ui;
};
#endif // PDS_CAL_H
