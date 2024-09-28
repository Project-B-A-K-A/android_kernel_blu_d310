#ifndef _DP_STREAM_H_
#define _DP_STREAM_H_

#include <DpChannel.h>
#include <DpUtils.h>
#include <DpDataType.h>

class DpControlProxy;

class DpStream
{
public:

    typedef KeyMap<DpChannel> PipeMap;
    typedef ListPtr<PipeMap> PipeMapList;

    typedef KeyMap<DpCallBack> CallBackNode;
    typedef ListPtr<CallBackNode> CallBackList;

    DpStream();
    ~DpStream();

    // register channel for this stream and return id
    int addChannel(DpChannel&);

    // remove channel from this stream
    bool removeChannel(int id);

    // set minimum require performance, ex. video play usually 30fps.
    void setMiniRequireFPS(int fps) { miniRequireFPS = fps; }

    void setVsyncSrc(DpVsyncSource src) { mVsyncSrc = src; }
    DpVsyncSource getVsyncSrc() { return mVsyncSrc; }
    
    bool setVsyncCallback(DpCallBack *c, DpCallbackType t);
    CallBackList& getVsyncCallback() { return mCallBackList; }
    
    // lock this stream and ensure hardware can do this scenario now
    bool lock(bool enTile = false);

    // unlock this stream resource
    bool unlock();

    // call invalidate to trigger hardware to render each frame
    bool invalidate(bool isSync);

    bool waitFrameDone(int wait_time_ms);

    void reset();

    void setStreamType(unsigned int t) { mStreamType = t; }

    PipeMapList* getPipeList() { return &mPipeMapList; }

    unsigned int getStreamType() { return mStreamType; }

    unsigned int getFlowCapacity() { return mFlowCapacity; }

    // for dump register
    void enableDumpReg(unsigned int flags){mDumpRegFlags = flags;}

    enum BLITSTREAM_MODULE_ENUM_
    {
        BLITSTREAM_MODULE_NONE  = 0x00,
        BLITSTREAM_MODULE_ROT   = 0x01,
        BLITSTREAM_MODULE_SCL   = 0x02,
        BLITSTREAM_MODULE_WDMA0 = 0x04
    };

    bool enTileMode;
    
private:

    // inital this stream
    bool init();

    //typedef ListPtr<DpStream> StreamList;

    unsigned int mStreamType;

    PipeMapList mPipeMapList;
    CallBackList mCallBackList;
    //StreamList mChildStreamList;

    int miniRequireFPS;
    int initIndex;

    unsigned int mFlowCapacity;
    DpControlProxy* mProxy;

    bool initDone;

	DpVsyncSource mVsyncSrc;

    int drvID;

    unsigned int mDumpRegFlags;
};

#endif
