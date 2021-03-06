#ifndef STACKINFO_H
#define STACKINFO_H

#include <QWidget>
#include "bms_model.h"

namespace Ui {
class StackInfo;
}
class BMS_System;
class BMSCollector;

class StackInfo : public QWidget
{
    Q_OBJECT

public:
    explicit StackInfo(QWidget *parent = nullptr);
    ~StackInfo();
    void loadConfiguration(QString path);
    void setCollector(BMSCollector *c);

private slots:
    void on_tableView_2_activated(const QModelIndex &index);

    void on_tableView_2_clicked(const QModelIndex &index);

    void on_pbNextStack_clicked();

    void on_pbPreviousStack_clicked();

    void on_pbSystemNavi_clicked();

    void on_system_config_ready();

    void on_system_data_ready();

    void on_pbSwitchInfo_clicked();

    void on_pbSetDO_0_clicked();

    void on_pbSetDO_1_clicked();

    void on_pbSetVsource_0_clicked();

    void on_pbSetVsource_1_clicked();

private:
    void dummyData();
    void updateStackInfo();
private:
    Ui::StackInfo *ui;
    BMS_StackModel *stackModel;
    BMS_BatteryModel *batteryModel;
    int m_currentStackIndex;
    int m_currentSystemIndex;
    QList<BMS_System*> m_bmsInfo;
    BMS_System *activeSystem;
    BMSCollector *collector;
};

#endif // STACKINFO_H
