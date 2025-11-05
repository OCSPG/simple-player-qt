#include "mainwindow.h"
#include "metadata.h"
#include "mpris2.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QTableView>
#include <QHeaderView>
#include <QRandomGenerator>
#include <QThreadPool>
#include <QRunnable>
#include <QApplication>
#include <QStyle>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QFileSystemModel>
#include <QStandardPaths>
#include <QDir>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QFont>
#include <QMenu>
#include <QPainter>
#include <QSettings>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentPlaylistIndex(0), isSeeking(false),
      shuffleEnabled(false), repeatMode(RepeatOff) {
    setWindowTitle("Music Player");
    setGeometry(100, 100, 1100, 700);

    mediaPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    mediaPlayer->setAudioOutput(audioOutput);

    setupUI();
    connectSignals();
    setupMediaControls();
    loadLastFolder();

    // Initialize MPRIS2 for system media control integration
    mpris2 = new Mpris2(this);
}

MainWindow::~MainWindow() {
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) {
        return;
    }

    switch (event->key()) {
    case Qt::Key_Space:
        if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
            onPauseClicked();
        } else {
            onPlayClicked();
        }
        break;
    case Qt::Key_N:
        onNextTrack();
        break;
    case Qt::Key_P:
        onPreviousTrack();
        break;
    case Qt::Key_S:
        onShuffleClicked();
        break;
    case Qt::Key_R:
        onRepeatClicked();
        break;
    case Qt::Key_Right:
        // Seek forward 5 seconds
        mediaPlayer->setPosition(mediaPlayer->position() + 5000);
        break;
    case Qt::Key_Left:
        // Seek backward 5 seconds
        mediaPlayer->setPosition(mediaPlayer->position() - 5000);
        break;
    case Qt::Key_Up:
        // Volume up
        volumeSlider->setValue(volumeSlider->value() + 5);
        break;
    case Qt::Key_Down:
        // Volume down
        volumeSlider->setValue(volumeSlider->value() - 5);
        break;
    default:
        QMainWindow::keyPressEvent(event);
        break;
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == fileExplorer) {
        // Handle drag start from tree widget
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                QTreeWidgetItem *item = fileExplorer->itemAt(mouseEvent->pos());
                if (item) {
                    // Store the item being dragged
                    draggedFiles.clear();
                    QString filePath = item->data(0, Qt::UserRole).toString();
                    if (!filePath.isEmpty() && !QFileInfo(filePath).isDir() && isAudioFile(filePath)) {
                        draggedFiles.append(filePath);
                    }
                }
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            // Clear on mouse release - drag didn't complete to playlist
            // Don't clear here, let drop event handle it
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    // Accept any drop with URI list (from file manager or custom tree widget)
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return;
    }

    // Accept drags from internal file explorer if we have stored files
    if (!draggedFiles.isEmpty()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();
    QStringList files;

    // Check if files were dragged from internal file explorer event filter
    if (!draggedFiles.isEmpty()) {
        files = draggedFiles;
        draggedFiles.clear();
    }
    // Check for URI list (from file manager or custom tree widget)
    else if (mimeData->hasUrls()) {
        for (const QUrl &url : mimeData->urls()) {
            QString path = url.toLocalFile();
            if (!path.isEmpty()) {
                QFileInfo fileInfo(path);
                if (fileInfo.isFile() && isAudioFile(path)) {
                    files.append(path);
                }
            }
        }
    }

    if (!files.isEmpty()) {
        loadMetadataForFiles(files);
        nowPlayingLabel->setText(QString("Added %1 track(s) from drag & drop").arg(files.size()));
        event->acceptProposedAction();
    }
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // Status bar (now playing info and progress) at top
    QWidget *statusWidget = new QWidget(this);
    QVBoxLayout *statusLayout = new QVBoxLayout(statusWidget);
    statusLayout->setContentsMargins(5, 3, 5, 3);
    statusLayout->setSpacing(3);

    // Now Playing Info
    nowPlayingLabel = new QLabel("No track playing", this);
    nowPlayingLabel->setStyleSheet("font-size: 11px; font-weight: bold;");
    statusLayout->addWidget(nowPlayingLabel);

    // Progress bar with time labels
    QHBoxLayout *progressLayout = new QHBoxLayout();
    currentTimeLabel = new QLabel("0:00", this);
    currentTimeLabel->setMinimumWidth(40);
    positionSlider = new QSlider(Qt::Horizontal, this);
    positionSlider->setRange(0, 0);
    positionSlider->setMinimumHeight(16);
    durationLabel = new QLabel("0:00", this);
    durationLabel->setMinimumWidth(40);

    progressLayout->addWidget(currentTimeLabel);
    progressLayout->addWidget(positionSlider);
    progressLayout->addWidget(durationLabel);
    statusLayout->addLayout(progressLayout);

    mainLayout->addWidget(statusWidget);

    // Top control bar
    QHBoxLayout *topControlLayout = new QHBoxLayout();
    QStyle *style = QApplication::style();

    previousButton = new QPushButton(style->standardIcon(QStyle::SP_MediaSkipBackward), "", this);
    playButton = new QPushButton(style->standardIcon(QStyle::SP_MediaPlay), "", this);
    pauseButton = new QPushButton(style->standardIcon(QStyle::SP_MediaPause), "", this);
    stopButton = new QPushButton(style->standardIcon(QStyle::SP_MediaStop), "", this);
    nextButton = new QPushButton(style->standardIcon(QStyle::SP_MediaSkipForward), "", this);

    QSize iconSize(28, 28);
    previousButton->setIconSize(iconSize);
    playButton->setIconSize(iconSize);
    pauseButton->setIconSize(iconSize);
    stopButton->setIconSize(iconSize);
    nextButton->setIconSize(iconSize);

    // Set tooltips with keyboard shortcuts
    previousButton->setToolTip("Previous (P)");
    playButton->setToolTip("Play (Space)");
    pauseButton->setToolTip("Pause");
    stopButton->setToolTip("Stop");
    nextButton->setToolTip("Next (N)");

    previousButton->setFlat(true);
    playButton->setFlat(true);
    pauseButton->setFlat(true);
    stopButton->setFlat(true);
    nextButton->setFlat(true);

    topControlLayout->addWidget(previousButton);
    topControlLayout->addWidget(playButton);
    topControlLayout->addWidget(pauseButton);
    topControlLayout->addWidget(stopButton);
    topControlLayout->addWidget(nextButton);
    topControlLayout->addSpacing(20);

    // Mode buttons with icons (state indicated by icon color)
    shuffleButton = new QPushButton(this);
    shuffleButton->setCheckable(true);
    QIcon shuffleIcon = QIcon::fromTheme("media-playlist-shuffle");
    if (shuffleIcon.isNull()) {
        shuffleIcon = QIcon::fromTheme("media-shuffle", style->standardIcon(QStyle::SP_MediaSkipForward));
    }
    shuffleIconOriginal = shuffleIcon;
    shuffleButton->setIcon(shuffleIcon);
    shuffleButton->setIconSize(QSize(20, 20));
    shuffleButton->setToolTip("Toggle Shuffle (S)");
    shuffleButton->setFlat(true);

    repeatButton = new QPushButton(this);
    QIcon repeatIcon = QIcon::fromTheme("media-playlist-repeat");
    if (repeatIcon.isNull()) {
        repeatIcon = QIcon::fromTheme("media-repeat", style->standardIcon(QStyle::SP_MediaPlay));
    }
    repeatIconOriginal = repeatIcon;
    repeatButton->setIcon(repeatIcon);
    repeatButton->setIconSize(QSize(20, 20));
    repeatButton->setToolTip("Cycle Repeat Mode (R)");
    repeatButton->setFlat(true);

    topControlLayout->addWidget(shuffleButton);
    topControlLayout->addWidget(repeatButton);
    topControlLayout->addSpacing(30);

    // Volume control in top bar with dynamic icon
    volumeIcon = new QLabel(this);
    volumeIcon->setPixmap(style->standardIcon(QStyle::SP_MediaVolume).pixmap(20, 20));

    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(70);
    volumeSlider->setMaximumWidth(120);

    topControlLayout->addWidget(volumeIcon);
    topControlLayout->addWidget(volumeSlider);
    topControlLayout->addStretch();

    mainLayout->addLayout(topControlLayout);

    // Splitter for file explorer and playlist
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    // File explorer widget with navigation
    QWidget *explorerWidget = new QWidget(this);
    QVBoxLayout *explorerLayout = new QVBoxLayout(explorerWidget);
    explorerLayout->setContentsMargins(0, 0, 0, 0);
    explorerLayout->setSpacing(3);

    // Navigation buttons
    QHBoxLayout *navLayout = new QHBoxLayout();
    backButton = new QPushButton(style->standardIcon(QStyle::SP_ArrowLeft), "", this);
    forwardButton = new QPushButton(style->standardIcon(QStyle::SP_ArrowRight), "", this);

    upButton = new QPushButton(style->standardIcon(QStyle::SP_ArrowUp), "", this);

    backButton->setToolTip("Back");
    forwardButton->setToolTip("Forward");
    upButton->setToolTip("Up");

    backButton->setIconSize(QSize(20, 20));
    forwardButton->setIconSize(QSize(20, 20));
    upButton->setIconSize(QSize(20, 20));

    backButton->setFlat(true);
    forwardButton->setFlat(true);
    upButton->setFlat(true);

    backButton->setMaximumWidth(35);
    forwardButton->setMaximumWidth(35);
    upButton->setMaximumWidth(35);

    navLayout->addWidget(backButton);
    navLayout->addWidget(forwardButton);
    navLayout->addWidget(upButton);
    navLayout->addStretch();

    explorerLayout->addLayout(navLayout);

    // Add path display widget with breadcrumb buttons
    currentPathWidget = new QWidget(this);
    QHBoxLayout *pathLayout = new QHBoxLayout(currentPathWidget);
    pathLayout->setContentsMargins(3, 2, 3, 2);
    pathLayout->setSpacing(0);
    pathLayout->addStretch();
    currentPathWidget->setMaximumHeight(28);
    explorerLayout->addWidget(currentPathWidget);

    // File explorer pane with custom tree widget for proper MIME data handling
    fileExplorer = new FileExplorerTree(this);
    fileExplorer->setHeaderLabel("Files");
    fileExplorer->setColumnCount(1);
    fileExplorer->setDragEnabled(true);
    fileExplorer->setDefaultDropAction(Qt::CopyAction);
    fileExplorer->installEventFilter(this);  // Install event filter for drag tracking
    pathHistory.append(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    pathHistoryIndex = 0;
    populateFileExplorer();
    explorerLayout->addWidget(fileExplorer);

    splitter->addWidget(explorerWidget);

    // Playlist table
    playlistModel = new PlaylistModel(this);
    playlistTable = new QTableView(this);
    playlistTable->setModel(playlistModel);
    playlistTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    playlistTable->setSelectionMode(QAbstractItemView::SingleSelection);
    playlistTable->setAlternatingRowColors(true);
    playlistTable->setColumnWidth(PlaylistModel::ColumnTrack, 35);
    playlistTable->setColumnWidth(PlaylistModel::ColumnArtist, 150);
    playlistTable->setColumnWidth(PlaylistModel::ColumnAlbum, 150);
    playlistTable->setColumnWidth(PlaylistModel::ColumnTitle, 250);
    playlistTable->setColumnWidth(PlaylistModel::ColumnDuration, 60);
    playlistTable->horizontalHeader()->setStretchLastSection(false);
    playlistTable->setShowGrid(false);
    playlistTable->setAcceptDrops(true);
    playlistTable->setDropIndicatorShown(true);
    playlistTable->setDefaultDropAction(Qt::CopyAction);
    splitter->addWidget(playlistTable);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    mainLayout->addWidget(splitter, 1);
}

void MainWindow::connectSignals() {
    // Playback controls
    connect(playButton, &QPushButton::clicked, this, &MainWindow::onPlayClicked);
    connect(pauseButton, &QPushButton::clicked, this, &MainWindow::onPauseClicked);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::onNextTrack);
    connect(previousButton, &QPushButton::clicked, this, &MainWindow::onPreviousTrack);

    // Media player signals
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &MainWindow::onPositionChanged);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &MainWindow::onDurationChanged);
    connect(mediaPlayer, QOverload<QMediaPlayer::MediaStatus>::of(&QMediaPlayer::mediaStatusChanged),
            this, &MainWindow::onMediaStatusChanged);

    // Sliders
    connect(positionSlider, &QSlider::sliderMoved, this, &MainWindow::onSeek);
    connect(volumeSlider, &QSlider::valueChanged, this, &MainWindow::onVolumeChanged);

    // Playlist
    connect(playlistTable, &QTableView::doubleClicked, this, &MainWindow::onPlaylistDoubleClicked);
    connect(playlistTable, &QTableView::customContextMenuRequested, this, &MainWindow::onPlaylistContextMenu);
    playlistTable->setContextMenuPolicy(Qt::CustomContextMenu);

    // Mode controls
    connect(shuffleButton, &QPushButton::clicked, this, &MainWindow::onShuffleClicked);
    connect(repeatButton, &QPushButton::clicked, this, &MainWindow::onRepeatClicked);

    // File explorer double click to add to playlist or navigate
    connect(fileExplorer, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item) {
        QString filePath = item->data(0, Qt::UserRole).toString();
        if (!filePath.isEmpty()) {
            QFileInfo fileInfo(filePath);
            if (fileInfo.isDir()) {
                navigateToPath(filePath);
            } else if (isAudioFile(filePath)) {
                Metadata metadata = MetadataReader::readMetadata(filePath);
                playlistModel->addTrack(metadata);
                nowPlayingLabel->setText("Added: " + fileInfo.fileName());
            }
        }
    });

    // Navigation controls
    connect(backButton, &QPushButton::clicked, this, &MainWindow::onNavigateBack);
    connect(forwardButton, &QPushButton::clicked, this, &MainWindow::onNavigateForward);
    connect(upButton, &QPushButton::clicked, this, &MainWindow::onNavigateUp);
}

