#include "hexwidget.h"
#include "windows.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainterPath>
#include <QPolygonF>
#include <QPainter>
#include <QBrush>
#include <QVector>
#include <QImage>
#include <QPen>
#include <QAction>
#include <QMenu>
#include <QDesktopServices>
#include <QMimeData>
#include <QDir>
#include <QFileIconProvider>
#include <QFileInfo>

HexWidget::HexWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    this->setAcceptDrops(true);
    //设置默认大小
    setSize(70);
    __lock = false;
}

HexWidget::~HexWidget()
{

}
void HexWidget::paintEvent(QPaintEvent *event)
{
    double size = getSize()/4.0;
    double d = 1.732;
    //计算正六边形的六个顶点位置
    QPointF p[6];
    p[0].setY(0);
    p[0].setX(size*d);

    p[1].setY(size);
    p[1].setX(0);

    p[2].setY(size*3);
    p[2].setX(0);

    p[3].setY(size*4);
    p[3].setX(size*d);

    p[4].setY(size*3);
    p[4].setX(size*2*d);

    p[5].setY(size);
    p[5].setX(size*2*d);
    QVector<QPointF> vecP;
    for(int i=0; i<=6; ++i)
        vecP.append(p[i%6]);
    //开始绘制界面
    QPainter painter(this);
    QPen pen;
    pen.setColor(QColor(0,0,0,0));
    painter.setPen(pen);
    //设置透明画刷
    QImage img(QSize(480,480), QImage::Format_ARGB32);
    img.fill(0xb0404040);
    QBrush brush(img);
    painter.setBrush(brush);
    //定义多边形
    QPolygonF polygon(vecP);
    //绘制实六边形
    QPainterPath path;
    path.addPolygon(polygon);
    painter.drawPath(path);
    //重画边界
    pen.setColor(QColor(0,190,190));
    pen.setWidth(3);
    painter.setPen(pen);
    painter.drawConvexPolygon(polygon);
    //绘制图标
    painter.drawPixmap(size*0.732,size,size*2,size*2, icon);
    event->accept();
}

void HexWidget::mousePressEvent(QMouseEvent *event)
{
    if(!__lock)
    {
        if(event->button()==Qt::LeftButton)
        {
            __dragPosition = event->globalPos();
            event->accept();
        }
    }
}

void HexWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(!__lock)
    {
        if(event->button() == Qt::LeftButton)
        {
            //每次移动完成都需要通知调整控件位置
            emit dockerAdjusted();
            event->accept();
        }
    }
}

void HexWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(!__lock)
    {
        if (event->buttons() & Qt::LeftButton)
        {
            QPoint pos = event->globalPos() - __dragPosition;
            __dragPosition = event->globalPos();
            move(pos+frameGeometry().topLeft());
            //通知控件被移动了
            emit dockerMoved(pos);
            event->accept();
        }
    }
}

void HexWidget::mouseDoubleClickEvent(QMouseEvent *)
{
    if(!url.isEmpty())
    {
        QString file = "file:///"+url;
        ShellExecute(0,L"open",file.toStdWString().c_str(), L"",L"",SW_SHOWNORMAL);
    }
//    if(!url.isEmpty())
//        QDesktopServices::openUrl(QUrl("file:///"+url, QUrl::TolerantMode));
}

void HexWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *menu = new QMenu();
    menu->setStyleSheet("QMenu {\
                         background-color:rgb(89,87,87);\
                         border: 3px solid rgb(0,190,190);\
                         }\
                     QMenu::item {\
                         font-size: 10pt; \
                         color: rgb(225,225,225);\
                         border: 3px solid rgb(60,60,60);\
                         background-color:rgb(89,87,87);\
                         padding:2px 16px;\
                         margin:2px 2px;\
                          }\
                     QMenu::item:selected {background-color:rgb(0,190,190);}\
                     QMenu::item:pressed {background-color:rgb(0,190,190);}\
                        ");
    QAction* add = new QAction(QIcon(":images/add.ico"),"新建");
    connect(add, SIGNAL(triggered(bool)),this,SLOT(newDocker()));
    QString lock_text = __lock?"解锁":"锁定";
    QAction* lock_pos = new QAction(QIcon(":images/lock.ico"),lock_text);
    connect(lock_pos, SIGNAL(triggered(bool)),this,SLOT(setLock(bool)));
    QAction* close = new QAction(QIcon(":images/close.ico"),"关闭");
    connect(close, SIGNAL(triggered(bool)), this, SLOT(closeDocker()));
    QAction* free = new QAction(QIcon(":images/free.ico"),"分离");
    connect(free, SIGNAL(triggered(bool)), this, SLOT(freeDocker()));

    menu->addAction(add);
    menu->addAction(lock_pos);
    menu->addAction(lock_pos);
    menu->addAction(free);
    menu->addAction(close);
    menu->exec(e->globalPos());
    delete menu;
}

void HexWidget::dragEnterEvent(QDragEnterEvent *e)
{
    auto data = e->mimeData();
    if(data->hasUrls())
        if(data->urls().size() == 1)
            e->acceptProposedAction();
}

void HexWidget::dropEvent(QDropEvent *e)
{
    auto data = e->mimeData();
    if(data->hasUrls())
    {
        setUrl(data->urls().first().toLocalFile());
    }
}

void HexWidget::setSize(unsigned int x)
{
    this->setMaximumSize(x*0.868+6,x+2);
    this->setMinimumSize(x*0.868+6,x+2);
}

int HexWidget::getSize()
{
    return this->height()-2;
}

QString HexWidget::getUrl()
{
    return url;
}

void HexWidget::setUrl(const QString &u)
{
    url = u;
    if(!url.isEmpty())
    {
        QFileInfo file_info(url);
        //获取文件图标
        QFileIconProvider icon_provider;
        QIcon ic = icon_provider.icon(file_info);
        icon = ic.pixmap(32,32);
    }
    this->setToolTip(url);
    //重绘界面
    update();
}

void HexWidget::newDocker()
{
    emit createDocker();
}

void HexWidget::setLock(bool)
{
    __lock = !__lock;
}

void HexWidget::closeDocker()
{
    emit dockerClosed();
}

void HexWidget::freeDocker()
{
    emit dockerFreed();
}
