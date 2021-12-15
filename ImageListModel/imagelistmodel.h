#ifndef IMAGELISTMODEL_H
#define IMAGELISTMODEL_H

#include "QAbstractListModel"

#include "VideoScanImage/videoscanimage.h"

// image gallery

class ImageListModelPrivate;
class ImageListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ImageListRole
    {
        __name = Qt::UserRole + 1,
        __path,
        __selection,
        __file_type,
        __file_path
    };
    enum FileType
    {
        __image = 0,
        __video
    };
    explicit ImageListModel(QObject *parent = nullptr);
    ~ImageListModel();

    void search(const QString &path);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QHash<int, QByteArray> roleNames() const;

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    int currentIndex();
    Q_INVOKABLE void setCurrentIndex(const int &index);

    Q_PROPERTY(QString name READ name CONSTANT)
    QString name();

    Q_PROPERTY(int lastType  READ lastType NOTIFY addNewFile)
    int lastType();

    Q_PROPERTY(QString lastImagePath READ lastImagePath NOTIFY addNewFile)
    QString lastImagePath();

    Q_PROPERTY(int selectionStatus READ selectionStatus WRITE setSelectionStatus NOTIFY selectionStatusChanged)
    bool selectionStatus();
    void setSelectionStatus(const bool &status);

    Q_INVOKABLE void removeSelection();

    VideoScanImage *provider();

public Q_SLOTS:
    void add(const QString &path);

private:
    friend class ImageListModelPrivate;
    QScopedPointer<ImageListModelPrivate> p;

Q_SIGNALS:
    void addNewFile();
    void searchFinished();
    void currentIndexChanged();
    void selectionStatusChanged();

    void videoStatusChanged();
    void videoFrameChanged();
};

#endif // IMAGELISTMODEL_H