bool MainWindow::isAudioFile(const QString &filename) {
    QStringList audioExtensions = {".mp3", ".flac", ".ogg", ".wav", ".m4a", ".aac", ".wma"};
    return std::any_of(audioExtensions.begin(), audioExtensions.end(),
                      [&filename](const QString &ext) {
                          return filename.endsWith(ext, Qt::CaseInsensitive);
                      });
}

void MainWindow::loadMetadataForFiles(const QStringList &files) {
    for (const QString &file : files) {
        if (isAudioFile(file)) {
            Metadata metadata = MetadataReader::readMetadata(file);
            playlistModel->addTrack(metadata);
        }
    }
}

void MainWindow::onAddToPlaylist() {
    QStringList files = QFileDialog::getOpenFileNames(this,
        "Add Music Files", "",
        "Music Files (*.mp3 *.flac *.ogg *.wav *.m4a *.aac *.wma);;All Files (*)");

    if (!files.isEmpty()) {
        loadMetadataForFiles(files);
        nowPlayingLabel->setText(QString("Loaded %1 tracks").arg(files.size()));
    }
}

void MainWindow::onRemoveFromPlaylist() {
    int row = playlistTable->currentIndex().row();
    if (row >= 0) {
        playlistModel->removeTrack(row);

        if (row == currentPlaylistIndex && row < playlistModel->rowCount()) {
            playTrackAtIndex(row);
        } else if (currentPlaylistIndex >= playlistModel->rowCount()) {
            currentPlaylistIndex = playlistModel->rowCount() - 1;
        }
    }
}

