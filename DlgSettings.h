#ifndef DLGSETTINGS_H
#define DLGSETTINGS_H

#include "ui_DlgSettings.h"

#include <QSettings>

class DlgSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DlgSettings(QWidget *parent = 0);
    void accept();

private slots:
    void onFont();

private:
    Ui::DlgSettings ui;
    QSettings _settings;
};

#endif // DLGSETTINGS_H
