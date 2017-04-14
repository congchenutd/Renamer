#ifndef EXIF_H
#define EXIF_H

#include <QMap>
#include <QString>

class Photo;

class Exif
{
public:
    typedef QMap<QString, QString> Data;

    Exif(const QString& filePath);
    Data getData() const;
    QString getValue(const QString& property, bool fuzzy = false) const;
    void setValue(const QString& property, const QString& value);

private:
    Data _data;
};

#endif
