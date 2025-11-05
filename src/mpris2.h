#ifndef MPRIS2_H
#define MPRIS2_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QMediaPlayer>
#include <QVariantMap>

class MainWindow;

// MPRIS2 Root Interface Adaptor
class Mpris2RootAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")

public:
    Mpris2RootAdaptor(QObject *parent);

    Q_PROPERTY(bool CanQuit READ CanQuit)
    Q_PROPERTY(bool Fullscreen READ Fullscreen WRITE SetFullscreen)
    Q_PROPERTY(bool CanSetFullscreen READ CanSetFullscreen)
    Q_PROPERTY(bool CanRaise READ CanRaise)
    Q_PROPERTY(bool HasTrackList READ HasTrackList)
    Q_PROPERTY(QString Identity READ Identity)
    Q_PROPERTY(QString DesktopEntry READ DesktopEntry)
    Q_PROPERTY(QStringList SupportedUriSchemes READ SupportedUriSchemes)
    Q_PROPERTY(QStringList SupportedMimeTypes READ SupportedMimeTypes)

public slots:
    void Raise();
    void Quit();

private:
    bool CanQuit() const { return true; }
    bool Fullscreen() const { return false; }
    void SetFullscreen(bool) {}
    bool CanSetFullscreen() const { return false; }
    bool CanRaise() const { return true; }
    bool HasTrackList() const { return false; }
    QString Identity() const { return "SimplePlayerQt"; }
    QString DesktopEntry() const { return "simpleplayerqt"; }
    QStringList SupportedUriSchemes() const { return {"file"}; }
    QStringList SupportedMimeTypes() const {
        return {"audio/mpeg", "audio/flac", "audio/wav", "audio/ogg", "audio/aac"};
    }
};

// MPRIS2 Player Interface Adaptor
class Mpris2PlayerAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

public:
    Mpris2PlayerAdaptor(QObject *parent);
    void setMainWindow(MainWindow *mw);

    Q_PROPERTY(QString PlaybackStatus READ PlaybackStatus)
    Q_PROPERTY(double Rate READ Rate WRITE SetRate)
    Q_PROPERTY(QVariantMap Metadata READ Metadata)
    Q_PROPERTY(double Volume READ Volume WRITE SetVolume)
    Q_PROPERTY(qint64 Position READ Position)
    Q_PROPERTY(double MinimumRate READ MinimumRate)
    Q_PROPERTY(double MaximumRate READ MaximumRate)
    Q_PROPERTY(bool CanGoNext READ CanGoNext)
    Q_PROPERTY(bool CanGoPrevious READ CanGoPrevious)
    Q_PROPERTY(bool CanPlay READ CanPlay)
    Q_PROPERTY(bool CanPause READ CanPause)
    Q_PROPERTY(bool CanSeek READ CanSeek)
    Q_PROPERTY(bool CanControl READ CanControl)

signals:
    void Seeked(qint64 Position);

public slots:
    void Play();
    void Pause();
    void PlayPause();
    void Stop();
    void Next();
    void Previous();
    void Seek(qint64 Offset);
    void SetPosition(const QDBusObjectPath &TrackId, qint64 Position);
    void OpenUri(const QString &Uri);

private:
    QString PlaybackStatus() const;
    double Rate() const { return 1.0; }
    void SetRate(double) {}
    QVariantMap Metadata() const;
    double Volume() const;
    void SetVolume(double volume);
    qint64 Position() const;
    double MinimumRate() const { return 1.0; }
    double MaximumRate() const { return 1.0; }
    bool CanGoNext() const { return true; }
    bool CanGoPrevious() const { return true; }
    bool CanPlay() const { return true; }
    bool CanPause() const { return true; }
    bool CanSeek() const { return true; }
    bool CanControl() const { return true; }

    MainWindow *m_mainWindow;
};

class Mpris2 : public QObject {
    Q_OBJECT

public:
    Mpris2(MainWindow *mainWindow);
    ~Mpris2();

    void updateMetadata();
    void updatePlaybackStatus();

private:
    MainWindow *m_mainWindow;
    QDBusConnection m_dbusConnection;
    Mpris2RootAdaptor *m_rootAdaptor;
    Mpris2PlayerAdaptor *m_playerAdaptor;

    bool RegisterService();
    void UnregisterService();
};

#endif // MPRIS2_H
