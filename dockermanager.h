#ifndef DOCKERMANAGER_H
#define DOCKERMANAGER_H
#include "hexwidget.h"
#include <QVector>
#include <QObject>
#include <QSet>
#include <QMap>

class DockerManager : public QObject
{
    Q_OBJECT
protected:
    //判断容器是否独立
    bool isFreeDocker(const QVector<HexWidget*>&);
    //分离容器
    void setDockerFree(HexWidget*);
    //移除src与dst的连接
    void removeFromMap(HexWidget* src, HexWidget* dst);
    //搜索附近的容器并关联
    void searchNearDocker(HexWidget* docker);
    //过滤，是否相离很远
    bool outOfNear(const QPoint& pos1, const QPoint& pos2, double size);
    //检查两个容器的相邻关系
    int checkNear(const QPoint& pos1, const QPoint& pos2, double size);
    //将from调整到to的指定位置
    void adjustPos(HexWidget* from, HexWidget* to, int type);
    //搜索关联的容器
    void searchLinkedDocker(HexWidget* docker, QSet<HexWidget*>& list);
    //删除容器
    void delDocker(HexWidget*);

public:
    explicit DockerManager(QObject *parent = 0);
    ~DockerManager();
    //添加容器
    HexWidget* addDocker(const QString& url);
    HexWidget* addDocker(const QString& url, const QPoint& pos);
    int size();
    void save();

private slots:
    void moveDocker(const QPoint& offset);
    void recover();
    void adjustDocker();
    void freeDocker();
    void closeDocker();
    void createDocker();
    void hideLinkedDockers(bool);

private:
    //图标整理网络邻接表
    QMap<HexWidget*, QVector<HexWidget*>> map;
    QVector< QSet<HexWidget*> > sets;
    //上一次移动的连通图的缓存
    QSet<HexWidget*> cache;
    QString savePath;
};

#endif // DOCKERMANAGER_H
