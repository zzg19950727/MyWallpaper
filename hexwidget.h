#ifndef HEXWIDGET_H
#define HEXWIDGET_H
#include <QWidget>
#include <QPoint>
#include <QPixmap>
#include <QString>
#include <QUrl>
class HexWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HexWidget(QWidget *parent = 0);
    ~HexWidget();

public:
    //绘制界面
    void paintEvent(QPaintEvent *event);
    //移动控件
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    //打开文件
    void mouseDoubleClickEvent(QMouseEvent*);
    //右键菜单
    virtual void contextMenuEvent(QContextMenuEvent*);
    //接受文件拖拽
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent*);

public:
    void setSize(unsigned int x);
    int getSize();
    void setUrl(const QString& url);
    QString getUrl();

signals:
    //控件被移动了
    void dockerMoved(const QPoint& offset);
    //控件分离
    void dockerFreed();
    //控件被关闭了
    void dockerClosed();
    //控件需要调整位置
    void dockerAdjusted();
    //需要新建控件
    void createDocker();

public slots:
    void newDocker();
    void setLock(bool);
    void closeDocker();
    void freeDocker();

private:
    QPoint __dragPosition;
    //锁定位置，能不能移动
    bool __lock;
    QString url;
    QPixmap icon;
};

#endif // HEXWIDGET_H
