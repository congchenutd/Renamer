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
#include <QProcess>
#include <QtConcurrent>
#include <QThreadPool>

Exif exifRunner(const QString& filePath)
{
    return Exif(filePath);
};

ExifLoaderThread::ExifLoaderThread(const QString &filePath) : _filePath(filePath)
{
}

//////////////////////////////////////////////////////////////////////////////////

namespace {
constexpr auto ExifDateColor = Qt::darkGreen;
constexpr auto ModifiedDateColor = Qt::blue;
}

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
    _progressBar->setFormat("%v/%m (%p%)");
    statusBar()->addPermanentWidget(_progressBar);
    _progressBar->hide();

    updateActions();

    _model.setColumnCount(5);
    _model.setHorizontalHeaderLabels(QStringList{"From", "To", "Date", "Modified Date", "Exif Date"});

    ui->tableView->setModel(&_model);
    ui->tableView->sortByColumn(COL_DATE, Qt::AscendingOrder);

    onSelectionChanged(QItemSelection());

    connect(ui->actionAdd,          SIGNAL(triggered()), SLOT(onAdd()));
    connect(ui->actionDel,          SIGNAL(triggered()), SLOT(onDel()));
    connect(ui->actionEmpty,        SIGNAL(triggered()), SLOT(onClean()));
    connect(ui->actionUseModified,  SIGNAL(triggered()), SLOT(onUseModified()));
    connect(ui->actionUseExif,      SIGNAL(triggered()), SLOT(onUseExif()));
    connect(ui->actionRename,       SIGNAL(triggered()), SLOT(onRename()));
    connect(ui->actionFixDate,      SIGNAL(triggered()), SLOT(onFixDate()));
    connect(ui->actionSettings,     SIGNAL(triggered()), SLOT(onSettings()));
    connect(ui->actionAbout,        SIGNAL(triggered()), SLOT(onAbout()));
    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            SLOT(onSelectionChanged(QItemSelection)));

    // For queued signal across threads
    qRegisterMetaType<Exif>("Exif");
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
    for(const QUrl& url: e->mimeData()->urls())
        filePaths << url.toLocalFile();

    addFiles(filePaths);
}

void ExifLoaderThread::run()
{
    emit resultReady(Exif(_filePath));
}

void MainWindow::onExifLoaded(const Exif& exif)
{
    const QString dateTimeFormat = "yyyy-MM-dd HH:mm:ss";
    const QSet<QString> videoFileExtensions{"mp4", "mov"};

    QMutexLocker lock(&_mutex);

    int row = _model.rowCount();
    _model.insertRow(row);

    const auto filePath = exif.getFilePath();
    _model.setData(_model.index(row, COL_FROM), QDir::toNativeSeparators(filePath));

    QDateTime lastModifiedDateTime = QFileInfo(filePath).lastModified();
    _model.setData(_model.index(row, COL_MODIFIED_DATE), lastModifiedDateTime.toString(dateTimeFormat));
    _model.setData(_model.index(row, COL_MODIFIED_DATE), QColor(ModifiedDateColor), Qt::ForegroundRole);

    // fuzzy search for "create time" in exif
    QString exifDateString = exif.getValue(QStringList{"Create", "Creation"}, true);

    // Capture the useful part of the date string
    QRegularExpression regex(R"(\d+:\d+:\d+\s+\d+:\d+:\d+)");
    QRegularExpressionMatch match = regex.match(exifDateString);
    if (!match.hasMatch()) {
        return;
    }
    const auto exifDateStringCaptured = match.captured(0);

    // Set exif date
    QDateTime exifDateTime = QDateTime::fromString(exifDateStringCaptured, "yyyy:MM:dd hh:mm:ss");
    _model.setData(_model.index(row, COL_EXIF_DATE), exifDateTime.toString(dateTimeFormat));
    _model.setData(_model.index(row, COL_EXIF_DATE), QColor(ExifDateColor), Qt::ForegroundRole);

    // Use modified date for video files
    if (videoFileExtensions.contains(QFileInfo(filePath).suffix()))
    {
        applyModifiedDate(row);
    }
    else
    {
        applyExifDate(row);
    }

    --_numLoadingFiles;
    _progressBar->setValue(_progressBar->maximum() - _numLoadingFiles);
    ui->tableView->resizeColumnsToContents();
    if (_numLoadingFiles == 0)
    {
        _progressBar->hide();
        ui->tableView->sortByColumn(COL_DATE, Qt::AscendingOrder);
        updateActions();
    }
}