void MainWindow::onClearPlaylist() {
    playlistModel->clear();
    mediaPlayer->stop();
    currentPlaylistIndex = 0;
    shuffledIndices.clear();
    nowPlayingLabel->setText("No track playing");
    nowPlayingLabel->setText("Playlist cleared");
}

void MainWindow::onPlayClicked() {
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        return;
    }

    if (playlistModel->rowCount() == 0) {
        nowPlayingLabel->setText("No tracks in playlist");
        return;
    }

    if (mediaPlayer->source().isEmpty()) {
        playTrackAtIndex(0);
    } else {
        mediaPlayer->play();
    }
}

void MainWindow::onPauseClicked() {
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        mediaPlayer->pause();
    } else if (mediaPlayer->playbackState() == QMediaPlayer::PausedState) {
        mediaPlayer->play();
    }
}

void MainWindow::onStopClicked() {
    mediaPlayer->stop();
    positionSlider->setValue(0);
}

void MainWindow::onNextTrack() {
    if (playlistModel->rowCount() == 0) return;
    int nextIndex = getNextTrackIndex();
    if (nextIndex >= 0) {
        playTrackAtIndex(nextIndex);
    }
}

void MainWindow::onPreviousTrack() {
    if (playlistModel->rowCount() == 0) return;

    currentPlaylistIndex = (currentPlaylistIndex - 1 + playlistModel->rowCount()) % playlistModel->rowCount();
    playTrackAtIndex(currentPlaylistIndex);
}

