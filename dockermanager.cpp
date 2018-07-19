#include "dockermanager.h"
#include <QPoint>
#include <QQueue>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QByteArray>
#include <QIODevice>
#include <QFile>

//遍历邻接矩阵，查看是否有相邻的容器
bool DockerManager::isFreeDocker(const QVector<HexWidget *> &vec)
{
    for(int i=0; i<vec.size(); ++i)
        if(vec[i])
            return false;
    return true;
}

//清空容器的邻接矩阵
void DockerManager::setDockerFree(HexWidget *docker)
{
    if(map.find(docker)!=map.end())
    {
        auto iter = map.find(docker);
        for(int i=0; i<iter->size(); ++i)
        {
            HexWidget* w = (*iter)[i];
            if(w)
            {
                //先把自己从对方的邻接矩阵中移除
                removeFromMap(w, docker);
                //再将对象从自己的邻接矩阵中移除
                (*iter)[i] = 0;
            }
        }
    }
}

//将dst容器从src的邻接矩阵中移除
void DockerManager::removeFromMap(HexWidget *src, HexWidget *dst)
{
    if(map.find(src) != map.end())
    {
        auto iter = map.find(src);
        for(int i=0; i<iter->size(); ++i)
        {
            if((*iter)[i] == dst)
            {
                //找到直接移除
                (*iter)[i] = 0;
                break;
            }
        }
    }
}

//搜索所有的容器，找出与容器相邻的容器
//相邻的概念：距离自己较近的容器
void DockerManager::searchNearDocker(HexWidget *docker)
{
    HexWidget* result = NULL;
    //与自己的哪一面相邻
    int type = -1;
    //容器的单位size
    double size = docker->getSize()/4.0;
    //自己的位置
    QPoint pos1 = docker->pos();
    for(auto iter = map.begin(); iter!=map.end(); ++iter)
    {
        //忽略自己
        if(iter.key() == docker)
            continue;
        if(iter.key()->isHidden())
            continue;
        QPoint pos2 = iter.key()->pos();
        //先判断是否距离自己很远，忽略一些很远的容器
        if(outOfNear(pos1, pos2, size))
            continue;
        //判断与自己哪一面相邻
        type = checkNear(pos1, pos2, size);
        if(type >= 0)
        {
            //在对应的邻接矩阵位置上添加记录
            map[docker][type] = iter.key();
            map[iter.key()][(type+3)%6] = docker;
            result = iter.key();
        }
    }
    if(result)
    {
        //调整自己的位置与目标相连
        adjustPos(result, docker, type);
    }
}

//下面三个函数需要在草稿纸上画着两个六边形理解
//判断两个位置上的容器是否相离“很远”
bool DockerManager::outOfNear(const QPoint &pos1, const QPoint &pos2, double size)
{
    int x = pos2.x() - pos1.x();
    int y = pos2.y() - pos1.y();
    if(x<-3.5*size || x>3.5*size || y<-4*size || y>4*size)
        return true;
    else
        return false;
}

//检查pos2上的容器在pos1上的容器的哪一面
int DockerManager::checkNear(const QPoint &pos1, const QPoint &pos2, double size)
{
    int x = pos2.x() - pos1.x();
    int y = pos2.y() - pos1.y();
    if(x>-3.5*size && x<-2*size && y>-2*size && y<2*size)
        return 0;
    else if(x>-3*size && x<0 && y>-4*size && y<-2*size)
        return 1;
    else if(x>0 && x<3*size && y>-4*size && y<-2*size)
        return 2;
    else if(x>2*size && x<3.5*size && y>-2*size && y<2*size)
        return 3;
    else if(x>0 && x<3*size && y>2*size && y<4*size)
        return 4;
    else if(x>-3*size && x<0 && y>2*size && y<4*size)
        return 5;
    else
        return -1;
}

//调整to容器的位置到from容器的指定相邻位置
void DockerManager::adjustPos(HexWidget *from, HexWidget *to, int type)
{
    auto rect = from->geometry();
    double x=0,y=0;
    double size = from->getSize()/4.0;
    double w = 3.464*size;
    switch(type)
    {
    case 0:
        x = w;
        y = 0;
        break;
    case 1:
        x = w/2.0;
        y = 3*size;
        break;
    case 2:
        x = -w/2.0+2;
        y = 3*size;
        break;
    case 3:
        x = -w+2;
        y = 0;
        break;
    case 4:
        x = -w/2.0+2;
        y = -3*size;
        break;
    case 5:
        x = w/2.0;
        y = -3*size;
        break;
    }
    rect.setX( rect.x()+x);
    rect.setY(rect.y()+y);
    to->setGeometry(rect);
}

