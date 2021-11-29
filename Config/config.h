#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QApplication>

#define REBOOT_CODE             (-1)

class ConfigPrivate;
class Config : public QObject
{
    Q_OBJECT

public:
    enum Language
    {
        __en = 0,
        __cn
    };
    explicit Config(QObject *parent = nullptr);
    ~Config();

    int init(QApplication *a, QQmlApplicationEngine *e);

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

    Q_PROPERTY(int language READ language NOTIFY languageChanged)
    int language();
    Q_INVOKABLE void setLanguage(const int &language);

Q_SIGNALS:
    void rotationChanged();
    void languageChanged();

private:
    friend class ConfigPrivate;
    QScopedPointer<ConfigPrivate> p;
};

#endif // CONFIG_H
