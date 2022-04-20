#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "VideoProcess/videoprocess.h"

class QQmlApplicationEngine;

class VideoPlayerPrivate;
class VideoPlayer : public QObject
{
    Q_OBJECT

public:
    VideoPlayer(QObject *parent = nullptr);
    ~VideoPlayer();

    Q_INVOKABLE void openStream(const QString &path, const int &index);
    // @param progress bar value: 0 - 1
    Q_INVOKABLE void seek(const qreal &f);
    Q_INVOKABLE void playPause();
    Q_INVOKABLE void closeStream();

    Q_PROPERTY(bool isValid READ isValid NOTIFY playStatusChanged)
    bool isValid();
    Q_PROPERTY(bool playing READ playing NOTIFY playStatusChanged)
    bool playing();
    Q_PROPERTY(int playIndex READ playIndex NOTIFY playStatusChanged)
    int playIndex();

    Q_PROPERTY(QString currentTime READ currentTime NOTIFY frameUpdate)
    QString currentTime();
    Q_PROPERTY(QString totalTime READ totalTime NOTIFY frameUpdate)
    QString totalTime();
    Q_PROPERTY(qreal progress READ progress NOTIFY frameUpdate)
    qreal progress();

    Q_PROPERTY(QString frameUrl READ frameUrl NOTIFY frameUpdate)
    QString frameUrl();
    void setFrameUrl(QQmlApplicationEngine *e, const QString &path);

private:
    friend class VideoPlayerPrivate;
    QScopedPointer<VideoPlayerPrivate> p;

Q_SIGNALS:
    void playStatusChanged();
    void currentTimeChanged();
    void frameUpdate();
};

#endif // VIDEOPLAYER_H
