#include "iosinterface.h"

#include "QDebug"

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

IOSInterface::IOSInterface(QObject *parent)
    : QObject(parent)
{

}

IOSInterface::~IOSInterface()
{

}

bool IOSInterface::hotspotOpen()
{
    return false;
}

QString IOSInterface::hotspotSSID()
{
    return QString("");
}

QString IOSInterface::hotspotPassword()
{
    return QString("");
}

void IOSInterface::openHotspot()
{
    NSString * urlString = @"App-Prefs:root=INTERNET_TETHERING";
    if ([[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:urlString]]) {
        if ([[UIDevice currentDevice].systemVersion doubleValue] >= 10.0) {
            [[UIApplication sharedApplication] openURL:[NSURL URLWithString:urlString] options:@{} completionHandler:nil];
        } else {
            [[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"prefs:root=WIFI"]];
        }
    }
}

void IOSInterface::keepScreenOn()
{
    [UIApplication sharedApplication].idleTimerDisabled = YES;
}

void IOSInterface::safeArea(SafeArea *t)
{
    if (@available(iOS 11.0, *)) {
            /*
             *  Qt api QScreen::availableGeometry()
             *  也能获取到安全区域
             */
            UIWindow *window = UIApplication.sharedApplication.windows.firstObject;
            t->top = (int)roundf(window.safeAreaInsets.top);
            t->bottom = (int)roundf(window.safeAreaInsets.bottom);
            t->left = (int)roundf(window.safeAreaInsets.left);
            t->right = (int)roundf(window.safeAreaInsets.right);
    }
}
