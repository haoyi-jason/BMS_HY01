#ifndef MAININFO_H
#define MAININFO_H

#include <QWidget>

namespace Ui {
class MainInfo;
}

class MainInfo : public QWidget
{
    Q_OBJECT

public:
    explicit MainInfo(QWidget *parent = nullptr);
    ~MainInfo();

private:
    Ui::MainInfo *ui;
};

#endif // MAININFO_H