void MainWindow::onPositionChanged(qint64 position) {
    if (!isSeeking) {
        positionSlider->blockSignals(true);
        positionSlider->setValue(position);
        positionSlider->blockSignals(false);
    }
    currentTimeLabel->setText(formatTime(position));
}

void MainWindow::onDurationChanged(qint64 duration) {
    positionSlider->setRange(0, duration);
    durationLabel->setText(formatTime(duration));
}

void MainWindow::onSeek(int position) {
    isSeeking = true;
    mediaPlayer->setPosition(position);
    isSeeking = false;
}

void MainWindow::onVolumeChanged(int volume) {
    audioOutput->setVolume(volume / 100.0);

    // Update volume icon based on volume level
    QStyle *style = QApplication::style();
    QIcon volumeIconToUse;

    if (volume == 0) {
        volumeIconToUse = style->standardIcon(QStyle::SP_MediaVolumeMuted);
    } else if (volume <= 33) {
        // Low volume - use muted icon
        volumeIconToUse = style->standardIcon(QStyle::SP_MediaVolumeMuted);
    } else if (volume <= 66) {
        // Medium volume - use volume icon
        volumeIconToUse = style->standardIcon(QStyle::SP_MediaVolume);
    } else {
        // High volume - use volume icon
        volumeIconToUse = style->standardIcon(QStyle::SP_MediaVolume);
    }

    volumeIcon->setPixmap(volumeIconToUse.pixmap(20, 20));
}

