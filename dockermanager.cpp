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

bool DockerManager::isFreeDocker(const QVector<HexWidget *> &vec)
{
    for(int i=0; i<vec.size(); ++i)
        if(vec[i])
            return false;
    return true;
}

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
                removeFromMap(w, docker);
                (*iter)[i] = 0;
            }
        }
    }
}

void DockerManager::removeFromMap(HexWidget *src, HexWidget *dst)
{
    if(map.find(src) != map.end())
    {
        auto iter = map.find(src);
        for(int i=0; i<iter->size(); ++i)
        {
            if((*iter)[i] == dst)
            {
                (*iter)[i] = 0;
                break;
            }
        }
    }
}

void DockerManager::searchNearDocker(HexWidget *docker)
{
    HexWidget* result = NULL;
    int type = -1;
    double size = docker->getSize()/4.0;
    QPoint pos1 = docker->pos();
    for(auto iter = map.begin(); iter!=map.end(); ++iter)
    {
        if(iter.key() == docker)
            continue;
        QPoint pos2 = iter.key()->pos();
        if(outOfNear(pos1, pos2, size))
            continue;

        type = checkNear(pos1, pos2, size);
        if(type >= 0)
        {
            map[docker][type] = iter.key();
            map[iter.key()][(type+3)%6] = docker;
            result = iter.key();
        }
    }
    if(result)
    {
        adjustPos(result, docker, type);
    }
}

bool DockerManager::outOfNear(const QPoint &pos1, const QPoint &pos2, double size)
{
    int x = pos2.x() - pos1.x();
    int y = pos2.y() - pos1.y();
    if(x<-3.5*size || x>3.5*size || y<-4*size || y>4*size)
        return true;
    else
        return false;
}

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
    return docker;
}

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

void DockerManager::save()
{
    QMap<HexWidget*, int> vecMap;
    int i=0;
    for(auto iter = map.cbegin(); iter!=map.cend(); ++iter, ++i)
    {
        vecMap[iter.key()] = i;
    }
    QJsonDocument doc;
    QJsonArray array;
    for(auto iter = map.cbegin(); iter!=map.cend(); ++iter)
    {
        QJsonObject object;
        object.insert("URL", QString(iter.key()->getUrl().toLocal8Bit().toHex()));
        object.insert("X", iter.key()->frameGeometry().topLeft().x());
        object.insert("Y", iter.key()->frameGeometry().topLeft().y());

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
                    if(object.contains("URL"))
                    {
                        QJsonValue value = object.value("URL");
                        if(value.isString())
                            url = QString::fromLocal8Bit(QByteArray::fromHex(value.toString().toLocal8Bit()));
                    }
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

void DockerManager::moveDocker(const QPoint &offset)
{
    emit beginMove();
    HexWidget* docker = (HexWidget*)this->sender();

    if(map.find(docker) != map.cend())
    {
        const QVector<HexWidget*>& vec = map[docker];
        if(!isFreeDocker(vec))
        {
            if(cache.empty())
            {
                searchLinkedDocker(docker, cache);
            }
            for(auto iter=cache.begin(); iter!=cache.end(); ++iter)
            {
                if((*iter) != docker)
                    (*iter)->move(offset+(*iter)->frameGeometry().topLeft());
            }
        }

    }
}

void DockerManager::adjustDocker()
{
    emit endMove();
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
