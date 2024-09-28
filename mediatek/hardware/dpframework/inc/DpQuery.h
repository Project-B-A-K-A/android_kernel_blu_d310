#ifndef _DP_QUERY_H_
#define _DP_QUERY_H_

enum DpCode
{
    DISPLAY_DEVICE_CODE,
    DISPLAY_INFO_CODE,    
    OVERLAY_INFO_CODE,
    LUMA_HISTOGRAM_CODE,
};

// for DISPLAY_DEVICE_CODE
struct DpDispDevice
{
    int id;
    
    int w;
    int h;
    int dpiX;
    int dpiY;
    
    int format;     // DpColorFormat
    int vsyncPeriod;
};

// for DISPLAY_INFO_CODE
struct DpDispInfo
{
    int number;
    int *deviceIdList;
};

// for OVERLAY_CODE
struct DpOverlayInfo
{
    int freeLayerNum;
    int *freeLayerIdList;
};

struct DpLumaHistogram
{
    unsigned int *histogram;
};

//--------------------------------------------------------
// Query Function for Dp Framework
//----------------------------------------------------------
bool dpQuery(DpCode code, void*);

#endif