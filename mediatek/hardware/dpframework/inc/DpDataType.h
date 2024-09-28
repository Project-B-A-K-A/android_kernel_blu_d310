#ifndef _DP_DATA_TYPE_H_
#define _DP_DATA_TYPE_H_

#include "DpBufferPool.h"

enum DpFeatureFlags
{
    eSCL_FLAG = 0x1,
    eROT_FLAG = 0x2,
    eOVL_FLAG = 0x4,
    eCOLOR_TRANFORM_FLAG = 0x8,
    eFLIP_FLAG = 0x10,
    eG2D_FLAG = 0x20,

    eBLS_FLAG = 0x100,
    eTTD_FLAG = 0x200,
    eSHARPNESS_FLAG = 0x400,
    eCOLOR_FLAG = 0x800,
    eDITHER_FLAG = 0x1000,
};

enum DpStreamType
{
    eNONE_STREAM,
    eOVERLAY_STREAM,
    eBITBLT_STREAM,
    eG2D_STREAM,
};

enum DpVsyncSource
{
    eNO_VSYNC = 0x0,
    eLCD0_VSYNC = 0x1,
    eLCD1_VSYNC = 0x2,
    eLCD2_VSYNC = 0x4,
    eLCD3_VSYNC = 0x8,
    eFRONT_CAM_VSYNC = 0x100,
    eBACK_CAM_VSYNC = 0x200
};

enum DpCallbackType
{
    eFRAME_DONE,
    eEVERY_VSYNC,
    eFRAME_UPDATE,      // before frame trigger
};

typedef void DpCallBack(void);

//FMT GROUP , 0-RGB , 1-YUV , 2-Bayer raw , 3-compressed format
#define DP_COLORFMT_PACK(PLANECNT , BITSPERPIXEL , FMTGROUP , UNIQUEID) ((PLANECNT << 8) | (BITSPERPIXEL << 12) | (FMTGROUP << 20) | UNIQUEID)
#define DP_PLANECNT(FMT) ((0xF00 & FMT) >> 8)
#define DP_BITSPERPIXEL(FMT) ((0xFF000 & FMT) >> 12)
#define DP_BYTESPERPIXEL(FMT) ((((0xFF000 & FMT) >> 12) + 4) >> 3)
#define DP_ISRGB(FMT) (0 == ((0xF0000 & FMT) >> 16) ? true : false)
#define DP_ISYUV(FMT) (1 == ((0xF0000 & FMT) >> 16) ? true : false)
#define DP_ISRAW(FMT) (2 == ((0xF0000 & FMT) >> 16) ? true : false)

enum DpColorFormat
{
    eY800 = DP_COLORFMT_PACK(1 , 8 , 1 , 0),
    eYUY2 = DP_COLORFMT_PACK(1 , 16 , 1 , 1),
    eUYVY = DP_COLORFMT_PACK(1 , 16 , 1 , 2),
    eYVYU = DP_COLORFMT_PACK(1 , 16 , 1 , 3),
    eVYUY = DP_COLORFMT_PACK(1 , 16 , 1 , 4),
    eYUV_444_1P = DP_COLORFMT_PACK(1 , 24 , 1 , 5),
    eYUV_422_I  = DP_COLORFMT_PACK(1 , 24 , 1 , 6),
    eYUV_422_I_BLK = DP_COLORFMT_PACK(1 , 24 , 1 , 7),

    eNV21 = DP_COLORFMT_PACK(2 , 12 , 1 , 10),
    eNV12 = DP_COLORFMT_PACK(2 , 12 , 1 , 11),
    eNV12_BLK = DP_COLORFMT_PACK(2 , 12 , 1 , 12),
    eNV12_BLK_FCM = DP_COLORFMT_PACK(2 , 12 , 1 , 13),
    eYUV_420_2P_ISP_BLK = eNV12_BLK,
    eYUV_420_2P_VDO_BLK = eNV12_BLK_FCM,
    eYUV_422_2P    = DP_COLORFMT_PACK(2 , 12 , 1 , 14),
    eYUV_444_2P    = DP_COLORFMT_PACK(2 , 12 , 1 , 15),
    eYUV_420_2P_UYVY = DP_COLORFMT_PACK(2 , 12 , 1 , 16),
    eYUV_420_2P_VYUY = DP_COLORFMT_PACK(2 , 12 , 1 , 17),
    eYUV_420_2P_YUYV = eNV21,
    eYUV_420_2P_YVYU = eNV12,

    
    eYV16 = DP_COLORFMT_PACK(3 , 16 , 1 , 20),
    eYV12 = DP_COLORFMT_PACK(3 , 12 , 1 , 21),
    eYV21 = DP_COLORFMT_PACK(3 , 12 , 1 , 22),
    eYUV_422_3P     = DP_COLORFMT_PACK(3 , 12 , 1 , 23),
    eYUV_444_3P     = DP_COLORFMT_PACK(3 , 12 , 1 , 24),
    eYUV_420_3P_YVU = eYV12,
    eYUV_420_3P     = eYV21,
    
    eRGB565 = DP_COLORFMT_PACK(1 , 16 , 0 , 30),  // 14
    eRGB888 = DP_COLORFMT_PACK(1 , 24 , 0 , 31),  // 15
    eARGB8888 = DP_COLORFMT_PACK(1 , 32 , 0 , 32),
    eABGR8888 = DP_COLORFMT_PACK(1 , 32 , 0 , 33),

    eRGBA8888 = DP_COLORFMT_PACK(1 , 32 , 0 , 34),
    eBGRA8888 = DP_COLORFMT_PACK(1 , 32 , 0 , 35),

    ePARGB8888 = DP_COLORFMT_PACK(1 , 32 , 0 , 36),
    eXARGB8888 = DP_COLORFMT_PACK(1 , 32 , 0 , 37),

    eCompactRaw1 = DP_COLORFMT_PACK(1 , 10 , 2 , 60),
    eMTKYUV = DP_COLORFMT_PACK(1 , 32 , 0 , 61)
};

enum DpInterlaceFormat
{
    eInterlace_None,
    eTop_Field,
    eBottom_Field
};

struct DpRect
{
    int x;
    int y;
    int w;
    int h;
};

struct DpPortOption
{
    int width;
    int height;
    DpRect ROI;
    DpColorFormat format;
    DpInterlaceFormat interlace;

    enum DpPort
    {
        eLCD0_PORT,
        eLCD1_PORT,
        eHDMI_PORT,
        eTVOUT_PORT,
        eOVERLAY_PORT,
        eVIRTUAL_PORT,
        eMEMORY_PORT
    };

    DpPort port;

    int overlayID;              // setting if choose port = eOVERLAY
    int virtualID;              // setting if choose port = eVIRTUAL_PORT
    DpBufferPool *buffer;       // setting if choose port = eMEMORY
};

#endif
