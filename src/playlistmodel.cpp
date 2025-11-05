#include "playlistmodel.h"

PlaylistModel::PlaylistModel(QObject *parent)
    : QAbstractTableModel(parent) {
}

int PlaylistModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return playlist.size();
}

int PlaylistModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return ColumnCount;
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= playlist.size())
        return QVariant();

    const Metadata &metadata = playlist[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColumnTrack:
            return metadata.trackNumber > 0 ? QString::number(metadata.trackNumber) : "-";
        case ColumnArtist:
            return metadata.artist;
        case ColumnAlbum:
            return metadata.album;
        case ColumnTitle:
            return metadata.title;
        case ColumnDuration: {
            qint64 seconds = metadata.duration / 1000;
            qint64 minutes = seconds / 60;
            seconds %= 60;
            return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
        }
        default:
            return QVariant();
        }
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == ColumnTrack || index.column() == ColumnDuration) {
            return Qt::AlignCenter;
        }
    }

    return QVariant();
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case ColumnTrack:
            return "#";
        case ColumnArtist:
            return "Artist";
        case ColumnAlbum:
            return "Album";
        case ColumnTitle:
            return "Title";
        case ColumnDuration:
            return "Duration";
        default:
            return QVariant();
        }
    }
    return QVariant();
}

void PlaylistModel::addTrack(const Metadata &metadata) {
    beginInsertRows(QModelIndex(), playlist.size(), playlist.size());
    playlist.append(metadata);
    endInsertRows();
}

void PlaylistModel::addTracks(const QList<Metadata> &metadataList) {
    beginInsertRows(QModelIndex(), playlist.size(), playlist.size() + metadataList.size() - 1);
    playlist.append(metadataList);
    endInsertRows();
}

void PlaylistModel::removeTrack(int row) {
    if (row >= 0 && row < playlist.size()) {
        beginRemoveRows(QModelIndex(), row, row);
        playlist.removeAt(row);
        endRemoveRows();
    }
}

void PlaylistModel::clear() {
    beginResetModel();
    playlist.clear();
    endResetModel();
}

Metadata PlaylistModel::getTrack(int row) const {
    if (row >= 0 && row < playlist.size()) {
        return playlist[row];
    }
    return Metadata();
}

QString PlaylistModel::getFilePath(int row) const {
    if (row >= 0 && row < playlist.size()) {
        return playlist[row].filePath;
    }
    return QString();
}
