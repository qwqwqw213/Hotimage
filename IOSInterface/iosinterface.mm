#include "iosinterface.h"

#include <UIKit/UIKit.h>

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
