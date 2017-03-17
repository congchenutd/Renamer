#include "Renamer.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QRegularExpression>

QString Renamer::run(QSettings* settings, const QFileInfo& fileInfo, int index, int length)
{
    // Fill out the template
    QString separator       = settings->value("Separator")      .toString();
    QString datePattern     = settings->value("DatePattern")    .toString();
    QString people          = settings->value("People")         .toString();
    QString event           = settings->value("Event")          .toString();
    QString indexPattern    = settings->value("IndexPattern")   .toString();

    QString destPath = fileInfo.path();
    QString suffix = fileInfo.suffix().isEmpty() ? QString()
                                                 : "." + fileInfo.suffix();

    QString date = fileInfo.lastModified().toString(datePattern);
    QStringList sections;
    if (!date.isEmpty())
        sections << date;
    if (!people.isEmpty())
        sections << people;
    if (!event.isEmpty())
        sections << event;

    QString indexNumber = indexPattern;
    indexNumber.replace("$00$", QString("%1").arg(QString::number(index), length, '0'));   // pad with 0
    indexNumber.replace("$0$",  QString::number(index));                                   // no padding

    QString filePath = destPath + QDir::separator() + sections.join(separator) + suffix;
    return getValidFilePath(filePath);
}

/**
 * @brief   Attemps to find a valid file name that a given file can be renamed to.
 * @return  A valid new file path
 */
QString Renamer::getValidFilePath(const QString& filePath)
{
    if (!QFile::exists(filePath))
        return filePath;

    // Find duplication suffix, e.g., (1)
    QRegularExpression re("\\((\\d+)\\)");
    QString numberString = re.match(filePath).captured(1);

    // Create duplication suffix
    if (numberString.isEmpty())
    {
        QFileInfo fileInfo(filePath);
        QString path   = fileInfo.path();
        QString suffix = fileInfo.suffix().isEmpty() ? QString()
                                                     : "." + fileInfo.suffix();
        QString newFilePath = path + QDir::separator() + fileInfo.baseName() + " (1)" + suffix;
        return getValidFilePath(newFilePath);
    }

    int number = numberString.toInt() + 1;
    QString newFilePath = filePath;
    newFilePath.replace(re, QString("(%1)").arg(number));
    return getValidFilePath(newFilePath);
}

