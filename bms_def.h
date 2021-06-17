#ifndef BMS_DEF_H
#define BMS_DEF_H

#include <QObject>
#include <QtCore>
#include <QtNetwork>
#include <QDateTime>
//#include "bms_bcudevice.h"

class QTcpSocket;
class CANBUSDevice;
class MODBUSDevice;
class BMS_BCUDevice;
class BMS_System;

namespace bms {
#define GROUP_OF(x)     (x >> 5)
#define ID_OF(x)        (x & 0x1F)
#define GROUP(x)        (x<< 5)
#define ID(x)           (x)
#define SVI_ID          0x1F
#define BCU_ID          0x01
    const int MAX_CELLS = 12;
    const int MAX_NTC = 5;

    enum AlarmID{
        STACK_OV,
        STACK_UV,
        STACK_OT,
        STACK_UT,
        CELL_OV,
        CELL_UV,
        CELL_OT,
        CELL_UT,
        BMU_LOST,
        BCU_LOST,
        SVI_LOST
    };
}


class HW_IOChannel{
public:
    explicit HW_IOChannel(int initValue = 0, bool inverted = false){
        m_value = initValue;
        m_valueToWrite = initValue;
        //m_valueReadBack = initValue;
    }

    int value(){return m_value;}
    void value(int v){m_value = v;}
    void writeValue(int value){m_valueToWrite = value;}
    int writeValue(){return m_valueToWrite;}
    //void setReadback(int value){m_valueReadBack = value;}
    bool valid(){return (m_valueToWrite == m_value);}
    bool valid_write(){return !(m_valueToWrite == m_value);}
    void limit(int v){m_limit = v;}
    int limit(){return m_limit;}
private:
    int m_valueToWrite;
    //int m_valueReadBack;
    int m_value;
    int m_limit;
};

class CAN_Packet{
public:
    quint16 Command;
    QByteArray data;
    bool readFrame= false;
    bool writeOK = false;
    bool remote = false;
};

class _EventItem{
public:
    int setValue;
    int resetValue;
    int holdTime;
    int activeTime;
};

class ValueDef{
public:
    qint32 value_set = 0;
    qint32 value_reset = 0;
    qint32 holdTime = 0;
    qint32 activeCount = 0;
    //bool activeated=false;
    bool valid(){return (activeCount > holdTime);}
};

class EventDef{
public:
    ValueDef high_thres;
    ValueDef low_thres;
    bool high_active = false;
    bool low_active = false;
    bool autoreset = false;
    QString name = "";
};

class _EventCriteria
{

public:
    ValueDef *alarmHigh = nullptr;
    ValueDef *alarmLow = nullptr;
    ValueDef *warningHigh = nullptr;
    ValueDef *warningLow = nullptr;
    void validEvents(qint32 tpv){
        if(alarmHigh != nullptr){
            if(tpv > alarmHigh->value_set){
                alarmHigh->activeCount++;
            }
            else{
                alarmHigh->activeCount = 0;
            }
        }

        if(warningHigh != nullptr){
            if(tpv > warningHigh->value_set){
                warningHigh->activeCount++;
            }
            else{
                warningHigh->activeCount = 0;
            }
        }

        if(alarmLow != nullptr){
            if(tpv < alarmLow->value_set){
                alarmLow->activeCount++;
            }
            else{
                alarmLow->activeCount = 0;
            }
        }
        if(warningLow != nullptr){
            if(tpv < warningLow->value_set){
                warningLow->activeCount++;
            }
            else{
                warningLow->activeCount = 0;
            }
        }
    }

    //qint32 tpv = 0; // current value
};

class _Events{
public:
    int eventType;
    int startTime;
    QString eventDescription;
};



/*
 *  stack information of a BMS
 *  each stack contains n battery and at most 1 stack voltage/current module,
 *  32-devices per stack with 8-bit CANBUS device address mapping, MSB 3-bits
 *  for group, LSB 5-bit for address
 *  1. stack voltage/current module config at fixed address 0x01
 *  2. battery (at most 12-cells) start from 2 to 31, 29 batteries most
 *  3. address 0x0 (group 0/ id 0) is for broadcasting.
 */






