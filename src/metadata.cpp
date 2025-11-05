#include "metadata.h"
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QAudioDecoder>
#include <QFile>
#include <QFileInfo>
#include <QEventLoop>
#include <QUrl>

Metadata MetadataReader::readMetadata(const QString &filePath) {
    Metadata metadata;
    metadata.filePath = filePath;

    // Use QMediaPlayer to read metadata
    QMediaPlayer player;
    player.setSource(QUrl::fromLocalFile(filePath));

    // Wait for metadata to be loaded
    QEventLoop loop;
    QObject::connect(&player, &QMediaPlayer::durationChanged, &loop, &QEventLoop::quit);
    loop.exec();

    // Extract metadata
    metadata.title = player.metaData().value(QMediaMetaData::Title).toString();
    metadata.artist = player.metaData().value(QMediaMetaData::ContributingArtist).toString();
    metadata.album = player.metaData().value(QMediaMetaData::AlbumTitle).toString();
    metadata.genre = player.metaData().value(QMediaMetaData::Genre).toString();
    metadata.duration = player.duration();
    metadata.trackNumber = player.metaData().value(QMediaMetaData::TrackNumber).toInt();
    metadata.year = player.metaData().value(QMediaMetaData::Date).toString().left(4).toInt();

    // Use filename as fallback for title
    if (metadata.title.isEmpty()) {
        QFileInfo fileInfo(filePath);
        metadata.title = fileInfo.baseName();
    }

    if (metadata.artist.isEmpty()) {
        metadata.artist = "Unknown Artist";
    }

    if (metadata.album.isEmpty()) {
        metadata.album = "Unknown Album";
    }

    return metadata;
}
