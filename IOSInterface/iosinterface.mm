#include "iosinterface.h"

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

IOSInterface::IOSInterface()
{

}

IOSInterface::~IOSInterface()
{

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
