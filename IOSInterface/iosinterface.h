#ifndef IOSINTERFACE_H
#define IOSINTERFACE_H

typedef struct {
    int top;
    int left;
    int bottom;
    int right;
} SafeArea;

class IOSInterface
{
public:
    explicit IOSInterface();
    ~IOSInterface();

    void keepScreenOn();
    void safeArea(SafeArea *t);
};

#endif // IOSINTERFACE_H
