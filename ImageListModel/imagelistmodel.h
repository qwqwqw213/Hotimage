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
        __selection,
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

    Q_PROPERTY(QString newImageUrl READ newImageUrl NOTIFY newImageChanged)
    QString newImageUrl();

    Q_PROPERTY(int selectionStatus READ selectionStatus WRITE setSelectionStatus NOTIFY selectionStatusChanged)
    bool selectionStatus();
    void setSelectionStatus(const bool &status);

    Q_INVOKABLE void removeSelection();

public Q_SLOTS:
    void add(const QString &path);

private:
    friend class ImageListModelPrivate;
    QScopedPointer<ImageListModelPrivate> p;

Q_SIGNALS:
    void newImageChanged();
    void searchFinished();
    void currentIndexChanged();
    void selectionStatusChanged();
};

#endif // IMAGELISTMODEL_H