class BMS_Event:public QObject{
    Q_OBJECT
public:
    explicit BMS_Event(QObject *parent = nullptr){}
    friend QTextStream &operator << (QTextStream &out,const BMS_Event *event){
        out << QString("%1,").arg(event->m_evtID);
        out << QString("%1,").arg(event->m_evtLevel);
        out << QString("%1,").arg(event->m_isAlarm?"TRUE":"FALSE");
        out << QString("%1,").arg(event->m_isWarning?"TRUE":"FALSE");
        out << event->m_timeStamp.toString("yyyy:MM:dd hh-mm-ss")<<",";
        out << event->m_description<<"\n";
    }

    bool parse(QString msg){
        QStringList sl = msg.split(",");
        if(sl.size() == 6){
            m_evtID = sl[0].toInt();
            m_evtLevel = sl[1].toInt();
            m_isAlarm = sl[2].contains("TRUE");
            m_isWarning = sl[3].contains("TRUE");
            m_timeStamp = QDateTime::fromString(sl[4]);
            m_description = sl[5];
        }
        else{
            return false;
        }
    }



public:
    int m_evtID;
    int m_evtLevel;
    bool m_isAlarm;
    bool m_isWarning;
    QDateTime m_timeStamp;
    QString m_description;

};


class RemoteSystem{
public:
    QString connection="";
    QTcpSocket *socket = nullptr;
    BMS_System *system = nullptr;
    bool configReady = false; // should be false
    QByteArray data;
    QString alias;
    QString logPath;
    int port;
    bool autoConnect;
    bool enableLog=false;
    int logDays = -1;
    int logRecords = 1000;
    void setDigitalOut(int id, int value){
        QString msg = QString("BCU:DO:%1:%2").arg(id).arg(value);
        writeCommand(msg);
//        if(socket != nullptr){
//            socket->write(msg.toUtf8());
//        }
    }

    void setVoltageSource(int id, int value){
        QString msg = QString("BCU:VO:%1:%2").arg(id).arg(value);
        writeCommand(msg);
//        if(socket != nullptr){
//            socket->write(msg.toUtf8());
//        }
    }

    void openSerialPort(QString name, int baudrate, int parity, int stopBits){
        QString msg = QString("PORT:OPEN:%1:%2:%3:%4").arg(name).arg(baudrate).arg(parity).arg(stopBits);
        writeCommand(msg);
//        if(socket != nullptr){
//            socket->write(msg.toUtf8());
//        }
    }

    void closeSerialPort(QString name){
        QString msg = QString("PORT:CLOSE:%1").arg(name);
        writeCommand(msg);
//        if(socket != nullptr){
//            socket->write(msg.toUtf8());
//        }
    }

    void writeSerialPort(QString name, QString data){
        QString msg = QString("PORT:WRITE:%1:%2").arg(name).arg(data);
        writeCommand(msg);
//        if(socket != nullptr){
//            socket->write(msg.toUtf8());
//        }

    }
    void writeCommand(QString command){
        if(socket != nullptr){
            socket->write(command.toUtf8());
        }
    }

    void logData(QByteArray b){
//        QByteArray b;
//        QDataStream ds(&b,QIODevice::ReadWrite);
//        ds << this->system;
        QString path = this->logPath + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss.bin");
        QFile f(path);
        if(f.open(QIODevice::ReadWrite)){
            //f.write(d);
            QDataStream df(&f);
            df << b;
            f.close();
        }

        // remove old files, keep 1000 only
        QDir dir(this->logPath);
        if(dir.count() > 110){
            QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time | QDir::Reversed);
            for(int i=files.size();i>100;--i){
                const QFileInfo& info = files.at(i-1);
                QFile::remove(info.absoluteFilePath());
            }
        }

    }
};



#endif // BMS_DEF_H
