#ifndef METADATA_H
#define METADATA_H

#include <QString>
#include <QVariant>

struct Metadata {
    QString filePath;
    QString title;
    QString artist;
    QString album;
    QString genre;
    qint64 duration;  // in milliseconds
    int trackNumber;
    int year;

    Metadata() : duration(0), trackNumber(0), year(0) {}
};

class MetadataReader {
public:
    static Metadata readMetadata(const QString &filePath);

private:
    MetadataReader() {}
};

#endif // METADATA_H
