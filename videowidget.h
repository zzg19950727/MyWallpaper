#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QVideoSurfaceFormat>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QtGui/QMovie>

class VideoWidget : public QVideoWidget
{
    Q_OBJECT
public:
    VideoWidget(QWidget* parent=0);
    //设置背景文件
    void setUrl(const QUrl &url);
    void play();
    void repair();
    void setMuted(bool);
    //保存壁纸路径
    void save();
    void read();
    void setWallPaperParent();

private slots:
    void stateChanged(QMediaPlayer::State);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);

private:
    //播放器
    QMediaPlayer mediaPlayer;
    qint64 currentDuration;
    //配置文件路径
    QString savePath;
    //壁纸路径
    QString wallpaperPath;
};

#endif // VIDEOWIDGET_H
