#include "tledownloader.h"
#include "ui_tledownloader.h"
#include <iomanip>

TLEDownloader::TLEDownloader(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TLEDownloader)
{
    ui->setupUi(this);

    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setPort(3306);
     this->processType = 3; // set process 1 one table/2 seperate table/3 day by day table

    if(this->processType == 1) {db.setDatabaseName("space_object"); qDebug() << "1 table";}
    else if(this->processType == 2){ db.setDatabaseName("separate_space_object"); qDebug() << "multiple table";}
    else if(this->processType == 3){ db.setDatabaseName("dbd_space_object"); qDebug() << "day by day table";}

    db.setUserName("root");
    db.setPassword("root");
    bool ok = db.open();
    qDebug() << ok;

//    this->selectDATA();
//    //-- test zone --
//    QStringList test;
//    test << "899"<< "U"<< ""<< ""<< ""<< "18"<< "303.97492016"<< "+.00001758"<< "+00000-0"<< "+13822-3"<< "0"<< "999"<< "1"<< "89453"<< "098.3397"<< "179.0388"<< "0157137"<< "319.7782"<< "039.1894"<< "14.92904369"<< "11793"<< "8";
//    this->insertTLE(test);
//    //-- end test zone

     this->getTLEData(0); //full = 0; LEO = 1

//      QTimer *timer = new QTimer(this);
//      connect(timer, SIGNAL(timeout()), this, SLOT(test_timer()));
//      timer->start(1800000);
}

TLEDownloader::~TLEDownloader()
{
    delete ui;
}

void TLEDownloader::replyFinished(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString statusName = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    qDebug() << "Status code " << statusCode << statusName;

    QByteArray bytes = reply->readAll();
    QString info = QString::fromUtf8(bytes.data(), bytes.size());

    if(statusCode == 200)
    {
        QStringList text = info.split("\r\n", QString::SkipEmptyParts);
        QDateTime start =  QDateTime::currentDateTimeUtc();
        qDebug() << "start" << start;
        for(int i = 0; i < text.size(); i++){
            //get line1 and line2 and send to getTLEData function
            this->receiveTLEData(text[i]);
        }

        //db.close();
        QDateTime end =  QDateTime::currentDateTimeUtc();
        qDebug() << "end" << end;
        this->save_time_process(start,end,this->processType); // save time

        QString filename="D:/tle_full.txt";
        if(this->tleType == 0){
            filename = "C:/FDS_Operation/thaichotelive/TLE/tle_full_"+QDate::currentDate().toString("yyyyMMdd")+".txt";
        }else if(this->tleType == 1){
            filename = "C:/FDS_Operation/thaichotelive/TLE/tle_LEO_"+QDate::currentDate().toString("yyyyMMdd")+".txt";
        }
        QFile file( filename );
        if ( file.open(QIODevice::ReadWrite) )
        {
            QTextStream stream( &file );
            for(int i = 0; i < text.size(); i++)
            {
                stream << text[i] << endl;
            }
            file.close();
        }
    }
    else
    {
        qDebug() << reply->error() << reply->errorString();
    }

    reply->deleteLater();
}

void TLEDownloader::getTLEData(int tleType)
{
    this->tleType = tleType;

    QUrl url("https://www.space-track.org/ajaxauth/login");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QString query;
    if(tleType == 0){
        query = "https://www.space-track.org/basicspacedata/query/class/tle_latest/ORDINAL/1/EPOCH/%3Enow-30/orderby/NORAD_CAT_ID/format/tle";
    }else if(tleType == 1){
        query = "https://www.space-track.org/basicspacedata/query/class/tle_latest/ORDINAL/1/EPOCH/%3Enow-30/MEAN_MOTION/%3E11.25/ECCENTRICITY/%3C0.25/OBJECT_TYPE/payload/orderby/NORAD_CAT_ID/format/tle";
    }

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("identity", "osa@gistda.or.th");
    urlQuery.addQueryItem("password", "Gistda1234567890");
    urlQuery.addQueryItem("query", query);
    QString params = urlQuery.query();
//    qDebug() << "param :" << params;

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

//    qDebug() << "method POST";
    manager->post(request, params.toUtf8());
}

int TLEDownloader::receiveTLEData(QString tleData)
{
    int successChecker;
    if (tleData.mid(0,1) == "1"){
        //split line1
        this->results << tleData.mid(2,5).trimmed(); //satellite number
        this->results << tleData.mid(7,1).trimmed(); //classification
        this->results << tleData.mid(9,2).trimmed(); //launch year
        this->results << tleData.mid(11,3).trimmed(); //launch number of the year
        this->results << tleData.mid(14,3).trimmed(); //piece of the luanch
        this->results << tleData.mid(18,2).trimmed(); //epoch year
        this->results << tleData.mid(20,12).trimmed(); //epoch day of year
        this->results << tleData.mid(33,10).trimmed(); //mean motion devided by two
        this->results << tleData.mid(44,8).trimmed(); //mean motion devided by six
        this->results << tleData.mid(53,8).trimmed(); //BSTAR
        this->results << tleData.mid(62,1).trimmed(); //ephemeris type
        this->results << tleData.mid(64,4).trimmed(); //element set number
        this->results << tleData.mid(68,1).trimmed(); //checksum
    }else if (tleData.mid(0,1) == "2"){
        //split line2
        this->results << tleData.mid(2,5).trimmed();
        this->results << tleData.mid(8,8).trimmed();
        this->results << tleData.mid(17,8).trimmed();
        this->results << tleData.mid(26,7).trimmed();
        this->results << tleData.mid(34,8).trimmed();
        this->results << tleData.mid(43,8).trimmed();
        this->results << tleData.mid(52,11).trimmed();
        this->results << tleData.mid(63,5).trimmed();
        this->results << tleData.mid(68,1).trimmed();
        successChecker = this->insertTLE(results);
        //qDebug() << "insert transaction success :" << successChecker;
        this->results.clear();
    }
    return successChecker;
}

