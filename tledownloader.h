#ifndef TLEDOWNLOADER_H
#define TLEDOWNLOADER_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QProcess>
#include <QtDebug>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QFile>
#include <QSqlQuery>

namespace Ui {
class TLEDownloader;
}

class TLEDownloader : public QMainWindow
{
    Q_OBJECT

public:
    explicit TLEDownloader(QWidget *parent = 0);
    ~TLEDownloader();

public slots:
    void replyFinished(QNetworkReply *reply);
    void getTLEData(int tleType); //0 = 2line full, 1 = 2line LEO
    int receiveTLEData(QString tleData);
    int insertTLE(QStringList tleData);
    void selectDATA();

private:
    int tleType;
    QStringList results;

    QNetworkAccessManager *manager;
    QProcess *processCDM;

    QVector<QStringList> listCDMAll;
    QVector<QString> lineCDMAll;

    QSqlDatabase db;

    Ui::TLEDownloader *ui;
};

#endif // TLEDOWNLOADER_H
