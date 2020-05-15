#include "Exif.h"

#include <QFile>
#include <QProcess>
#include <QSettings>

Exif::Exif(const QString& filePath) : _filePath(filePath)
{
    if (_filePath.isEmpty())
        return;

    QSettings settings("Settings.ini", QSettings::IniFormat);
    QString exiftoolPath = settings.value("ExiftoolPath").toString();
    if (exiftoolPath.isEmpty() || !QFile::exists(exiftoolPath))
        return;

    // Run the exiftool
    QProcess* process = new QProcess;
    process->start(exiftoolPath, QStringList() << _filePath);
    process->waitForFinished();

    const QStringList& propertyList = QString(process->readAllStandardOutput()).split("\n");
    for (QString line: propertyList)
    {
        if (const int indexColon = line.indexOf(':'); indexColon > 0)
        {
            const QString property    = line.left(indexColon).simplified();
            const QString value       = line.right(line.length() - indexColon - 1).simplified();
            setValue(property, value);
        }
    }
}

Exif::Data Exif::getData() const {
    return _data;
}

QString Exif::getValue(const QString& property, bool fuzzy) const
{
    // exact property name
    if (!fuzzy)
        return _data.contains(property) ? _data[property] : QString();

    // fuzzy search
    for (Data::ConstIterator it = _data.begin(); it != _data.end(); ++it)
        if (it.key().contains(property, Qt::CaseInsensitive))
            return it.value();
    return {};
}

QString Exif::getValue(const QStringList& properties, bool fuzzy) const
{
    for (const auto& property: properties)
    {
        const auto value = getValue(property, fuzzy);
        if (!value.isEmpty())
        {
            return value;
        }
    }
    return {};
}

void Exif::setValue(const QString& property, const QString& value) {
    _data[property] = value;
}

QString Exif::getFilePath() const
{
    return _filePath;
}
