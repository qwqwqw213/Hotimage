#ifndef CONFIG_H
#define CONFIG_H
#include <QObject>
#include <QQmlApplicationEngine>
//#include <QApplication>
#include "QGuiApplication"

class Config;
#ifdef g_Config
#undef g_Config
#endif
#define g_Config                (static_cast<Config *>(Config::interface()))

#define REBOOT_CODE             (-1)

typedef struct {
    QString  key;
    QVariant value;
} SettingParam;

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

    static Config * instance();
    static Config * interface() {
        return configSelf;
    }

    int init(QGuiApplication *a, QQmlApplicationEngine *e);

    QString documentsPath();
    QString imageGalleryPath();
    QVariant readSetting(const QString &key, const QVariant &normal);
    void insertSetting(const QVector<SettingParam> &param);

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

    Q_PROPERTY(bool isLandscape READ isLandscape NOTIFY orientationChanged)
    bool isLandscape();

    Q_PROPERTY(int leftMargin READ leftMargin CONSTANT)
    int leftMargin();
    Q_PROPERTY(int rightMargin READ rightMargin CONSTANT)
    int rightMargin();
    Q_PROPERTY(int topMargin READ topMargin CONSTANT)
    int topMargin();
    Q_PROPERTY(int bottomMargin READ bottomMargin CONSTANT)
    int bottomMargin();

    Q_PROPERTY(QString fontLight READ fontLight CONSTANT)
    QString fontLight();
    Q_PROPERTY(QString fontSolid READ fontSolid CONSTANT)
    QString fontSolid();
    Q_PROPERTY(QString fontRegular READ fontRegular CONSTANT)
    QString fontRegular();
    Q_PROPERTY(QString fontThin READ fontThin CONSTANT)
    QString fontThin();
    Q_PROPERTY(QString fontBrandsRegular READ fontBrandsRegular CONSTANT)
    QString fontBrandsRegular();
    Q_PROPERTY(QString fontDuotoneSolid READ fontDuotoneSolid CONSTANT)
    QString fontDuotoneSolid();

    Q_PROPERTY(bool canReadTemperature READ canReadTemperature CONSTANT)
    bool canReadTemperature();
    Q_PROPERTY(bool isMobile READ isMobile CONSTANT)
    bool isMobile();

    Q_INVOKABLE void saveSetting();

Q_SIGNALS:
    void rotationChanged();
    void languageChanged();
    void orientationChanged();


private:
    friend class ConfigPrivate;
    QScopedPointer<ConfigPrivate> p;

    static Config *configSelf;
};

#endif // CONFIG_H