void MainWindow::addFiles(const QStringList& filePaths)
{
    // Find new files
    QStringList newFiles;
    for (const auto& filePath: filePaths)
    {
        if (!_filePaths.contains(filePath))
        {
            _filePaths << filePath;
            newFiles << filePath;
        }
    }
    _numLoadingFiles = newFiles.count();
    if (_numLoadingFiles == 0)
        return;

    _progressBar->show();
    _progressBar->setRange(0, newFiles.count());
    _progressBar->setValue(0);

    // Start multi-threaded loading
    for (const auto& filePath: newFiles)
    {
        auto loader = new ExifLoaderThread(filePath);
        connect(loader, &ExifLoaderThread::resultReady, this, &MainWindow::onExifLoaded);
        QThreadPool::globalInstance()->start(loader);
    }
}

void MainWindow::onAdd()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("Open files"), ".",
                                                          "All files (*.*)");
    if(!filePaths.isEmpty())
        addFiles(filePaths);
}

void MainWindow::onDel()
{
    QList<int> rows;
    for (const QModelIndex& idx: getSelected())
        rows.append(idx.row());

    std::sort(std::begin(rows), std::end(rows), std::greater<int>());
    for (int row: rows)
    {
        _filePaths.remove(_model.data(_model.index(row, COL_FROM)).toString());
        _model.removeRow(row);
    }
}

void MainWindow::applyModifiedDate(int row)
{
    QString date = _model.data(_model.index(row, COL_MODIFIED_DATE)).toString();
    if (!date.isEmpty())
    {
        _model.setData(_model.index(row, COL_DATE), date);

        // Highlight when different
        if (_model.data(_model.index(row, COL_EXIF_DATE)).toString() != date)
        {
            _model.setData(_model.index(row, COL_DATE), QColor(ModifiedDateColor), Qt::ForegroundRole);
        }
    }
}

void MainWindow::applyExifDate(int row)
{
    QString date = _model.data(_model.index(row, COL_EXIF_DATE)).toString();
    if (!date.isEmpty())
    {
        _model.setData(_model.index(row, COL_DATE), date);

        // Highlight when different
        if (_model.data(_model.index(row, COL_MODIFIED_DATE)).toString() != date)
        {
            _model.setData(_model.index(row, COL_DATE), QColor(ExifDateColor), Qt::ForegroundRole);
        }
    }
}


void MainWindow::onUseModified()
{
    for (const QModelIndex& idx: getSelected())
    {
        applyModifiedDate(idx.row());
    }
}

void MainWindow::onUseExif()
{
    for (const QModelIndex& idx: getSelected())
    {
        applyExifDate(idx.row());
    }
}

/**
 * Pre-run the renaming
 * Put result under COL_TO without actually running it
 */
void MainWindow::preview()
{
    // collect input
    ui->tableView->sortByColumn(COL_DATE, Qt::AscendingOrder);

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
void MainWindow::onRename()
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
        else
            return;
    }

    // Acturally run renaming based on previewed results
    for(int row = 0; row < _model.rowCount(); ++row)
    {
        QString from = _model.data(_model.index(row, COL_FROM)).toString();
        QString to   = _model.data(_model.index(row, COL_TO))  .toString();
        if(to.isEmpty())
            continue;

        // Fix date
        QColor dateColor = _model.data(_model.index(row, COL_DATE), Qt::ForegroundRole).value<QColor>();
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
    _filePaths.clear();
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
                          "<p>Built on 05/14/2020</p>"
                          "<p><a href=mailto:CongChenUTD@Gmail.com>CongChenUTD@Gmail.com</a></p>"));
}

void MainWindow::onSelectionChanged(const QItemSelection& selection)
{
    ui->actionUseModified   ->setEnabled(!selection.isEmpty());
    ui->actionUseExif       ->setEnabled(!selection.isEmpty());
}

/**
 * EXIF date -> modified date
 */
void MainWindow::onFixDate()
{
    for(int row = 0; row < _model.rowCount(); ++row)
    {
        QString from = _model.data(_model.index(row, COL_FROM)).toString();
        QDateTime dateTime = _model.data(_model.index(row, COL_DATE)).toDateTime();
        QProcess::execute("touch", QStringList() << "-t" << dateTime.toString("yyyyMMddhhmm") << from);
    }
}

QModelIndexList MainWindow::getSelected() const {
    return ui->tableView->selectionModel()->selectedIndexes();
}

void MainWindow::updateActions()
{
    ui->actionEmpty  ->setEnabled(_model.rowCount() > 0);
    ui->actionRename ->setEnabled(_model.rowCount() > 0);
    ui->actionFixDate->setEnabled(_model.rowCount() > 0);
}
