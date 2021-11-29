#ifndef IMAGELISTMODEL_H
#define IMAGELISTMODEL_H

#include "QAbstractListModel"

class ImageListModelPrivate;
class ImageListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ImageListRole
    {
        __name = Qt::UserRole + 1,
        __path,
    };
    explicit ImageListModel(QObject *parent = nullptr);
    ~ImageListModel();

    void search(const QString &path);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    int currentIndex();
    Q_INVOKABLE void setCurrentIndex(const int &index);

    Q_PROPERTY(QString newImageUrl READ newImageUrl NOTIFY newImageChanged)
    QString newImageUrl();

public Q_SLOTS:
    void add(const QString &path);

private:
    friend class ImageListModelPrivate;
    QScopedPointer<ImageListModelPrivate> p;

    int m_currentIndex;

Q_SIGNALS:
    void newImageChanged();
    void searchFinished();
    void currentIndexChanged();
};

#endif // IMAGELISTMODEL_H
