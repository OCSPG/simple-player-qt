#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QTreeWidget>
#include <QMimeData>
#include <QDBusConnection>
#include <QSettings>
#include "playlistmodel.h"

class Mpris2;

// Custom tree widget that properly encodes file paths in MIME data
class FileExplorerTree : public QTreeWidget {
public:
    FileExplorerTree(QWidget *parent = nullptr) : QTreeWidget(parent) {}

protected:
    QMimeData* mimeData(const QList<QTreeWidgetItem*> &items) const override {
        QMimeData *mimeData = new QMimeData();
        QList<QUrl> urls;

        for (QTreeWidgetItem *item : items) {
            QString filePath = item->data(0, Qt::UserRole).toString();
            if (!filePath.isEmpty()) {
                urls.append(QUrl::fromLocalFile(filePath));
            }
        }

        if (!urls.isEmpty()) {
            mimeData->setUrls(urls);
        }

        return mimeData;
    }
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    enum RepeatMode {
        RepeatOff,
        RepeatAll,
        RepeatOne
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onPlayClicked();
    void onPauseClicked();
    void onStopClicked();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onSeek(int position);
    void onVolumeChanged(int volume);
    void onPlaylistDoubleClicked(const QModelIndex &index);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onAddToPlaylist();
    void onRemoveFromPlaylist();
    void onClearPlaylist();
    void onShuffleClicked();
    void onRepeatClicked();
    void onNextTrack();
    void onPreviousTrack();
    void onNavigateBack();
    void onNavigateForward();
    void onNavigateUp();
    void onPlaylistContextMenu(const QPoint &pos);

private:
    void setupUI();
    void connectSignals();
    QString formatTime(qint64 milliseconds);
    bool isAudioFile(const QString &filename);
    void updateShuffleButton();
    void updateRepeatButton();
    int getNextTrackIndex();
    void playTrackAtIndex(int index);
    void loadMetadataForFiles(const QStringList &files);
    void populateFileExplorer();
    void populateDirectoryTree(QTreeWidgetItem *parent, const QString &dirPath, int depth);
    void updateNavigationButtons();
    void navigateToPath(const QString &path);
    void setupMediaControls();
    void loadLastFolder();
    void saveLastFolder();
    void onPathClicked(const QString &path);

    QMediaPlayer *mediaPlayer;
    QAudioOutput *audioOutput;

    // Control buttons
    QPushButton *playButton;
    QPushButton *pauseButton;
    QPushButton *stopButton;
    QPushButton *nextButton;
    QPushButton *previousButton;
    QPushButton *shuffleButton;
    QPushButton *repeatButton;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *clearButton;

    // Sliders
    QSlider *positionSlider;
    QSlider *volumeSlider;
    QLabel *volumeIcon;

    // Labels (for status bar)
    QLabel *currentTimeLabel;
    QLabel *durationLabel;
    QLabel *nowPlayingLabel;

    // Playlist table
    QTableView *playlistTable;
    PlaylistModel *playlistModel;

    // File explorer
    QTreeWidget *fileExplorer;
    QPushButton *backButton;
    QPushButton *forwardButton;
    QPushButton *upButton;
    QWidget *currentPathWidget;

    QStringList pathHistory;
    int pathHistoryIndex;

    QList<int> shuffledIndices;
    int currentPlaylistIndex;
    bool isSeeking;
    bool shuffleEnabled;
    RepeatMode repeatMode;

    // For tracking drag operations from file explorer
    QStringList draggedFiles;

    // For storing original icon colors
    QIcon shuffleIconOriginal;
    QIcon repeatIconOriginal;

    Mpris2 *mpris2;
};

#endif // MAINWINDOW_H
