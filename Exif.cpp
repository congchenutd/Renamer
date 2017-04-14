#include "Exif.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>
#include <QDebug>

Exif::Exif(const QString& filePath)
{
    if (filePath.isEmpty())
        return;

    QSettings settings("Settings.ini", QSettings::IniFormat);
    QString exiftoolPath = settings.value("ExiftoolPath").toString();
    if (exiftoolPath.isEmpty() || !QFile::exists(exiftoolPath))
        return;

    QProcess* process = new QProcess;
    process->start(exiftoolPath, QStringList() << filePath);
    process->waitForFinished();

    QStringList list = QString(process->readAllStandardOutput()).split("\n");
    foreach (QString line, list)
    {
        int indexColon = line.indexOf(':');
        if (indexColon > 0)
        {
            QString property    = line.left(indexColon).simplified();
            QString value       = line.right(line.length() - indexColon - 1).simplified();
            if (!_data.contains(property))
                _data.insert(property, value);
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
        if (it.key().startsWith(property, Qt::CaseInsensitive))
            return it.value();
    return QString();
}

void Exif::setValue(const QString& property, const QString& value) {
    _data[property] = value;
}
