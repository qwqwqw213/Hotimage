#include "imagelistmodel.h"

#include "thread"

#include "QDebug"
#include "QDirIterator"
#include "QFile"
#include "QThread"

class ImageListModelPrivate
{
public:
    explicit ImageListModelPrivate(ImageListModel *parent);
    ~ImageListModelPrivate();

    typedef std::tuple<QString, QString, int, int, QString> Image;
    QVector<Image> list;

    std::thread searchThread;

    // 当前选中索引
    int currentIndex;

    // 是否处于选择状态
    bool selectionStatus;

    // 记录删除起始索引
    int removeStart;
    // 记录删除总数
    int removeCount;

    VideoScanImage *videoScan;

private:
    ImageListModel *f;
};

ImageListModel::ImageListModel(QObject *parent)
    : QAbstractListModel(parent)
    , p(new ImageListModelPrivate(this))
{

}

ImageListModel::~ImageListModel()
{

}

void ImageListModel::search(const QString &path)
{
    qDebug() << "search path:" << path;
    p->searchThread = std::thread([=](){
        QDir d;
        d.setPath(path);
        d.setSorting(QDir::Time | QDir::Reversed);
        d.setNameFilters(QStringList()
                         << QString("*.jpg")
                         << QString("*.jpeg")
                         << QString("*.avi"));

//        d.setNameFilters(QStringList()
//                         << QString("*.jpg")
//                         << QString("*.jpeg"));
        int count = 0;
        QFileInfoList list = d.entryInfoList();
        for(int i = 0; i < list.size(); i++) {
            QString file = list.at(i).filePath();
            QString name = list.at(i).fileName();
            int nameIndex = name.indexOf("_") + 1;
            if( nameIndex >= 1 ) {
                QString strDatetime = name.right(name.length() - nameIndex);
                strDatetime = strDatetime.left(strDatetime.indexOf('.'));
                if( strDatetime.length() == 14 ) {
                    name = QString("%1-%2-%3 %4:%5:%6")
                            .arg(strDatetime.mid(0, 4))
                            .arg(strDatetime.mid(4, 2))
                            .arg(strDatetime.mid(6, 2))
                            .arg(strDatetime.mid(8, 2))
                            .arg(strDatetime.mid(10, 2))
                            .arg(strDatetime.mid(12, 2));
                }
            }
            int type = file.lastIndexOf(".avi") >= 0 ? __video : __image;
            QString qmlPath;
            if( type == __video ) {
//                qDebug() << file;
                qmlPath = p->videoScan->addQueue(file);
            }
            else {
                qmlPath = QString::fromUtf8(QString("file:///" + file).toUtf8());
            }

            p->list.append(std::make_tuple(name, qmlPath, 0, type, file));
            count ++;
        }
        emit searchFinished();
    });
}

int ImageListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return p->list.size();
}

QVariant ImageListModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if( (row >= 0) && (row < p->list.size()) ) {
        auto d = p->list.at(row);
        switch (role) {
        case __name: { return std::get<0>(d); }
        case __path: { return std::get<1>(d); }
        case __selection: { return std::get<2>(d); }
        case __file_type: { return std::get<3>(d); }
        case __file_path: { return std::get<4>(d); }
        default: break;
        }
    }
    return QVariant(0);
}

bool ImageListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int row = index.row();
    if( (row >= 0) && (row < p->list.size()) ) {
        auto d = p->list.at(row);
        switch (role) {
        case __name: { std::get<0>(d) = value.toString(); } break;
        case __path: { std::get<1>(d) = value.toString(); } break;
        case __selection: {
            int old = std::get<2>(d);
            int flag = value.toInt();
            if( p->selectionStatus )
            {
                p->removeStart = p->removeStart > row ? row : p->removeStart;
                if( old == 0 && flag == 1 ) {
                    p->removeCount ++;
                }
                if( old == 1 & flag == 0 ) {
                    if( p->removeCount > 0 ) {
                        p->removeCount --;
                    }
                }
            }
            std::get<2>(d) = flag;
        } break;
        case __file_type: { std::get<3>(d) = value.toInt(); } break;
        case __file_path: { std::get<4>(d) = value.toString(); } break;
        default: { return false; }
        }
        p->list.replace(row, d);
    }
    else {
        return false;
    }
    emit dataChanged(index, index, QVector<int>() << role);

    return true;
}

