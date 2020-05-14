#pragma once

#include "Exif.h"
#include <QMainWindow>
#include <QSet>
#include <QSettings>
#include <QStandardItemModel>
#include <QFutureWatcher>
#include <QRunnable>
#include <QObject>

namespace Ui {
class MainWindow;
}

class QProgressBar;
class QItemSelection;

class ExifLoaderThread : public QObject, public QRunnable
{
    Q_OBJECT

public:
    ExifLoaderThread(const QString& filePath);
    void run() override;

signals:
    void resultReady(const Exif& exif);

private:
    QString _filePath;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent* e);
    void dropEvent     (QDropEvent* e);

private slots:
    void onAdd();
    void onDel();
    void onRename();
    void onClean();
    void onUseModified();
    void onUseExif();
    void onSettings();
    void onAbout();
    void onSelectionChanged(const QItemSelection& selection);
    void onFixDate();
    void onExifLoaded(const Exif& exif);

private:
    void addFiles(const QStringList& filePaths);
    void preview();
    void updateActions();
    QModelIndexList getSelected() const;

private:
    Ui::MainWindow* ui;
    QStandardItemModel  _model;
    QProgressBar*       _progressBar;
    QSettings           _settings;

    QSet<QString>       _filePaths;

    enum {COL_FROM, COL_TO, COL_DATE, COL_MODIFIED_DATE, COL_EXIF_DATE};

    QFutureWatcher<Exif> _watcher;

    size_t _numLoadingFiles{0};
};
