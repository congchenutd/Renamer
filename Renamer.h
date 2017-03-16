#ifndef RENAMER_H
#define RENAMER_H

#include <QString>

class QSettings;
class QFileInfo;

class Renamer
{
public:
    QString run(QSettings* settings, const QFileInfo& fileInfo, int index = 0, int length = 3);
};

#endif // RENAMER_H