QHash<int, QByteArray> ImageListModel::roleNames() const
{
    QHash<int, QByteArray> hash;
    hash[__name] = "name";
    hash[__path] = "path";
    hash[__selection] = "selection";
    hash[__file_type] = "fileType";
    hash[__file_path] = "filePath";
    return hash;
}

int ImageListModel::currentIndex()
{
    return p->currentIndex;
}

void ImageListModel::setCurrentIndex(const int &index)
{
    p->currentIndex = index;
    emit currentIndexChanged();
}

QString ImageListModel::name()
{
    if( p->currentIndex >= 0 ) {
        auto d = p->list.at(p->currentIndex);
        return std::get<0>(d);
    }
    return QString("");
}

int ImageListModel::lastType()
{
    if( p->list.size() > 0 ) {
        auto d = p->list.at(p->list.size() - 1);
        return std::get<3>(d);
    }
    return 0;
}

QString ImageListModel::lastImagePath()
{
    if( p->list.size() > 0 ) {
        auto d = p->list.at(p->list.size() - 1);
        return std::get<1>(d);
    }
    return QString("");
}

bool ImageListModel::selectionStatus()
{
    return p->selectionStatus;
}

void ImageListModel::setSelectionStatus(const bool &status)
{
    p->selectionStatus = status;
    if( !status && p->removeCount > 0 ) {
        // 重置选中状态
        int i = p->removeStart;
        int count = 0;
        while (true) {
            if( i >= p->list.size() || count == p->removeCount ) {
                break;
            }
            auto d = p->list.at(i);
            if( std::get<2>(d) ) {
                setData(index(i), 0, __selection);
            }
            i ++;
        }
    }
    p->removeCount = 0;
    p->removeStart = p->list.size();
    emit selectionStatusChanged();
}

void ImageListModel::removeSelection()
{
    if( p->selectionStatus && p->removeCount > 0 ) {
        int i = p->removeStart;
        int removeCount = 0;
        while (true) {
            if( i >= p->list.size() || removeCount == p->removeCount ) {
                qDebug() << "remove finished, count:" << removeCount << p->removeCount << p->removeStart;
                break;
            }
            auto d = p->list.at(i);
            if( std::get<2>(d) ) {
                beginRemoveRows(QModelIndex(), i, i);
                QString path = std::get<1>(d);
                QFile::remove(path.right(path.length() - QString("file:///").length()));
                p->list.takeAt(i);
                removeCount ++;
                endRemoveRows();
            }
            else {
                i ++;
            }
        }

        p->removeCount = 0;
        p->removeStart = p->list.size();
    }
}

VideoScanImage *ImageListModel::provider()
{
    return p->videoScan;
}

void ImageListModel::add(const QString &path)
{
    beginInsertRows(QModelIndex(), p->list.size(), p->list.size());
    QString name = path.right(path.length() - path.lastIndexOf('/') - 1);
    QString file = QString::fromUtf8(QString("file:///" + path).toUtf8());
    int type = file.lastIndexOf(".avi") >= 0 ? __video : __image;

    if( type == __video ) {
        QString scanPath = p->videoScan->addQueue(path);
        p->list.append(std::make_tuple(name, scanPath, 0, type, path));
    }
    else {
        p->list.append(std::make_tuple(name, file, 0, type, path));
    }

//    p->list.append(std::make_tuple(name, file, 0, type));
    endInsertRows();
    emit addNewFile();
}

ImageListModelPrivate::ImageListModelPrivate(ImageListModel *parent)
{
    f = parent;
    QObject::connect(f, &ImageListModel::searchFinished, f, [=](){
        searchThread.join();
    }, Qt::QueuedConnection);

    list.clear();

    currentIndex = -1;
    selectionStatus = false;

    videoScan = new VideoScanImage("videoscan");
}

ImageListModelPrivate::~ImageListModelPrivate()
{
    if( searchThread.joinable() ) {
        searchThread.join();
    }
}
