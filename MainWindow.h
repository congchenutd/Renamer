#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QStandardItemModel>

namespace Ui {
class MainWindow;
}

class QProgressBar;

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
    void onRun();
    void onClean();
    void onSettings();
    void onAbout();

private:
    void addFiles(const QStringList& filePaths);
    void preview();
    void updateActions();

private:
    Ui::MainWindow* ui;
    QStandardItemModel  _model;
    QProgressBar*       _progressBar;
    QSettings           _settings;

    enum {COL_FROM, COL_TO, COL_DATE};
};

#endif // MAINWINDOW_H
