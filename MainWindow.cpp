#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Renamer.h"
#include "DlgSettings.h"
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QProgressBar>
#include <QDragEnterEvent>
#include <QMimeData>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _settings("Settings.ini", QSettings::IniFormat)
{
    ui->setupUi(this);
    setAcceptDrops(true);

    if(!_settings.value("Font").toString().isEmpty())
    {
        QFont font;
        font.fromString(_settings.value("Font").toString());
        qApp->setFont(font);
    }

    _progressBar = new QProgressBar(this);
    statusBar()->addPermanentWidget(_progressBar);
    _progressBar->hide();

    updateActions();

    _model.setColumnCount(3);
    _model.setHeaderData(COL_FROM, Qt::Horizontal, tr("From"));
    _model.setHeaderData(COL_TO,   Qt::Horizontal, tr("To"));
    _model.setHeaderData(COL_DATE, Qt::Horizontal, tr("Date"));

    ui->tableView->setModel(&_model);
    ui->tableView->sortByColumn(COL_DATE, Qt::AscendingOrder);

    connect(ui->actionAdd,      SIGNAL(triggered()), this, SLOT(onAdd()));
    connect(ui->actionPreview,  SIGNAL(triggered()), this, SLOT(onPreview()));
    connect(ui->actionRun,      SIGNAL(triggered()), this, SLOT(onRun()));
    connect(ui->actionEmpty,    SIGNAL(triggered()), this, SLOT(onClean()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(onSettings()));
    connect(ui->actionAbout,    SIGNAL(triggered()), this, SLOT(onAbout()));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent* e)
{
    if(e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* e)
{
    QStringList filePaths;
    foreach(const QUrl& url, e->mimeData()->urls())
        filePaths << url.toLocalFile();

    add(filePaths);
}

void MainWindow::add(const QStringList& filePaths)
{
    foreach(const QString& fileName, filePaths)
    {
        QFileInfo fileInfo(fileName);
        int lastRow = _model.rowCount();
        _model.insertRow(lastRow);
        _model.setData(_model.index(lastRow, COL_FROM),
                       QDir::toNativeSeparators(fileInfo.filePath()));
        _model.setData(_model.index(lastRow, COL_DATE),
                       fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss"));
    }
    ui->tableView->sortByColumn(COL_DATE, Qt::AscendingOrder);
    ui->tableView->resizeColumnsToContents();
    updateActions();
}

void MainWindow::onAdd()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("Open files"), ".",
                                                          "All files (*.*)");
    if(!filePaths.isEmpty())
        add(filePaths);
}

void MainWindow::onPreview()
{
    QMap<QDate, int> date2Count;   // date -> total # of files on that date
    for(int row = 0; row < _model.rowCount(); ++row)
    {
        QFileInfo fileInfo(_model.data(_model.index(row, COL_FROM)).toString());
        QDate date = fileInfo.lastModified().date();
        date2Count[date] ++;
    }

    QMap<QDate, int> date2Index;   // date -> current number of files on that date
    for(int row = 0; row < _model.rowCount(); ++row)
    {
        QFileInfo fileInfo(_model.data(_model.index(row, COL_FROM)).toString());
        QDate date = fileInfo.lastModified().date();
        date2Index[date] ++;

        Renamer renamer;
        QString newName = renamer.run(&_settings, fileInfo, date2Index[date], log10(date2Count[date]) + 1);
        _model.setData(_model.index(row, COL_TO), newName);
    }
    ui->tableView->resizeColumnsToContents();
}

void MainWindow::onRun()
{
    if (_model.data(_model.index(0, COL_TO)).isNull())
        onPreview();

    _progressBar->show();
    _progressBar->setMaximum(_model.rowCount());
    for(int row = 0; row < _model.rowCount(); ++row)
    {
        QString from = _model.data(_model.index(row, COL_FROM)).toString();
        QString to   = _model.data(_model.index(row, COL_TO))  .toString();
        if(_model.data(_model.index(row, COL_TO)) .toString().isEmpty())
            continue;
        QFile::rename(from, to);
        _progressBar->setValue(row);
        qApp->processEvents();
    }
    _progressBar->hide();
    onClean();
}

void MainWindow::onClean()
{
    _model.removeRows(0, _model.rowCount());
    updateActions();
}

void MainWindow::onSettings()
{
    DlgSettings dlg(this);
    dlg.exec();
}

void MainWindow::onAbout() {
    QMessageBox::about(this, tr("About"),
                       tr("<h3><b>Rename by Date</b></h3>"
                          "<p>Built on 01/27/2016</p>"
                          "<p><a href=mailto:CongChenUTD@Gmail.com>CongChenUTD@Gmail.com</a></p>"));
}

void MainWindow::updateActions()
{
    ui->actionPreview->setEnabled(_model.rowCount() > 0);
    ui->actionEmpty  ->setEnabled(_model.rowCount() > 0);
    ui->actionRun    ->setEnabled(_model.rowCount() > 0);
}