int TLEDownloader::insertTLE(QStringList tleData)
{
    int result;
    QString strQuery = "";
    //qDebug() << "start insert" << QDateTime::currentDateTimeUtc().toString("dd/MM/yyyy HH:mm:ss.zzz");
    strQuery += "SET @psat_no = " + tleData[0] + ";";
    strQuery += "SET @pclassification = '" + tleData[1] + "';";
    if(tleData[2] != ""){
        strQuery += "SET @plaunch_year = '" + tleData[2] + "';";
    }else{
        strQuery += "SET @plaunch_year = NULL;"; //use for space_object table
        if(this->processType == 2) strQuery += "SET @plaunch_year = '-';"; //use for separate_space_object table
    }

    if(tleData[3] != ""){
        strQuery += "SET @plaunch_no = '" + tleData[3] + "';";
    }else{
        strQuery += "SET @plaunch_no = NULL;"; //use for space_object table
        if(this->processType == 2) strQuery += "SET @plaunch_no = '-';"; //use for separate_space_object table
    }

    if(tleData[4] != ""){
        strQuery += "SET @ppiece_launch = '" + tleData[4] + "';";
    }else{
        strQuery += "SET @ppiece_launch = NULL;"; //use for space_object table
        if(this->processType == 2) strQuery += "SET @ppiece_launch = '-';"; //use for separate_space_object table
    }

    strQuery += "SET @pepoch_year = '" + tleData[5] + "';";
    strQuery += "SET @pepoch_doy = " + tleData[6] + ";";
    strQuery += "SET @p1st_mean_motion = '" + tleData[7] + "';";
    strQuery += "SET @p2nd_mean_motion = '" + tleData[8] + "';";
    strQuery += "SET @pbstar = '" + tleData[9] + "';";
    strQuery += "SET @pephe_type = " + tleData[10] + ";";
    strQuery += "SET @pelement_no = " + tleData[11] + ";";
    strQuery += "SET @pchecksum_line1 = " + tleData[12] + ";";
    strQuery += "SET @pinclination = " + tleData[14] + ";";
    strQuery += "SET @pRAAN = " + tleData[15] + ";";
    strQuery += "SET @peccentricity = '" + tleData[16] + "';";
    strQuery += "SET @parg_of_perigee = " + tleData[17] + ";";
    strQuery += "SET @pmean_anomaly = " + tleData[18] + ";";
    strQuery += "SET @pmean_motion = " + tleData[19] + ";";
    strQuery += "SET @prev_no = " + tleData[20] + ";";
    strQuery += "SET @pchecksum_line2 = " + tleData[21] + ";";
    if(this->processType == 1) strQuery += "CALL insertTLE(@psat_no, @pclassification,@plaunch_year,@plaunch_no,@ppiece_launch,@pepoch_year,@pepoch_doy,";
    else if(this->processType == 2) strQuery += "CALL manage_TLE_data(@psat_no, @pclassification,@plaunch_year,@plaunch_no,@ppiece_launch,@pepoch_year,@pepoch_doy,";
    else if(this->processType == 3) strQuery += "CALL dbdTLEUpdate(@psat_no, @pclassification,@plaunch_year,@plaunch_no,@ppiece_launch,@pepoch_year,@pepoch_doy,";
    strQuery += "@p1st_mean_motion,@p2nd_mean_motion,@pbstar,@pephe_type,@pelement_no,@pchecksum_line1,@pinclination,";
    strQuery += "@pRAAN,@peccentricity,@parg_of_perigee,@pmean_anomaly,@pmean_motion,@prev_no,@pchecksum_line2, @pResult);";

    //qDebug() << "end insert" << QDateTime::currentDateTimeUtc().toString("dd/MM/yyyy HH:mm:ss.zzz");
    // pResult = 0 is satellite is not found, pResult = 1 is satellite and epoch are found, pResult = 2 is satellite is found but epoch is not found ;
    QSqlQuery query;
    query.exec(strQuery);
    query.exec("SELECT @pResult AS 'pResult';");
    query.next();
    result = query.value(0).toInt();

    return result;
}

void TLEDownloader::selectDATA()
{
    qDebug() << "start" << QDateTime::currentDateTimeUtc().toString("dd/MM/yyyy HH:mm:ss.zzz");
    QSqlQuery query;
    query.exec("SELECT * from 33396u order by id desc");
    while(query.next()){
        qDebug() << query.value(1).toString() << query.value(2).toString() << query.value(3).toString() << query.value(4).toString()
                    << query.value(5).toString() << query.value(6).toString() << query.value(7).toString() << query.value(8).toString()
                       << query.value(9).toString() << query.value(10).toString() << query.value(11).toString() << query.value(12).toString()
                          << query.value(13).toString() << query.value(14).toString() << query.value(15).toString() << query.value(16).toString();
        qDebug() << "end" << QDateTime::currentDateTimeUtc().toString("dd/MM/yyyy HH:mm:ss.zzz");
    }
}

void TLEDownloader::save_time_process(QDateTime start,QDateTime end,int process)
{
    QSqlQuery query;
    query.prepare("INSERT INTO report (process, start, end) "
                  "VALUES (?, ?, ?)");
    query.addBindValue(process);
    query.addBindValue(start);
    query.addBindValue(end);
    query.exec();
}

void TLEDownloader::test_timer()
{
    this->getTLEData(0);
}
