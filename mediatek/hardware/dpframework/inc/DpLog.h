#ifndef _DP_LOG_
#define _DP_LOG_

#include <DpDebug.h>
#include <DpOS.h>

#if WIN32
#define LOGD(...) DpOS::instance()->getDebug()->log(DpDebug::eDebug, __VA_ARGS__)
#define LOGI(...) DpOS::instance()->getDebug()->log(DpDebug::eInfo, __VA_ARGS__)
#define LOGW(...) DpOS::instance()->getDebug()->log(DpDebug::eWarning, __VA_ARGS__)
#define LOGE(...) DpOS::instance()->getDebug()->log(DpDebug::eError, __VA_ARGS__)
#define LOGP(...) DpOS::instance()->getDebug()->log(DpDebug::eProfile, __VA_ARGS__)

#define XLOGE   LOGE
#define XLOGW   LOGW
#define XLOGI   LOGI
#define XLOGD   LOGD
#define XLOGP   LOGP
#else

#include <cutils/xlog.h>

#define LOGD    XLOGD
#define LOGI    XLOGI
#define LOGW    XLOGW
#define LOGE    XLOGE

#define XLOGP   XLOGI

#undef LOG_TAG
#define LOG_TAG "DISPSYS"

#endif

#endif