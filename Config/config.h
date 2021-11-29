#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

class ConfigPrivate;
class Config : public QObject
{
    Q_OBJECT

public:
    explicit Config(QObject *parent = nullptr);
    ~Config();

    int init(QGuiApplication *a, QQmlApplicationEngine *e);

    // 添加CONSTANT关键字
    // 屏蔽QML depends on non-NOTIFYable properties 警告
    Q_PROPERTY(int width READ width CONSTANT)
    int width();
    Q_PROPERTY(int height READ height CONSTANT)
    int height();

    Q_PROPERTY(QString albumFolder READ albumFolder CONSTANT)
    QString albumFolder();

    void release();

    Q_PROPERTY(qreal rotation READ rotation NOTIFY rotationChanged)
    qreal rotation();
    Q_INVOKABLE void setRotation();

Q_SIGNALS:
    void rotationChanged();

private:
    friend class ConfigPrivate;
    QScopedPointer<ConfigPrivate> p;
};

#endif // CONFIG_H
