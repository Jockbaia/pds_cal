#ifndef PDS_CAL_H
#define PDS_CAL_H

#include <QMainWindow>
#include <QtNetwork/QNetworkAccessManager>

QT_BEGIN_NAMESPACE
namespace Ui { class pds_cal; }
QT_END_NAMESPACE

class QNetworkAccessManager;

class pds_cal : public QMainWindow
{
    Q_OBJECT

public:
    pds_cal(QWidget *parent = nullptr);
    ~pds_cal();

private slots:
    void on_getButton_clicked();

private:
    Ui::pds_cal *ui;
    QNetworkAccessManager *mManager;
};
#endif // PDS_CAL_H