//利用广度优先搜索算法查找容器所在的连通图
void DockerManager::searchLinkedDocker(HexWidget *docker, QSet<HexWidget *> &list)
{
    QQueue<HexWidget*> que;
    que.push_back(docker);
    list.insert(docker);
    while(!que.empty())
    {
        HexWidget* d = que.front();
        que.pop_front();
        if(map.find(d) != map.cend())
        {
            const QVector<HexWidget*>& vec = map[d];
            for(int i=0; i<vec.size(); ++i)
            {
                if(vec[i])
                {
                    if(list.find(vec[i]) == list.cend())
                    {
                        que.push_back(vec[i]);
                        list.insert(vec[i]);
                    }
                }
            }
        }
    }
}

DockerManager::DockerManager(QObject *parent) : QObject(parent)
{
    savePath = "DesktopGO.save";
    recover();
    //定时保存状态，避免程序崩溃导致用户配置丢失
    connect(&timer, SIGNAL(timeout()), this, SLOT(save()));
    timer.start(5000);
}

DockerManager::~DockerManager()
{

}

HexWidget* DockerManager::addDocker(const QString& url)
{
    auto docker = new HexWidget();
    docker->setUrl(url);
    docker->show();
    map[docker] = QVector<HexWidget*>(6, 0);
    connect(docker, SIGNAL(dockerMoved(QPoint)), this, SLOT(moveDocker(QPoint)));
    connect(docker, SIGNAL(dockerAdjusted()), this, SLOT(adjustDocker()));
    connect(docker, SIGNAL(dockerFreed()), this, SLOT(freeDocker()));
    connect(docker, SIGNAL(createDocker()), this ,SLOT(createDocker()));
    connect(docker, SIGNAL(dockerClosed()), this, SLOT(closeDocker()));
    connect(docker, SIGNAL(hideLinkedDockers(bool)), this, SLOT(hideLinkedDockers(bool)));
    return docker;
}

HexWidget *DockerManager::addDocker(const QString &url, const QPoint &pos)
{
    auto docker = new HexWidget();
    docker->setUrl(url);
    docker->move(pos);
    docker->show();
    map[docker] = QVector<HexWidget*>(6, 0);
    connect(docker, SIGNAL(dockerMoved(QPoint)), this, SLOT(moveDocker(QPoint)));
    connect(docker, SIGNAL(dockerAdjusted()), this, SLOT(adjustDocker()));
    connect(docker, SIGNAL(dockerFreed()), this, SLOT(freeDocker()));
    connect(docker, SIGNAL(createDocker()), this ,SLOT(createDocker()));
    connect(docker, SIGNAL(dockerClosed()), this, SLOT(closeDocker()));
    connect(docker, SIGNAL(hideLinkedDockers(bool)), this, SLOT(hideLinkedDockers(bool)));
    return docker;
}

//关闭指定容器
void DockerManager::delDocker(HexWidget *docker)
{
    setDockerFree(docker);
    map.remove(docker);
    docker->deleteLater();
    if(map.empty())
    {
        exit(0);
    }
}

int DockerManager::size()
{
    return map.size();
}

//以json格式保存当前状态
void DockerManager::save()
{
    QMap<HexWidget*, int> vecMap;
    int i=0;
    //为所有容器建立索引
    for(auto iter = map.cbegin(); iter!=map.cend(); ++iter, ++i)
    {
        vecMap[iter.key()] = i;
    }
    QJsonDocument doc;
    QJsonArray array;
    for(auto iter = map.cbegin(); iter!=map.cend(); ++iter)
    {
        //保存每个容器的URL及位置
        QJsonObject object;
        object.insert("URL", QString(iter.key()->getUrl().toLocal8Bit().toHex()));
        object.insert("X", iter.key()->frameGeometry().topLeft().x());
        object.insert("Y", iter.key()->frameGeometry().topLeft().y());

        //保存邻接矩阵
        QJsonArray linked;
        for(int i=0; i<iter->size(); ++i)
        {
            if( (*iter)[i] )
            {
                linked.append( vecMap[ (*iter)[i] ] );
            }
            else
                linked.append( -1 );
        }
        object.insert("Link", linked);

        array.append(object);
    }
    doc.setArray(array);

    QString jsonStr(doc.toJson(QJsonDocument::Compact));
    QByteArray bytes = jsonStr.toLocal8Bit();
    QFile file(savePath);
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(bytes);
    }
    file.close();
}

