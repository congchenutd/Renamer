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

    typedef enum {PREVIEW, RENAME} ActionCode;
    ActionCode getActionCode() const;

private slots:
    void onFont();
    void onPreview();
    void onRename();

private:
    Ui::DlgSettings ui;
    QSettings   _settings;
    ActionCode  _actionCode;
};

#endif // DLGSETTINGS_H
