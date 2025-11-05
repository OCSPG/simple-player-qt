#include "mpris2.h"
#include "mainwindow.h"
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMetaType>
#include <QDebug>
#include <QCoreApplication>
#include <QMediaPlayer>
#include <cstdio>

// Mpris2RootAdaptor implementation
Mpris2RootAdaptor::Mpris2RootAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent) {
    setAutoRelaySignals(true);
}

void Mpris2RootAdaptor::Raise() {
    // Raise the main window
}

void Mpris2RootAdaptor::Quit() {
    QCoreApplication::quit();
}

// Mpris2PlayerAdaptor implementation
Mpris2PlayerAdaptor::Mpris2PlayerAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent), m_mainWindow(nullptr) {
    setAutoRelaySignals(true);
}

void Mpris2PlayerAdaptor::setMainWindow(MainWindow *mw) {
    m_mainWindow = mw;
}

void Mpris2PlayerAdaptor::Play() {
    if (m_mainWindow) {
        // For MPRIS2, Play should toggle play/pause when called from media keys
        QMetaObject::invokeMethod(m_mainWindow, "onPauseClicked", Qt::QueuedConnection);
    }
}

void Mpris2PlayerAdaptor::Pause() {
    if (m_mainWindow) {
        QMetaObject::invokeMethod(m_mainWindow, "onPauseClicked", Qt::QueuedConnection);
    }
}

void Mpris2PlayerAdaptor::PlayPause() {
    if (m_mainWindow) {
        QMetaObject::invokeMethod(m_mainWindow, "onPauseClicked", Qt::QueuedConnection);
    }
}

void Mpris2PlayerAdaptor::Stop() {
    if (m_mainWindow) {
        QMetaObject::invokeMethod(m_mainWindow, "onStopClicked", Qt::QueuedConnection);
    }
}

void Mpris2PlayerAdaptor::Next() {
    if (m_mainWindow) {
        QMetaObject::invokeMethod(m_mainWindow, "onNextTrack", Qt::QueuedConnection);
    }
}

void Mpris2PlayerAdaptor::Previous() {
    if (m_mainWindow) {
        QMetaObject::invokeMethod(m_mainWindow, "onPreviousTrack", Qt::QueuedConnection);
    }
}

void Mpris2PlayerAdaptor::Seek(qint64 Offset) {
    if (m_mainWindow) {
        // Seek by offset
    }
}

void Mpris2PlayerAdaptor::SetPosition(const QDBusObjectPath &TrackId, qint64 Position) {
    if (m_mainWindow) {
        // Set position
    }
}

void Mpris2PlayerAdaptor::OpenUri(const QString &Uri) {
    if (m_mainWindow) {
        // Open URI
    }
}

QString Mpris2PlayerAdaptor::PlaybackStatus() const {
    return "Stopped";
}

QVariantMap Mpris2PlayerAdaptor::Metadata() const {
    return QVariantMap();
}

double Mpris2PlayerAdaptor::Volume() const {
    return 0.5;
}

void Mpris2PlayerAdaptor::SetVolume(double volume) {
    if (m_mainWindow) {
        // Set volume
    }
}

qint64 Mpris2PlayerAdaptor::Position() const {
    return 0;
}

// Mpris2 implementation
Mpris2::Mpris2(MainWindow *mainWindow)
    : QObject(mainWindow), m_mainWindow(mainWindow), m_dbusConnection(QDBusConnection::sessionBus()) {

    // Create adaptors - these must be children of this object for ExportAdaptors to work
    m_rootAdaptor = new Mpris2RootAdaptor(this);
    m_playerAdaptor = new Mpris2PlayerAdaptor(this);
    m_playerAdaptor->setMainWindow(mainWindow);

    RegisterService();
}

Mpris2::~Mpris2() {
    UnregisterService();
}

bool Mpris2::RegisterService() {
    const QString serviceName = "org.mpris.MediaPlayer2.simpleplayerqt";
    const QString objectPath = "/org/mpris/MediaPlayer2";

    // Check if D-Bus connection is valid
    if (!m_dbusConnection.isConnected()) {
        qWarning() << "D-Bus session bus is not connected!";
        return false;
    }

    // Register the D-Bus object
    if (!m_dbusConnection.registerObject(objectPath, this, QDBusConnection::ExportAdaptors)) {
        qWarning() << "Failed to register D-Bus object at" << objectPath;
        return false;
    }

    // Register the D-Bus service
    if (!m_dbusConnection.registerService(serviceName)) {
        qWarning() << "Failed to register D-Bus service:" << serviceName;
        return false;
    }

    qDebug() << "Registered MPRIS2 service:" << serviceName;
    return true;
}

void Mpris2::UnregisterService() {
    const QString serviceName = "org.mpris.MediaPlayer2.simpleplayerqt";
    const QString objectPath = "/org/mpris/MediaPlayer2";
    m_dbusConnection.unregisterObject(objectPath);
    m_dbusConnection.unregisterService(serviceName);
}

void Mpris2::updateMetadata() {
    // Update metadata signals
}

void Mpris2::updatePlaybackStatus() {
    // Update playback status signals
}