//读取配置文件，恢复状态
void DockerManager::recover()
{
    QByteArray bytes;
    QFile file(savePath);
    if(file.open(QIODevice::ReadOnly))
    {
        bytes = file.readAll();
    }
    file.close();

    QVector<HexWidget*> vec;
    QVector< QVector<int> > vecMap;

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(bytes, &jsonError);
    if(!doc.isNull() && jsonError.error == QJsonParseError::NoError)
    {
        if(doc.isArray())
        {
            //读取容器列表
            QJsonArray array = doc.array();
            for(int i=0; i<array.size(); ++i)
            {
                if(array[i].isObject())
                {
                    QJsonObject object = array[i].toObject();
                    QString url;
                    int x;
                    int y;
                    QVector<int> linked;
                    //读取URL
                    if(object.contains("URL"))
                    {
                        QJsonValue value = object.value("URL");
                        if(value.isString())
                            url = QString::fromLocal8Bit(QByteArray::fromHex(value.toString().toLocal8Bit()));
                    }
                    //读取容器坐标
                    if(object.contains("X"))
                    {
                        QJsonValue value = object.value("X");
                        if(value.isDouble())
                            x = value.toDouble();
                    }
                    if(object.contains("Y"))
                    {
                        QJsonValue value = object.value("Y");
                        if(value.isDouble())
                            y = value.toDouble();
                    }
                    //读取容器的邻接矩阵
                    if(object.contains("Link"))
                    {
                        QJsonValue value = object.value("Link");
                        if(value.isArray())
                        {
                            QJsonArray link = value.toArray();
                            for(int j=0; j<link.size(); ++j)
                                linked.append(link[j].toDouble());
                        }
                    }
                    auto docker = addDocker(url);
                    vec.append(docker);
                    vecMap.append(linked);
                    docker->move(x,y);
                }
            }
        }
    }
    //恢复管理器的容器状态图
    if(vec.size() == vecMap.size() && !vec.isEmpty())
    {
        map.clear();
        for(int i=0; i<vec.size(); ++i)
        {
            QVector<HexWidget*> link;
            for(int j=0; j<vecMap[i].size(); ++j)
            {
                if(vecMap[i][j] < 0)
                    link.append(0);
                else
                    link.append(vec[vecMap[i][j]]);
            }
            map[vec[i]] = link;
        }
    }
    if(map.empty())
        addDocker("");
}

//移动单个容器时，容器所在的连通图内所有容器会跟随移动
void DockerManager::moveDocker(const QPoint &offset)
{
    HexWidget* docker = (HexWidget*)this->sender();

    if(map.find(docker) != map.cend())
    {
        const QVector<HexWidget*>& vec = map[docker];
        if(!isFreeDocker(vec))
        {
            //是否存在连通图缓存
            if(cache.empty())
            {
                searchLinkedDocker(docker, cache);
            }
            for(auto iter=cache.begin(); iter!=cache.end(); ++iter)
            {
                if((*iter) != docker)
                {
                    (*iter)->move(offset+(*iter)->frameGeometry().topLeft());

                }
            }
        }
    }
}

//单个容器移动完后会自动调整位置，达到自动拖拽连接的效果
void DockerManager::adjustDocker()
{
    cache.clear();
    HexWidget* docker = (HexWidget*)this->sender();
    if(map.find(docker) != map.cend())
    {
        const QVector<HexWidget*>& vec = map[docker];
        if(isFreeDocker(vec))
            searchNearDocker(docker);
    }
}

void DockerManager::freeDocker()
{
    HexWidget* docker = (HexWidget*)this->sender();
    setDockerFree(docker);
}

void DockerManager::closeDocker()
{
    HexWidget* docker = (HexWidget*)this->sender();
    delDocker( docker );
}

void DockerManager::createDocker()
{
    addDocker("");
}

//收拢指定容器所在的连通图
void DockerManager::hideLinkedDockers(bool hide)
{
    HexWidget* docker = (HexWidget*)this->sender();
    if(map.find(docker) != map.cend())
    {
        const QVector<HexWidget*>& vec = map[docker];
        if(!isFreeDocker(vec))
        {
            searchLinkedDocker(docker, cache);
            for(auto iter=cache.begin(); iter!=cache.end(); ++iter)
            {
                if((*iter) != docker)
                {
                    (*iter)->setHidden(hide);
                }
            }
            cache.clear();
        }

    }
}
