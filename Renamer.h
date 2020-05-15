#pragma once

#include <QFileInfoList>
#include <QString>

class QSettings;
class QFileInfo;

class Renamer
{
public:
    /**
     * @brief Rename a list of files based on a given template
     * @param settings  - the renaming template
     * @param fileInfos - the list of files
     * @return          - a list of new names
     */
    QStringList run(QSettings* settings, const QFileInfoList& filePaths, const QList<QDateTime>& dateTimes);

private:
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
    QString run(QSettings* settings, const QFileInfo& fileInfo, const QDateTime& dateTime,
                const QStringList& newFilePaths, int groupSize, int index = 0, int length = 3);

    /**
     * @brief Attemps to find a valid name that a given file can be renamed to.
     * @param filePath  - the file to be renamed
     * @param newPaths  - paths of files already renamed yet to be written to disk
     * @return          - A valid new file path
     */
    QString getValidFilePath(const QString& filePath, const QStringList& newPaths);
};
