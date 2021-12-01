#include "imagelistmodel.h"

#include "thread"

#include "QDebug"
#include "QDirIterator"
#include "QFile"

class ImageListModelPrivate
{
public:
    explicit ImageListModelPrivate(ImageListModel *parent);
    ~ImageListModelPrivate();

    typedef std::tuple<QString, QString, int> Image;
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
    p->searchThread = std::thread([=](){
        QDir d;
        d.setPath(path);
        d.setNameFilters(QStringList()
                         << QString("*.jpg")
                         << QString("*.jpeg"));
        QDirIterator iterator(d, QDirIterator::Subdirectories);
        int count = 0;
        while (iterator.hasNext()) {
            iterator.next();
            QFileInfo info = iterator.fileInfo();
            if( info.isFile() ) {
                QString file = info.filePath();
                QString name = file.right(file.length() - file.lastIndexOf('/') - 1);
                QString path = QString::fromUtf8(QString("file:///" + file).toUtf8());
//                qDebug() << name << path;
                p->list.append(std::make_tuple(name, path, 0));
                count ++;
            }
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

QString ImageListModel::newImageUrl()
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

void ImageListModel::add(const QString &path)
{
    beginInsertRows(QModelIndex(), p->list.size(), p->list.size());
    QString name = path.right(path.length() - path.lastIndexOf('/') - 1);
    QString file = QString::fromUtf8(QString("file:///" + path).toUtf8());
    p->list.append(std::make_tuple(name, file, 0));
    endInsertRows();
    emit newImageChanged();
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
}

ImageListModelPrivate::~ImageListModelPrivate()
{
    if( searchThread.joinable() ) {
        searchThread.join();
    }
}