void MainWindow::onPlaylistDoubleClicked(const QModelIndex &index) {
    if (index.isValid()) {
        playTrackAtIndex(index.row());
    }
}

void MainWindow::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::EndOfMedia) {
        if (repeatMode == RepeatOne) {
            mediaPlayer->setPosition(0);
            mediaPlayer->play();
        } else {
            onNextTrack();
        }
    }
}

void MainWindow::onShuffleClicked() {
    shuffleEnabled = !shuffleEnabled;
    updateShuffleButton();

    if (shuffleEnabled) {
        shuffledIndices.clear();
        for (int i = 0; i < playlistModel->rowCount(); ++i) {
            shuffledIndices.append(i);
        }
        for (int i = shuffledIndices.size() - 1; i > 0; --i) {
            int j = QRandomGenerator::global()->bounded(i + 1);
            shuffledIndices.swapItemsAt(i, j);
        }
    } else {
        shuffledIndices.clear();
    }
}

void MainWindow::onRepeatClicked() {
    repeatMode = (RepeatMode)((repeatMode + 1) % 3);
    updateRepeatButton();
}

int MainWindow::getNextTrackIndex() {
    if (playlistModel->rowCount() == 0) return -1;

    if (shuffleEnabled) {
        int currentPos = shuffledIndices.indexOf(currentPlaylistIndex);
        if (currentPos >= 0 && currentPos < shuffledIndices.size() - 1) {
            currentPlaylistIndex = shuffledIndices[currentPos + 1];
            return currentPlaylistIndex;
        } else if (repeatMode == RepeatAll) {
            currentPlaylistIndex = shuffledIndices[0];
            return currentPlaylistIndex;
        }
        return -1;
    } else {
        if (currentPlaylistIndex < playlistModel->rowCount() - 1) {
            return ++currentPlaylistIndex;
        } else if (repeatMode == RepeatAll) {
            currentPlaylistIndex = 0;
            return 0;
        }
        return -1;
    }
}

void MainWindow::playTrackAtIndex(int index) {
    if (index >= 0 && index < playlistModel->rowCount()) {
        currentPlaylistIndex = index;
        QString filePath = playlistModel->getFilePath(index);
        Metadata metadata = playlistModel->getTrack(index);

        mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
        playlistTable->selectRow(index);
        playlistTable->scrollTo(playlistTable->currentIndex(), QAbstractItemView::PositionAtCenter);

        nowPlayingLabel->setText(QString("Now Playing: %1 - %2")
                                    .arg(metadata.artist)
                                    .arg(metadata.title));
        mediaPlayer->play();
    }
}

