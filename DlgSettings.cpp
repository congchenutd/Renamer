#include "DlgSettings.h"
#include <QFileDialog>
#include <QFontDialog>

DlgSettings::DlgSettings(QWidget *parent) :
    QDialog(parent),
    _settings("Settings.ini", QSettings::IniFormat)
{
    ui.setupUi(this);
    connect(ui.btFont, SIGNAL(clicked()), this, SLOT(onFont()));

    ui.leGeneralPattern ->setText(_settings.value("GeneralPattern").toString());
    ui.leDatePattern    ->setText(_settings.value("DatePattern").toString());
    ui.leEvent          ->setText(_settings.value("EventPattern").toString());
    ui.leIndexPattern   ->setText(_settings.value("IndexPattern").toString());
}

void DlgSettings::accept()
{
    _settings.setValue("GeneralPattern",    ui.leGeneralPattern->text());
    _settings.setValue("DatePattern",       ui.leDatePattern->text());
    _settings.setValue("Event",             ui.leEvent      ->text());
    _settings.setValue("IndexPattern",      ui.leIndexPattern->text());
    _settings.setValue("Font",              ui.btFont->font().toString());
    qApp->setFont(ui.btFont->font());
    QDialog::accept();
}

void DlgSettings::onFont() {
    ui.btFont->setFont(QFontDialog::getFont(0, ui.btFont->font()));
}
