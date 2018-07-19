#include "desktopgo.h"
#include <QFileDialog>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QMouseEvent>
#include <QMenu>

DesktopGo::DesktopGo(QWidget *parent)
    : QWidget(parent),setwindow(new Ui::Form)
{
    initTrayUI();
    setwindow->setupUi(this);
    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    connect(setwindow->pushButton_close, SIGNAL(clicked(bool)), this, SLOT(hideSetWindow()));
    connect(setwindow->pushButton_wallpaper, SIGNAL(clicked(bool)), this, SLOT(openFile()));
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
    systray->m_topLabel = new QLabel(QStringLiteral(""));
    QPixmap pixmap(":menu/bkg.jpg");
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(pixmap));
    systray->m_topWidget->setAutoFillBackground(true);
    systray->m_topWidget->setPalette(palette);

    QVBoxLayout* m_topLayout = new QVBoxLayout();
    m_topLayout->addWidget(systray->m_topLabel, 0, Qt::AlignLeft|Qt::AlignVCenter);

    m_topLayout->setSpacing(5);
    m_topLayout->setContentsMargins(5, 5, 5, 5);

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

    systray->m_aboutBtn = new QPushButton(QIcon(":menu/about.ico"), QStringLiteral("关于"));
    systray->m_aboutBtn->setObjectName(QStringLiteral("TrayButton"));
    systray->m_aboutBtn->setFixedSize(60, 25);

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

void DesktopGo::openFile()
{
    QUrl url = QFileDialog::getOpenFileUrl(&background);
    if(url.isLocalFile())
    {
        background.setUrl(url);
        setwindow->lineEdit_wallpaper->setText(url.toLocalFile());
    }
}

void DesktopGo::repair()
{
    background.repair();
}

void DesktopGo::quit()
{
    exit(0);
}

void DesktopGo::showSetWindow()
{
    show();
}

void DesktopGo::hideSetWindow()
{
    close();
}
