#pragma once

#include <QMap>
#include <QString>

///
/// @brief Models the EXIF of a file
///
class Exif
{
public:
    using Data = QMap<QString, QString>;

    Exif() = default;
    Exif(const QString& filePath);

    Data getData() const;
    QString getValue(const QString& property, bool fuzzy = false) const;
    QString getValue(const QStringList& properties, bool fuzzy = false) const;

    void setValue(const QString& property, const QString& value);
    QString getFilePath() const;

private:
    // Key value pairs
    Data _data;

    // File path
    QString _filePath;
};
