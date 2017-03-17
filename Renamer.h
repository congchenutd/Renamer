#ifndef RENAMER_H
#define RENAMER_H

#include <QString>

class QSettings;
class QFileInfo;

class Renamer
{
public:
    QString run(QSettings* settings, const QFileInfo& fileInfo, int index = 0, int length = 3);

private:
    QString getValidFilePath(const QString& filePath);
};

#endif // RENAMER_H
