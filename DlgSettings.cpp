#include "DlgSettings.h"
#include <QFileDialog>
#include <QFontDialog>

DlgSettings::DlgSettings(QWidget* parent) :
    QDialog(parent),
    _settings("Settings.ini", QSettings::IniFormat)
{
    ui.setupUi(this);
    connect(ui.btFont,      SIGNAL(clicked()), this, SLOT(onFont()));
    connect(ui.btPreview,   SIGNAL(clicked()), this, SLOT(onPreview()));
    connect(ui.btRename,    SIGNAL(clicked()), this, SLOT(onRename()));

    ui.leGeneralPattern ->setText(_settings.value("Separator")      .toString());
    ui.leDatePattern    ->setText(_settings.value("DatePattern")    .toString());
    ui.leEvent          ->setText(_settings.value("Event")          .toString());
    ui.lePeople         ->setText(_settings.value("People")         .toString());
    ui.leIndexPattern   ->setText(_settings.value("IndexPattern")   .toString());
    ui.leExiftoolPath   ->setText(_settings.value("ExiftoolPath")   .toString());
}

void DlgSettings::accept()
{
    _settings.setValue("Separator",         ui.leGeneralPattern ->text());
    _settings.setValue("DatePattern",       ui.leDatePattern    ->text());
    _settings.setValue("People",            ui.lePeople         ->text());
    _settings.setValue("Event",             ui.leEvent          ->text());
    _settings.setValue("IndexPattern",      ui.leIndexPattern   ->text());
    _settings.setValue("ExiftoolPath",      ui.leExiftoolPath   ->text());
    _settings.setValue("Font",              ui.btFont->font().toString());
    qApp->setFont(ui.btFont->font());
    QDialog::accept();
}

DlgSettings::ActionCode DlgSettings::getActionCode() const {
    return _actionCode;
}

void DlgSettings::onFont() {
    ui.btFont->setFont(QFontDialog::getFont(0, ui.btFont->font()));
}

void DlgSettings::onPreview()
{
    _actionCode = PREVIEW;
    accept();
}

void DlgSettings::onRename()
{
    _actionCode = RENAME;
    accept();
}
