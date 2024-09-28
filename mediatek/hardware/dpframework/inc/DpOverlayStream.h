
#include <DpStream.h>
#include <DpChannel.h>
#include <DpDataType.h>

#ifndef _DP_OVERLAY_STREAM_H_
#define _DP_OVERLAY_STREAM_H_

#define MAX_LAYER_NUM 4

class DpOverlayStream
{
public:
    static DpOverlayStream* getInstance();

    struct LayerParam
    {
        int srcWidth;
        int srcHeight;
        int rotDegree;

        DpBufferPool* srcBufferPool;
        DpColorFormat colorFormat;
        DpInterlaceFormat interlace;

        DpRect srcROI;

        // the output dimension is always LCD size
        // so we only need to set the ROI of panel size for this layer
        DpRect overlayROI;
    };

    int queryAvailableLayerNum();
    bool queryAvailableLayer(int layerId) { return (mOverlayChannel[layerId] == 0); }
    void queryDisplaySize(int *width, int *height);

    bool setLayer(int layerID, LayerParam& param);
    bool disableLayer(int layerID);

    bool setNormalDisplay(bool isEnable);
    bool setExternDisplay(bool isEnable);
    bool setDumpDisplay(bool isEnable, void* addr = 0);
    
    //bool setOutputMemArgs(void* addr, int width, int height, DpColorFormat format);
    
    bool setBLS(bool isEnable);
    bool setCOLOR(bool isEnable);

    bool setAAL(unsigned int backlight, unsigned int *adaptiveLuma);
    bool getLumaHistogram(unsigned int *histogram);
    
    bool invalidate();
    bool stop();

private:
    static DpOverlayStream *_instance;

    DpOverlayStream();
    ~DpOverlayStream();

    enum STREAM_STATUS
    {
        eSTOP = 0x0,
        eRUN = 0x1
    };

    unsigned int mStatus;

    int mAvailableLayer;

    DpStream* mOvlStream;

    DpStream*   mBlitStream[MAX_LAYER_NUM];
    DpChannel*  mBlitbltChannel[MAX_LAYER_NUM];
    void*       mBlitAddr[MAX_LAYER_NUM];

    DpChannel*   mOverlayChannel[MAX_LAYER_NUM];
    DpChannel*   mDisplayChannel;

    DpChannel*   mOutMemChannel;

    DpChannel*   mExtDispChannel;

    // used for kernel layer
    DpBufferPool* mLayerPool[MAX_LAYER_NUM];

    LayerParam* mLayerParam[MAX_LAYER_NUM];
    
    int mDisplayCID;
    int mExtDispCID;
    int mOutMemCID;
    int mOverlayCID[MAX_LAYER_NUM];

    bool enBLS;
    bool enCOLOR;
    bool enNorDisp;
    bool enExtDisp;
    bool enOutMem;

    void addBlitChannel(int layerID, LayerParam& param);
    void checkKernelLayer();
};

#endif