void MainWindow::updateShuffleButton() {
    if (shuffleEnabled) {
        // Create a colored version of the icon
        QPixmap pixmap(20, 20);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);

        // Draw the icon in blue
        QIcon coloredIcon = shuffleIconOriginal;
        QPalette pal;
        pal.setColor(QPalette::ButtonText, QColor(100, 150, 200));

        shuffleButton->setIcon(shuffleIconOriginal);
        shuffleButton->setStyleSheet("QPushButton { color: #6496C8; }");
    } else {
        shuffleButton->setIcon(shuffleIconOriginal);
        shuffleButton->setStyleSheet("");
    }
}

void MainWindow::updateRepeatButton() {
    switch (repeatMode) {
    case RepeatOff:
        repeatButton->setIcon(repeatIconOriginal);
        repeatButton->setStyleSheet("");
        break;
    case RepeatAll:
        repeatButton->setIcon(repeatIconOriginal);
        repeatButton->setStyleSheet("QPushButton { color: #6496C8; }");
        break;
    case RepeatOne:
        repeatButton->setIcon(repeatIconOriginal);
        repeatButton->setStyleSheet("QPushButton { color: #6496C8; }");
        break;
    }
}

QString MainWindow::formatTime(qint64 milliseconds) {
    qint64 seconds = milliseconds / 1000;
    qint64 minutes = seconds / 60;
    seconds %= 60;
    return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
}

void MainWindow::onNavigateBack() {
    if (pathHistoryIndex > 0) {
        pathHistoryIndex--;
        populateFileExplorer();
    }
}

void MainWindow::onNavigateForward() {
    if (pathHistoryIndex < pathHistory.size() - 1) {
        pathHistoryIndex++;
        populateFileExplorer();
    }
}

void MainWindow::onNavigateUp() {
    QString currentPath = pathHistory[pathHistoryIndex];
    QDir dir(currentPath);
    if (dir.cdUp()) {
        navigateToPath(dir.absolutePath());
    }
}

void MainWindow::onPlaylistContextMenu(const QPoint &pos) {
    QModelIndex index = playlistTable->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    QMenu contextMenu(this);
    QAction *removeAction = contextMenu.addAction("Remove");
    contextMenu.addSeparator();
    QAction *clearAction = contextMenu.addAction("Clear All");

    QAction *selectedAction = contextMenu.exec(playlistTable->mapToGlobal(pos));

    if (selectedAction == removeAction) {
        playlistModel->removeTrack(index.row());
        if (index.row() == currentPlaylistIndex && index.row() < playlistModel->rowCount()) {
            playTrackAtIndex(index.row());
        } else if (currentPlaylistIndex >= playlistModel->rowCount()) {
            currentPlaylistIndex = playlistModel->rowCount() - 1;
        }
    } else if (selectedAction == clearAction) {
        onClearPlaylist();
    }
}

