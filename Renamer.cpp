#include "Renamer.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

QString Renamer::run(QSettings* settings, const QFileInfo& fileInfo, int index, int length)
{
    QString result = settings->value("GeneralPattern").toString();
    QString datePattern  = settings->value("DatePattern") .toString();
    QString eventPattern = settings->value("EventPattern").toString();
    QString indexPattern = settings->value("IndexPattern").toString();

    QString destPath = fileInfo.path();
    QString suffix = fileInfo.suffix().isEmpty() ? QString()
                                                 : "." + fileInfo.suffix();

    QString date = fileInfo.lastModified().toString(datePattern);
    result.replace("$Date$", date);
    result.replace("$Event$", eventPattern);

    result.replace("$Index$", indexPattern);
    result.replace("$00$", QString("%1").arg(QString::number(index), length, '0'));  // pad with 0
    result.replace("$0$", QString::number(index));                                   // no padding
    result.prepend(destPath + QDir::separator());
    result.append(suffix);
    return result;
}

