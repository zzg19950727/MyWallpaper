#include "desktopgo.h"
#include <QStandardPaths>
#include <QDesktopWidget>
#include <QApplication>
#include <QMouseEvent>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QBrush>
#include <QMenu>
#include <QDebug>

DesktopGo::DesktopGo(QWidget *parent)
    : QWidget(parent),setwindow(new Ui::Form)
{
    setwindow->setupUi(this);

    QDesktopWidget* desktop = QApplication::desktop();
    bgCount = desktop->screenCount();
    background = new VideoWidget[bgCount];

    initUI();
    initTrayUI();

    QFile file("wallpaper.save");
    if(file.open(QIODevice::ReadOnly))
    {
        for(int i=0; i<bgCount; ++i)
        {
            QString wallpaperPath = QString::fromLocal8Bit(file.readLine());
            wallpaperPath.replace("\n", "");
            background[i].setUrl(wallpaperPath);
            background[i].showFullScreen();
            background[i].move(desktop->screen(i)->pos());
            background[i].setMaximumSize(desktop->screen(i)->size());
        }
    }
    file.close();

    timer.start(5000);
}

DesktopGo::~DesktopGo()
{

}

void DesktopGo::initUI()
{
    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground,true);

    setwindow->comboBox_screen->addItem("所有屏幕");
    for(int i=0; i<bgCount; ++i)
        setwindow->comboBox_screen->addItem(QString("第%1块屏幕").arg(i+1));
    connect(setwindow->comboBox_screen, SIGNAL(currentIndexChanged(int)), this, SLOT(bgItemChanged(int)));
    connect(setwindow->pushButton_close, SIGNAL(clicked(bool)), this, SLOT(hideSetWindow()));
    connect(setwindow->pushButton_wallpaper, SIGNAL(clicked(bool)), this, SLOT(openFile()));
    connect(setwindow->checkBox_show, SIGNAL(stateChanged(int)), this, SLOT(showBackground()));
    connect(&timer, SIGNAL(timeout()), this, SLOT(auto_save()));
}

void DesktopGo::initTrayUI()
{
    systray = new SystemTray;
    if( !systray )
        return;

    systray->m_systemTray = new QSystemTrayIcon(this);
    systray->m_trayMenu = new QMenu();

    //init top widget
    systray->m_topWidget = new QWidget();
    systray->m_topWidget->setMinimumHeight(50);
    systray->m_topWidgetAction = new QWidgetAction(systray->m_trayMenu);
    systray->m_topLabel = new QLabel(QStringLiteral(""));;
    systray->m_topLabel->setStyleSheet("QLabel{border-image:url(:/menu/bkg.jpg);}");
    QVBoxLayout* m_topLayout = new QVBoxLayout();
    m_topLayout->addWidget(systray->m_topLabel);

    m_topLayout->setSpacing(0);
    m_topLayout->setContentsMargins(0, 0, 0, 0);

    systray->m_topWidget->setLayout(m_topLayout);
    systray->m_topWidget->installEventFilter(this);
    systray->m_topWidgetAction->setDefaultWidget(systray->m_topWidget);
    systray->m_trayMenu->addAction(systray->m_topWidgetAction);


    //init action
    systray->m_mdWidget = new QWidget();
    systray->m_mdWidgetAction = new QWidgetAction(systray->m_trayMenu);
    systray->m_setBtn = new QPushButton(QIcon(":menu/repair.ico"), QStringLiteral("设置"));
    connect(systray->m_setBtn, SIGNAL(clicked(bool)),this, SLOT(showSetWindow()));
    systray->m_setBtn->setMinimumHeight(30);

    systray->m_repairBtn = new QPushButton(QIcon(":menu/display.ico"), QStringLiteral("修复"));
    connect(systray->m_repairBtn, SIGNAL(clicked(bool)),this, SLOT(repair()));
    systray->m_repairBtn->setMinimumHeight(30);

    QVBoxLayout* m_mdLayout = new QVBoxLayout();
    m_mdLayout->addWidget(systray->m_setBtn);
    m_mdLayout->addWidget(systray->m_repairBtn);

    m_mdLayout->setSpacing(5);
    m_mdLayout->setContentsMargins(5, 5, 5, 5);

    systray->m_mdWidget->setLayout(m_mdLayout);
    systray->m_mdWidget->installEventFilter(this);
    systray->m_mdWidgetAction->setDefaultWidget(systray->m_mdWidget);
    systray->m_trayMenu->addAction(systray->m_mdWidgetAction);
    systray->m_trayMenu->addSeparator();

    //init buttom widget
    systray->m_bottomWidget = new QWidget();
    systray->m_bottomWidgetAction = new QWidgetAction(systray->m_trayMenu);

    systray->m_aboutBtn = new QPushButton(QIcon(":menu/about.ico"), QStringLiteral("新建"));

    systray->m_aboutBtn->setObjectName(QStringLiteral("TrayButton"));
    systray->m_aboutBtn->setFixedSize(60, 25);
    connect(systray->m_aboutBtn, SIGNAL(clicked(bool)), this, SLOT(addDocker()));
    systray->m_exitBtn = new QPushButton(QIcon(":menu/quit.ico"), QStringLiteral("退出"));
    connect(systray->m_exitBtn, SIGNAL(clicked(bool)), this, SLOT(quit()));
    systray->m_exitBtn->setObjectName(QStringLiteral("TrayButton"));
    systray->m_exitBtn->setFixedSize(60, 25);

    QHBoxLayout* m_bottomLayout = new QHBoxLayout();
    m_bottomLayout->addWidget(systray->m_aboutBtn, 0, Qt::AlignCenter);
    m_bottomLayout->addWidget(systray->m_exitBtn, 0, Qt::AlignCenter);

    m_bottomLayout->setSpacing(5);
    m_bottomLayout->setContentsMargins(2,2,2,2);

    systray->m_bottomWidget->setLayout(m_bottomLayout);
    systray->m_bottomWidgetAction->setDefaultWidget(systray->m_bottomWidget);
    systray->m_trayMenu->addAction(systray->m_bottomWidgetAction);


    //init menu event
    systray->m_systemTray->setContextMenu(systray->m_trayMenu);
    systray->m_systemTray->setIcon(QIcon(":menu/display.ico"));
    systray->m_systemTray->setToolTip("DesktopGo");
    systray->m_systemTray->show();

    systray->m_trayMenu->setStyleSheet("QMenu{background:white;border:0px solid lightgray;}"
    "QMenu::separator{height:1px;background: lightgray;margin:2px 0px 2px 0px;\
    }");

    systray->m_aboutBtn->setStyleSheet("QPushButton{border:0px;}"
        "QPushButton:hover{background: lightgray;color: rgb(42, 120, 192);}"
        "QPushButton:pressed{background: lightgray;color: rgb(42, 120, 192);}");

    systray->m_exitBtn->setStyleSheet("QPushButton{border:0px;}"
        "QPushButton:hover{background: lightgray;color: rgb(42, 120, 192);}"
        "QPushButton:pressed{background: lightgray;color: rgb(42, 120, 192);}");

    systray->m_setBtn->setStyleSheet("QPushButton{border:0px;}"
        "QPushButton:hover{background: lightgray;color: rgb(42, 120, 192);}"
        "QPushButton:pressed{background: lightgray;color: rgb(42, 120, 192);}"
                                     "QPushButton{text-align : left;}");\
    systray->m_repairBtn->setStyleSheet("QPushButton{border:0px;}"
        "QPushButton:hover{background: lightgray;color: rgb(42, 120, 192);}"
        "QPushButton:pressed{background: lightgray;color: rgb(42, 120, 192);}"
                                        "QPushButton{text-align : left;}");
}