void MainWindow::populateFileExplorer() {
    fileExplorer->clear();
    QString currentPath = pathHistory[pathHistoryIndex];
    populateDirectoryTree(fileExplorer->invisibleRootItem(), currentPath, 0);
    updateNavigationButtons();

    // Update the path display with clickable breadcrumbs
    // Clear previous layout items except stretch
    QHBoxLayout *pathLayout = qobject_cast<QHBoxLayout*>(currentPathWidget->layout());
    if (!pathLayout) return;

    while (pathLayout->count() > 1) {  // Keep the last stretch item
        QLayoutItem *item = pathLayout->takeAt(0);
        if (item && item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Split path and create clickable buttons
    QStringList pathComponents = currentPath.split('/', Qt::SkipEmptyParts);
    if (pathComponents.isEmpty()) {
        pathComponents << "/";
    }

    QString accumulatedPath;
    for (int i = 0; i < pathComponents.size(); ++i) {
        if (i > 0) {
            QLabel *separator = new QLabel("/");
            separator->setStyleSheet("margin: 0px; padding: 0px 2px;");
            pathLayout->insertWidget(pathLayout->count() - 1, separator);
        }

        QString component = pathComponents[i];
        if (component == "/") {
            accumulatedPath = "/";
        } else {
            if (accumulatedPath != "/" && !accumulatedPath.endsWith("/")) {
                accumulatedPath += "/";
            }
            accumulatedPath += component;
        }

        QPushButton *pathButton = new QPushButton(component, this);
        pathButton->setFlat(true);
        pathButton->setStyleSheet("QPushButton { color: #6496C8; padding: 0px 2px; margin: 0px; border: none; font-size: 13px; } QPushButton:hover { text-decoration: underline; }");
        pathButton->setCursor(Qt::PointingHandCursor);
        pathButton->setMaximumHeight(22);
        pathButton->setMaximumWidth(150);
        pathButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        pathButton->setToolTip(accumulatedPath);
        QString pathToNavigate = accumulatedPath;
        connect(pathButton, &QPushButton::clicked, this, [this, pathToNavigate]() {
            onPathClicked(pathToNavigate);
        });
        pathLayout->insertWidget(pathLayout->count() - 1, pathButton);
    }
}

void MainWindow::populateDirectoryTree(QTreeWidgetItem *parent, const QString &dirPath, int depth) {
    // Limit depth to avoid too deep recursion
    if (depth > 2) return;

    QDir dir(dirPath);
    dir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

    QFileInfoList entries = dir.entryInfoList();

    for (const QFileInfo &fileInfo : entries) {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, fileInfo.fileName());
        item->setData(0, Qt::UserRole, fileInfo.filePath());

        if (fileInfo.isDir()) {
            // Add directory
            populateDirectoryTree(item, fileInfo.filePath(), depth + 1);
        } else if (isAudioFile(fileInfo.filePath())) {
            // Mark audio files with icon or styling
            item->setText(0, fileInfo.fileName());
        }
    }
}

void MainWindow::updateNavigationButtons() {
    backButton->setEnabled(pathHistoryIndex > 0);
    forwardButton->setEnabled(pathHistoryIndex < pathHistory.size() - 1);
    upButton->setEnabled(pathHistory[pathHistoryIndex] != "/");
}

void MainWindow::navigateToPath(const QString &path) {
    // Remove any forward history when navigating to a new path
    while (pathHistoryIndex < pathHistory.size() - 1) {
        pathHistory.removeLast();
    }

    pathHistory.append(path);
    pathHistoryIndex++;
    populateFileExplorer();
    saveLastFolder();
}

void MainWindow::setupMediaControls() {
    // Register for media key events from the system (KDE, GNOME, etc.)
    // This allows keyboard media keys and system media controls to work

    QDBusConnection dbusConnection = QDBusConnection::sessionBus();

    // Register the application to receive media control signals
    // This is typically handled through MPRIS2, but we can also use KDE's native signals

    // For KDE Plasma, we listen to media control signals
    // Note: Direct MPRIS2 implementation would require more complex D-Bus setup
    // For now, Qt's QMediaPlayer already integrates with system media keys on most desktop environments

    // Ensure the media player responds to system signals
    // This is already handled by Qt6's multimedia integration with the desktop
}

void MainWindow::loadLastFolder() {
    QSettings settings("SimplePlayerQt", "SimplePlayerQt");
    QString lastFolder = settings.value("lastFolder", QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toString();

    // Verify the folder exists, otherwise use home directory
    if (!QDir(lastFolder).exists()) {
        lastFolder = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }

    pathHistory.clear();
    pathHistory.append(lastFolder);
    pathHistoryIndex = 0;
    populateFileExplorer();
}

void MainWindow::saveLastFolder() {
    if (pathHistoryIndex >= 0 && pathHistoryIndex < pathHistory.size()) {
        QSettings settings("SimplePlayerQt", "SimplePlayerQt");
        settings.setValue("lastFolder", pathHistory[pathHistoryIndex]);
        settings.sync();
    }
}

void MainWindow::onPathClicked(const QString &path) {
    // Navigate to the clicked path component
    // Remove any forward history when navigating to a new path
    while (pathHistoryIndex < pathHistory.size() - 1) {
        pathHistory.removeLast();
    }

    pathHistory.append(path);
    pathHistoryIndex++;
    populateFileExplorer();
    saveLastFolder();
}
