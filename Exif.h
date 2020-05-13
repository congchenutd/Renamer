#pragma once

#include <QMap>
#include <QString>

class Photo;

class Exif
{
public:
    using Data = QMap<QString, QString>;

    Exif() = default;
    Exif(const QString& filePath);
    Data getData() const;
    QString getValue(const QString& property, bool fuzzy = false) const;
    void setValue(const QString& property, const QString& value);
    QString getFilePath() const;

private:
    Data _data;
    QString _filePath;
};