void DesktopGo::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        __dragPosition = event->globalPos();
        event->accept();
    }
}

void DesktopGo::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        QPoint pos = event->globalPos() - __dragPosition;
        __dragPosition = event->globalPos();
        move(pos+frameGeometry().topLeft());
        event->accept();
    }
}

void DesktopGo::paintEvent(QPaintEvent *event)
{
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
    painter.drawRoundedRect(this->rect(), 15, 15);
}

void DesktopGo::bgItemChanged(int index)
{
    if(index)
    {
        if(background[index-1].isVisible())
        {
            setwindow->checkBox_show->setChecked(false);
        }
        else
        {
            setwindow->checkBox_show->setChecked(true);
        }
        setwindow->lineEdit_wallpaper->setText(background[index-1].getUrl());
    }
    else
    {
        setwindow->lineEdit_wallpaper->clear();
    }
}

void DesktopGo::openFile()
{
    QUrl url = QFileDialog::getOpenFileUrl(this);
    if(url.isLocalFile())
    {
        int index = setwindow->comboBox_screen->currentIndex();
        if(index)
            background[index-1].setUrl(url);
        else
        {
            for(int i=0; i<bgCount; ++i)
                background[i].setUrl(url);
        }
        setwindow->lineEdit_wallpaper->setText(url.toLocalFile());
    }
}

void DesktopGo::repair()
{
    for(int i=0; i<bgCount; ++i)
        background[i].repair();
}

void DesktopGo::quit()
{
    exit(0);
}

void DesktopGo::addDocker()
{
    dockers.addDocker("", QPoint(100,100));
}

void DesktopGo::auto_save()
{
    QString savePath = "wallpaper.save";
    QFile file(savePath);
    if(!file.open(QIODevice::WriteOnly))
    {
        return;
    }

    for(int i=0; i<bgCount; ++i)
    {
        file.write(background[i].getUrl().toLocal8Bit());
        file.write("\n");
    }
    file.close();

    dockers.save();
}

void DesktopGo::showSetWindow()
{
    show();
}

void DesktopGo::hideSetWindow()
{
    close();
}

void DesktopGo::showBackground()
{
    int index = setwindow->comboBox_screen->currentIndex();
    if(setwindow->checkBox_show->isChecked())
    {
        if(index)
            background[index-1].close();
        else
        {
            for(int i=0; i<bgCount; ++i)
                background[i].close();
        }
    }
    else
    {
        if(index)
            background[index-1].showFullScreen();
        else
        {
            for(int i=0; i<bgCount; ++i)
                background[i].showFullScreen();
        }
    }
}
