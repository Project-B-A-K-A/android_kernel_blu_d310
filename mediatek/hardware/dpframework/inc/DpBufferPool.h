#ifndef _DP_BUFFER_POOL_
#define _DP_BUFFER_POOL_

#include <DpUtils.h>

class DpMemoryProxy;

class DpBufferPool
{
public:
    DpBufferPool();
    ~DpBufferPool();

    enum BufferType
    {
        eNone,
        eOnePlane,
        eTwoPlane,
        eThreePlane

    };

    enum BufferStatus
    {
        eFREE,
        eQUEUED,
        eDEQUEUED,
        eDONE
    };

    enum MemoryType
    {
        eNormal,
        eION,       
        ePHY,        // only have physical address
        eMva
    };

    enum
    {
        eInvalidID = -1,
    };
    
    bool setBufferType(BufferType t);
    void setMemoryType(MemoryType t) { memoryType = t; }

    BufferType getBufferType() { return bufferType; }
    MemoryType getMemoryType() { return memoryType; }

    // register buffer for this memory pool
    int registerBuffer(unsigned int addr, unsigned int size); // return register id
    int registerBuffer(unsigned int *addr, unsigned int *size); // return register id

    // register buffer by fd, for ION
    int registerBufferFD(int fd, unsigned int size);
    int registerBufferFD(int fd, unsigned int *size);
    int registerBufferFD(int *fd, unsigned int *size);
    
    // will return the id of the avaliable buffer
    int dequeueBuffer();

    // queue the buffer, the dp framework will render the buffer in queue sequence
    bool queueBuffer(int id);

    /* remove this api
    inline bool setActiveBuffer(int id) { 
        if(findBuffer(id) != 0) {
            activeID = id;
            return true;
        }
        return false;
    }*/
    
    bool unregisterBuffer(int id);

    // pop the buffer from the queue and the buffer will be marked as FREE
    bool popActiveBuffer();
    bool getActiveConfigAddr(unsigned int *addr, unsigned int *size);
    bool getActiveCpuAddr(unsigned int *addr, unsigned int *size);

    void setActiveBufferDone();
    void setBufferEngineType(unsigned int t) { engineType = t; }
    void setOverlayLayer(int layer) { mLayer = layer; }
    
    void flushActiveReadBuffer();
    void flushActiveWriteBuffer();
    
private:
    class BufferNode 
    {

    public:
        BufferNode(int c);
        ~BufferNode();
        unsigned int addr[3];
        unsigned int size[3];
        int fd[3];
        DpMemoryProxy* memProxy[3];
        BufferStatus status;

        int count;
        bool isDecorateProxy;
        
        inline bool operator==(const BufferNode& rhs) {
            return (this == &rhs);
        }

        inline void release() { delete this; }
        
    };

    typedef KeyMap<BufferNode>  MAP;
    typedef ListPtr<MAP> LIST;

    BufferNode* findBuffer(int id);
    bool decorateProxy(BufferNode* node);
    
    LIST mBufferList;
    LIST activeQueue;

    int initKey;
    int activeID;
    unsigned int engineType;
    int mLayer;
    BufferType bufferType;
    MemoryType memoryType;

    
};

#endif
