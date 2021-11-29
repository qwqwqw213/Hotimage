#include "imagelistmodel.h"

#include "thread"

#include "QDebug"
#include "QDirIterator"

class ImageListModelPrivate
{
public:
    explicit ImageListModelPrivate(ImageListModel *parent);
    ~ImageListModelPrivate();

    typedef std::tuple<QString, QString> Image;
    QVector<Image> list;

    std::thread searchThread;

private:
    ImageListModel *f;
};

ImageListModel::ImageListModel(QObject *parent)
    : QAbstractListModel(parent)
    , p(new ImageListModelPrivate(this))
{
    m_currentIndex = -1;
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
                p->list.append(std::make_tuple(name, path));
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
        default: break;
        }
    }
    return QVariant(0);
}

QHash<int, QByteArray> ImageListModel::roleNames() const
{
    QHash<int, QByteArray> hash;
    hash[__name] = "name";
    hash[__path] = "path";
    return hash;
}

int ImageListModel::currentIndex()
{
    return m_currentIndex;
}

void ImageListModel::setCurrentIndex(const int &index)
{
    m_currentIndex = index;
    emit currentIndexChanged();
}

QString ImageListModel::newImageUrl()
{
    if( p->list.size() > 0 ) {
        auto d = p->list.at(p->list.size() - 1);
        return std::get<1>(d);
    }
    return QString("");
}

void ImageListModel::add(const QString &path)
{
    QString name = path.right(path.length() - path.lastIndexOf('/') - 1);
    QString file = QString::fromUtf8(QString("file:///" + path).toUtf8());
    p->list.append(std::make_tuple(name, file));
    emit newImageChanged();
}

ImageListModelPrivate::ImageListModelPrivate(ImageListModel *parent)
{
    f = parent;
    QObject::connect(f, &ImageListModel::searchFinished, f, [=](){
        searchThread.join();
    }, Qt::QueuedConnection);

    list.clear();
}

ImageListModelPrivate::~ImageListModelPrivate()
{
    if( searchThread.joinable() ) {
        searchThread.join();
    }
}
