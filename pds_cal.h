#ifndef PDS_CAL_H
#define PDS_CAL_H

#include <QMainWindow>
#include <QtNetwork/QNetworkAccessManager>

QT_BEGIN_NAMESPACE
namespace Ui { class pds_cal; }
QT_END_NAMESPACE

class QNetworkAccessManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_getButton_clicked();
    void login(std::string usr, std::string pwd);
    void login_slot(QNetworkReply* reply);
    void do_authentication(QNetworkReply *, QAuthenticator *q);
    void saveNewEvent();
    void getAllEvents();
    void deleteEvent();
    void report_function(QNetworkReply* reply);
protected slots:


private:
    Ui::pds_cal *ui;
    QNetworkAccessManager *mManager;
};
#endif // PDS_CAL_H
