#include "pds_cal.h"
#include "./ui_pds_cal.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QDebug>
#include <QBuffer>
#include <QDateTime>
#include <QTimer>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::pds_cal)
{
    ui->setupUi(this);
    mManager = new QNetworkAccessManager(this);
    connect(mManager, &QNetworkAccessManager::finished, this, [&](QNetworkReply *reply){
        QByteArray data = reply->readAll();
        QString str = QString::fromLatin1(data);
        ui->plainTextEdit->setPlainText(str);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_getButton_clicked()
{
    // mManager->get(QNetworkRequest(QUrl(ui->urlLineEdit->text())));

    // AUTHENTICATION

    QString username = ui->username_login->text();
    QString pass = ui->password_login->text();

    QString concatenated = username + ":" + pass; // username:password
    QByteArray data = concatenated.toLocal8Bit().toBase64();

    /* QString headerData = "Basic " + data;
    QNetworkRequest request=QNetworkRequest(QUrl(ui->urlLineEdit->text()));
    request.setRawHeader("Authorization", headerData.toLocal8Bit());
    mManager->get(request); */

    qDebug() << "saving event: TEST";

    QString authorization = "Basic ";
    authorization.append(data);

    QBuffer* buffer = new QBuffer();

    buffer->open(QIODevice::ReadWrite);

    // startDateTime creato manualmente ma veniva passato come parametro
    QDate start_date(2022, 05, 21);
    QTime start_time(14, 30);
    QDateTime startDateTime(start_date, start_time);    //Local Time, non ho trovato un modo facile per scegliere il fuso orario
    QString uid = QDateTime::currentDateTime().toString("yyyyMMdd-HHMM-00ss") + "-0000-" + startDateTime.toString("yyyyMMddHHMM");

    QString filename = uid + ".ics";    // portato fuori dall'if commmentato

    // Questo credo andrà reinserito dopo
    /*if (filename.isEmpty())
    {
      filename = uid + ".ics";
    }*/

    //Anche questi erano passati come parametri
    QString summary = "Climbing";
    QTime end_time(18,30);
    QDateTime endDateTime(start_date, end_time);
    QString location = "Espoo";
    QString description = "It's in the summary :)";
    QString requestString = "BEGIN:VCALENDAR\r\n"
                            "BEGIN:VEVENT\r\n"
                            "UID:" + uid + "\r\n"
                            "VERSION:2.0\r\n"
                            "DTSTAMP:" + QDateTime::currentDateTime().toString("yyyyMMddTHHmmssZ") + "\r\n"
                            "SUMMARY:" + summary + "\r\n"
                            "DTSTART:" + startDateTime.toString("yyyyMMddTHHmmss") + "\r\n"
                            "DTEND:" + endDateTime.toString("yyyyMMddTHHmmss") + "\r\n"
                            "LOCATION:" + location + "\r\n"
                            "DESCRIPTION:" + description + "\r\n"
                            "TRANSP:OPAQUE\r\n";

    // Questa è roba opzionale che credo serva per gli eventi ripetuti
    /*if (!rrule.isEmpty())
    {
      requestString.append("RRULE:" + rrule + "\r\n");
    }

    if (!exdate.isEmpty())
    {
      requestString.append("EXDATE:" + exdate + "\r\n");
    }*/

    requestString.append("END:VEVENT\r\nEND:VCALENDAR");

    int buffersize = buffer->write(requestString.toUtf8());
    buffer->seek(0);
    buffer->size();

    // L'originale creava QByteArray vuoto e gli appendeva la stringa ma non funziona nella versione 6.3, quindi questa conversione potrebbe essere un problema
    QByteArray contentlength = QString::number(buffersize).toLocal8Bit();
    // contentlength.append(QString::number(buffersize));

    QNetworkRequest request;
    // Per l'url, la stringa dovrebbe essere host name + filename, con host name definito come attributo di classe
    // Potrei aver scazzato l'host name, non ne sono sicura
    request.setUrl(QUrl("https://cloud.mackers.dev/remote.php/dav/calendars/progetto-pds/test/" + filename));
    request.setRawHeader("User-Agent", "CalendarClient_CalDAV"); // ?
    request.setRawHeader("Authorization", authorization.toUtf8());
    request.setRawHeader("Depth", "0");
    request.setRawHeader("Prefer", "return-minimal");
    request.setRawHeader("Content-Type", "text/calendar; charset=utf-8");
    request.setRawHeader("Content-Length", contentlength); // qui usa il QByteArray di cui non sono sicura


    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    QNetworkAccessManager m_UploadNetworkManager;

    QNetworkReply* m_pUploadReply = m_UploadNetworkManager.put(request, buffer);

    if (NULL != m_pUploadReply)
    {
      connect(m_pUploadReply, SIGNAL(error(QNetworkReply::NetworkError)),
              this, SLOT(handleUploadHTTPError()));

      connect(m_pUploadReply, SIGNAL(finished()),
              this, SLOT(handleUploadFinished()));

      QTimer m_UploadRequestTimeoutTimer;
      m_UploadRequestTimeoutTimer.start(2000);
    }
    else
    {
      qDebug() << ": " << "ERROR: Invalid reply pointer when requesting URL.";
      // prossima riga commentata perché servirebbe copiare altra roba per farla funzionare (credo)
      // emit error("Invalid reply pointer when requesting URL.");
    }
}

void handleUploadHTTPError(void)
{
      qDebug() << "";
}

void handleUploadFinished(void)
{
      qDebug() << "";
}



