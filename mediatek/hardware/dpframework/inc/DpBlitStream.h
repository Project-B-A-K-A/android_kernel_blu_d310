#include <DpStream.h>
#include <DpChannel.h>
#include <DpDataType.h>

#ifndef _DP_BLIT_STREAM_H_
#define _DP_BLIT_STREAM_H_


class DpBlitStream
{
public:
    DpBlitStream();
    ~DpBlitStream();

    enum DpOrientation {
        ROT_0   = 0x00000000,
        FLIP_H  = 0x00000001,
        FLIP_V  = 0x00000002,
        ROT_90  = 0x00000004,
        ROT_180 = FLIP_H|FLIP_V,
        ROT_270 = ROT_180|ROT_90,
        ROT_INVALID = 0x80
    };

    // set src buffer address list, size list, list size (plan number) & memory type (defined in DpBufferPool.h)
    // set src buffuer has 3 overloading function
    // 1. for only one plan and memory type can use eNormal(default) or ePHY
    void setSrcBuffer(void* addr, unsigned int size, int type = 0);
    // 2. for multi-plan used(one plan also work) and memory type can use eNormal(default) or ePHY
    void setSrcBuffer(void** addrList, unsigned int *sizeList, unsigned int planeNumber, int type = 0);

    // 3. for ion buffer only (must pass ion fd)    
    void setSrcBuffer(int fd, unsigned int *sizeList, unsigned int planeNumber);

    // set src buffer attribute including width, hiehgt, format, interlace and roi
    // here width and height should be buffer w/h stride
    // format defined in DpDataType.h
    // interlace default is eInterlace_None and can be eTop_Field or eBottom_Field for block YUV format
    // roi is the really rectangle want to do blit
    void setSrcConfig(int w, int h, DpColorFormat color, DpInterlaceFormat i = eInterlace_None, DpRect* ROI = 0);

    // set dst buffer address list, size list, list size (plan number) & memory type (defined in DpBufferPool.h)
    // dst basic usage way is the same with src buffer
    void setDstBuffer(void* addr, unsigned int size, int type = 0);
    void setDstBuffer(void** addrList, unsigned int *sizeList, unsigned int planeNumber, int type = 0);

    // for ion fd
    void setDstBuffer(int fd, unsigned int *sizeList, unsigned int planeNumber);

    // set dst buffer attribute including width, hiehgt, format, interlace and roi
    void setDstConfig(int w, int h, DpColorFormat color, DpInterlaceFormat i = eInterlace_None, DpRect* ROI = 0);

    void setRotate(int rot) { mRotate = rot; }
    void setFlip(int flip) { mFlip = flip; }
    void setOrientation(const unsigned int transform);

    void setTdshp(int gain) { mTdshp = gain; }

    bool invalidate();

    // for dump register
    void enableDumpReg(unsigned int flags){mDumpRegFlags = flags;}

    static bool queryHWSupport(unsigned int src_w,
              unsigned int src_h,
              unsigned int dst_w,
              unsigned int dst_h,
              DpOrientation orientation = ROT_0);

private:
    int mRotate;
    int mFlip;
    int mTdshp;

    DpStream    *mStream;
    DpChannel   *mChannel;
    DpBufferPool *mSrcPool;
    DpBufferPool *mDstPool;
    DpPortOption *mSrcPort;
    DpPortOption *mDstPort;

    int mSrcBufferId;
    int mDstBufferId;

    unsigned int mDumpRegFlags;
};

#endif
