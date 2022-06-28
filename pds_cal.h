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

    void traduce(QString data);
    void on_getButton_clicked();
    void on_loginButton_clicked();
    void on_createEventButton_clicked();
    bool login(std::string usr, std::string pwd);
    void login_slot(QNetworkReply* reply);
    void do_authentication(QNetworkReply *, QAuthenticator *q);

    void getAllEvents(QString user, QString pass, QString calendar_name);
    void report_getAllEvents(QNetworkReply* reply);
    void createEvent(QString user, QString calendar_name, QString summary, QDate start_date, QTime start_time, QTime end_time);
    void report_createEvent(QNetworkReply* reply, QString UID, QString calendar_name, QString summary, QDate start_date, QTime start_time, QTime end_time);
    void deleteEvent(QString user, QString pass, QString calendar_name, QString uid);
    void report_function(QNetworkReply* reply);
protected slots:


private:
    Ui::pds_cal *ui;
    QNetworkAccessManager *mManager;
};
#endif // PDS_CAL_H
