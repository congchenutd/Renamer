#include "Exif.h"
#include "Renamer.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QRegularExpression>
#include <cmath>

/**
 * @brief Rename a list of files based on a given template
 * @param settings  - the renaming template
 * @param fileInfos - the list of files
 * @return          - a list of new names
 */
QStringList Renamer::run(QSettings* settings, const QFileInfoList& fileInfos, const QList<QDateTime>& dateTimes)
{
    QMap<QDate, int> date2Count;   // date -> total # of files on that date
    foreach (const QDateTime& dateTime, dateTimes)
        date2Count[dateTime.date()] ++;

    QStringList result;
    QMap<QDate, int> date2Index;   // date -> index (starting from 1) of the file in the list of that date
    for (int i = 0; i < fileInfos.length(); ++i)
    {
        QFileInfo fileInfo = fileInfos.at(i);
        QDate date = dateTimes.at(i).date();
        date2Index[date] ++;

        QString newName = run(settings, fileInfo, dateTimes.at(i), result, date2Count[date],
                              date2Index[date], static_cast<int>(log10(date2Count[date])) + 1);
        result << newName;
    }
    return result;
}

/**
 * @brief Get the new name of a file based on a template
 * @param settings  - renaming template
 * @param fileInfo  - the file to be renamed
 * @param newPaths  - paths of files already renamed yet to be written to disk
 * @param groupSize - # of files in the same-dated file group
 * @param index     - index of this file in the group
 * @param length    - length of the index (ie, how many digits)
 * @return          - a valid new name
 */
QString Renamer::run(QSettings* settings, const QFileInfo& fileInfo, const QDateTime& dateTime,
                     const QStringList& newFilePaths, int groupSize, int index, int length)
{
    // Load the template
    QString separator       = settings->value("Separator")      .toString();
    QString datePattern     = settings->value("DatePattern")    .toString();
    QString people          = settings->value("People")         .toString();
    QString event           = settings->value("Event")          .toString();
    QString indexPattern    = settings->value("IndexPattern")   .toString();


    QStringList sections;
    if (!datePattern.isEmpty())
        sections << dateTime.toString(datePattern);
    if (!people.isEmpty())
        sections << people;
    if (!event.isEmpty())
        sections << event;

    // index
    if (groupSize > 1)
    {
        QString indexNumber = indexPattern;
        indexNumber.replace("$00$", QString("%1").arg(QString::number(index), length, '0'));   // pad with 0
        indexNumber.replace("$0$",  QString::number(index));                                   // no padding
        sections << indexNumber;
    }

    // path and file extension are not changed
    QString destPath = fileInfo.path();
    QString extension = fileInfo.suffix().isEmpty() ? QString()
                                                    : "." + fileInfo.suffix();
    QString filePath = destPath + QDir::separator() + sections.join(separator) + extension; // [path]/[file name][.extension]
    return getValidFilePath(filePath, newFilePaths);    // check duplication
}

/**
 * @brief Attemps to find a valid name that a given file can be renamed to.
 * @param filePath  - the file to be renamed
 * @param newPaths  - paths of files already renamed yet to be written to disk
 * @return          - A valid new file path
 */
QString Renamer::getValidFilePath(const QString& filePath, const QStringList& newFilePaths)
{
    // No duplication, return
    if (!QFile::exists(filePath) && !newFilePaths.contains(filePath))
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
        return getValidFilePath(newFilePath, newFilePaths); // check the name again
    }

    // Increase duplication number
    int number = numberString.toInt() + 1;
    QString newFilePath = filePath;
    newFilePath.replace(re, QString("(%1)").arg(number));
    return getValidFilePath(newFilePath, newFilePaths); // check the name again
}
