#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Renamer.h"
#include "DlgSettings.h"
#include "Exif.h"
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QProgressBar>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QProcess>

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

    _model.setColumnCount(5);
    _model.setHeaderData(COL_FROM,      Qt::Horizontal, tr("From"));
    _model.setHeaderData(COL_TO,        Qt::Horizontal, tr("To"));
    _model.setHeaderData(COL_DATE,      Qt::Horizontal, tr("Date"));
    _model.setHeaderData(COL_MODIFIED_DATE,  Qt::Horizontal, tr("Modified date"));
    _model.setHeaderData(COL_EXIF_DATE, Qt::Horizontal, tr("Exif date"));

    ui->tableView->setModel(&_model);
    ui->tableView->sortByColumn(COL_DATE, Qt::AscendingOrder);

    ui->actionUseModified   ->setEnabled(false);
    ui->actionUseExif       ->setEnabled(false);

    connect(ui->actionAdd,          SIGNAL(triggered()), SLOT(onAdd()));
    connect(ui->actionRun,          SIGNAL(triggered()), SLOT(onRun()));
    connect(ui->actionEmpty,        SIGNAL(triggered()), SLOT(onClean()));
    connect(ui->actionUseModified,  SIGNAL(triggered()), SLOT(onUseModified()));
    connect(ui->actionUseExif,      SIGNAL(triggered()), SLOT(onUseExif()));
    connect(ui->actionSettings,     SIGNAL(triggered()), SLOT(onSettings()));
    connect(ui->actionAbout,        SIGNAL(triggered()), SLOT(onAbout()));
    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            SLOT(onSelectionChanged(QItemSelection)));
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

    addFiles(filePaths);
}

void MainWindow::addFiles(const QStringList& filePaths)
{
    _progressBar->show();
    _progressBar->setMaximum(_model.rowCount());

    const QString dateTimeFormat = "yyyy-MM-dd HH:mm:ss";
    foreach(const QString& filePath, filePaths)
    {
        QFileInfo fileInfo(filePath);
        int row = _model.rowCount();
        _model.insertRow(row);
        _model.setData(_model.index(row, COL_FROM), QDir::toNativeSeparators(fileInfo.filePath()));

        // Load modified date and time
        QDateTime lastModifiedDateTime = QFileInfo(filePath).lastModified();
        _model.setData(_model.index(row, COL_MODIFIED_DATE), lastModifiedDateTime.toString(dateTimeFormat));
        _model.setData(_model.index(row, COL_DATE),          lastModifiedDateTime.toString(dateTimeFormat));

        // Verify date and time using exif
        Exif exif(filePath);
        QString exifDateString = exif.getValue("create", true); // fuzzy search "create" in exif
        if (!exifDateString.isEmpty())
        {
            // Correct date and mark it in red
            QDateTime exifDateTime = QDateTime::fromString(exifDateString, "yyyy:MM:dd hh:mm:ss");
            _model.setData(_model.index(row, COL_EXIF_DATE), exifDateTime.toString(dateTimeFormat));

            if (qAbs(exifDateTime.secsTo(lastModifiedDateTime)) > 60)   // allow 1 minute error
            {
                _model.setData(_model.index(row, COL_DATE), exifDateTime.toString(dateTimeFormat)); // use exif date
                _model.setData(_model.index(row, COL_DATE), QColor(Qt::red), Qt::TextColorRole);    // mark text in red
            }
        }

        _progressBar->setValue(row + 1);
        ui->tableView->resizeColumnsToContents();
        qApp->processEvents();
    }

    _progressBar->hide();
    ui->tableView->sortByColumn(COL_DATE, Qt::AscendingOrder);
    updateActions();
}

void MainWindow::onAdd()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("Open files"), ".",
                                                          "All files (*.*)");
    if(!filePaths.isEmpty())
        addFiles(filePaths);
}

void MainWindow::onUseModified()
{
    foreach (const QModelIndex& idx, getSelected())
    {
        int row = idx.row();
        QString date = _model.data(_model.index(row, COL_MODIFIED_DATE)).toString();
        if (!date.isEmpty())
            _model.setData(_model.index(row, COL_DATE), date);
    }
}

void MainWindow::onUseExif()
{
    foreach (const QModelIndex& idx, getSelected())
    {
        int row = idx.row();
        QString date = _model.data(_model.index(row, COL_EXIF_DATE)).toString();
        if (!date.isEmpty())
            _model.setData(_model.index(row, COL_DATE), date);
    }
}

/**
 * Pre-run the renaming
 * Put result under COL_TO without actually running it
 */
void MainWindow::preview()
{
    // collect input
    QFileInfoList fileInfos;
    QList<QDateTime> dateTimes;
    for(int row = 0; row < _model.rowCount(); ++row)
    {
        fileInfos << QFileInfo(_model.data(_model.index(row, COL_FROM)).toString());
        dateTimes << _model.data(_model.index(row, COL_DATE)).toDateTime();
    }

    // get results
    QStringList newFilePaths = Renamer().run(&_settings, fileInfos, dateTimes);

    // write results to COL_TO
    for(int row = 0; row < _model.rowCount(); ++row)
        _model.setData(_model.index(row, COL_TO), newFilePaths.at(row));

    ui->tableView->resizeColumnsToContents();
}

/**
 * Run renaming
 */
void MainWindow::onRun()
{
    // Run preview if no previewed results
    if (_model.data(_model.index(0, COL_TO)).isNull())
    {
        DlgSettings dlg(this);
        if (dlg.exec() == QDialog::Accepted)
        {
            preview();
            if (dlg.getActionCode() == DlgSettings::PREVIEW)    // user selected preview
                return;
        }
    }

    // Acturally run renaming based on previewed results
    for(int row = 0; row < _model.rowCount(); ++row)
    {
        QString from = _model.data(_model.index(row, COL_FROM)).toString();
        QString to   = _model.data(_model.index(row, COL_TO))  .toString();
        if(to.isEmpty())
            continue;

        // Fix date
        QColor dateColor = _model.data(_model.index(row, COL_DATE), Qt::TextColorRole).value<QColor>();
        if (dateColor == QColor(Qt::red))   // marked in red
        {
            // change modified date
            QDateTime dateTime = _model.data(_model.index(row, COL_DATE)).toDateTime();
            QProcess::execute("touch", QStringList() << "-t" << dateTime.toString("yyyyMMddhhmm") << from);
        }

        QFile::rename(from, to);
    }

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
                       tr("<h3><b>Renamer</b></h3>"
                          "<p>Built on 04/13/2017</p>"
                          "<p><a href=mailto:CongChenUTD@Gmail.com>CongChenUTD@Gmail.com</a></p>"));
}

void MainWindow::onSelectionChanged(const QItemSelection& selection)
{
    ui->actionUseModified   ->setEnabled(!selection.isEmpty());
    ui->actionUseExif       ->setEnabled(!selection.isEmpty());
}

QModelIndexList MainWindow::getSelected() const {
    return ui->tableView->selectionModel()->selectedIndexes();
}

void MainWindow::updateActions()
{
    ui->actionEmpty  ->setEnabled(_model.rowCount() > 0);
    ui->actionRun    ->setEnabled(_model.rowCount() > 0);
}
