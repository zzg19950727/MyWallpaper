#include "videowidget.h"
#include <windows.h>
#include <QIODevice>
#include <QRect>
#include <QFile>

//枚举窗口寻找桌面图标窗口
inline BOOL CALLBACK EnumWindowsProc(_In_ HWND   tophandle, _In_ LPARAM parm)
{
    HWND iconw = FindWindowEx(tophandle, 0, L"SHELLDLL_DefView", nullptr);
    HWND* workerw = (HWND*)parm;
    if ( iconw != nullptr)
    {
        *workerw = FindWindowEx(0, tophandle, L"WorkerW", 0);
    }
    return true;
}

HWND GetWallpaperParentHWND()
{
    //寻找桌面窗口
    HWND windowHandle = FindWindow(L"Progman", nullptr);
    if(windowHandle == NULL)
    {
        return 0;
    }
    //发送多屏消息
    SendMessage(windowHandle, 0x52c, 0 ,0);
    HWND workerw = nullptr;
    EnumWindows(EnumWindowsProc, (LPARAM)&workerw);
    SendMessage(workerw, 16, 0, 0);
    return windowHandle;
}


VideoWidget::VideoWidget(QWidget* parent)
    :QVideoWidget(parent)
{
    setWallPaperParent();
    mediaPlayer.setVideoOutput(this);
    //原意用于视频意外停止自动恢复播放，然而与切换视频文件冲突，所以暂时弃用该功能
//    connect(&mediaPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));
    connect(&mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
    connect(&mediaPlayer, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));

    savePath = "wallpaper.save";
    read();
    setUrl(QUrl(wallpaperPath));
}

void VideoWidget::setUrl(const QUrl &url)
{
    mediaPlayer.stop();
    mediaPlayer.setMedia(url);
    mediaPlayer.setMuted(true);
    play();
    //保存路径
    wallpaperPath = "file:///"+url.toLocalFile();
    save();
}

void VideoWidget::play()
{
    switch(mediaPlayer.state())
    {
    case QMediaPlayer::PlayingState:
        mediaPlayer.pause();
        break;
    default:
        mediaPlayer.play();
        break;
    }
}

void VideoWidget::repair()
{
    setUrl(QUrl(wallpaperPath));
}

void VideoWidget::setMuted(bool disable)
{
    mediaPlayer.setMuted( disable );
}

void VideoWidget::save()
{
    QFile file(savePath);
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(wallpaperPath.toLocal8Bit());
    }
    file.close();
}

void VideoWidget::read()
{
    QFile file(savePath);
    if(file.open(QIODevice::ReadOnly))
    {
        wallpaperPath = QString::fromLocal8Bit(file.readAll());
    }
    file.close();
}

void VideoWidget::setWallPaperParent()
{
    this->setWindowFlags(Qt::SubWindow);
    SetParent((HWND)this->winId(),GetWallpaperParentHWND());
}

void VideoWidget::stateChanged(QMediaPlayer::State state)
{
    if(state == QMediaPlayer::PausedState || state == QMediaPlayer::StoppedState)
        setUrl(QUrl(wallpaperPath));
}

void VideoWidget::positionChanged(qint64 position)
{
    //由于Qt自带的循环播放功能会有暂停时间，影响体验，所以改用变换播放位置来达到循环播放功能
    if(position > currentDuration-1000)
        mediaPlayer.setPosition(0);
}

void VideoWidget::durationChanged(qint64 duration)
{
    //记录当前视频的时长
    currentDuration = duration;
}
