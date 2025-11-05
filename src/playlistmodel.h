#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractTableModel>
#include "metadata.h"

class PlaylistModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        ColumnTrack = 0,
        ColumnArtist,
        ColumnAlbum,
        ColumnTitle,
        ColumnDuration,
        ColumnCount
    };

    explicit PlaylistModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void addTrack(const Metadata &metadata);
    void addTracks(const QList<Metadata> &metadataList);
    void removeTrack(int row);
    void clear();

    Metadata getTrack(int row) const;
    QString getFilePath(int row) const;

private:
    QList<Metadata> playlist;
};

#endif // PLAYLISTMODEL_H
