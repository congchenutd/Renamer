#ifndef RENAMER_H
#define RENAMER_H

#include <QFileInfoList>
#include <QString>

class QSettings;
class QFileInfo;

class Renamer
{
public:
    QStringList run(QSettings* settings, const QFileInfoList& filePaths, const QList<QDateTime>& dateTimes);

private:
    QString run(QSettings* settings, const QFileInfo& fileInfo, const QDateTime& dateTime,
                const QStringList& newFilePaths, int groupSize, int index = 0, int length = 3);
    QString getValidFilePath(const QString& filePath, const QStringList& newPaths);
};

#endif // RENAMER_H
