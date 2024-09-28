#ifndef _DP_CHANNEL_H_
#define _DP_CHANNEL_H_

#include <DpBufferPool.h>
#include <DpDataType.h>
#include <DpUtils.h>

class DpChannel
{
public:
    DpChannel() : functionFlag(0), engineFlag(0), rotateDegree(0), srcPort(0), dstPort(0), srcBuffer(0), dstBuffer(0) {}
    ~DpChannel();

    bool setDstPort(DpPortOption&);
    bool setSrcPort(DpPortOption&);

    DpBufferPool* getSrcBuffer() { return srcBuffer; }
    DpBufferPool* getDstBuffer() { return dstBuffer; }
    DpPortOption* getSrcPort() { return srcPort; }
    DpPortOption* getDstPort() { return dstPort; }

    bool setFeatureArg(DpFeatureFlags f, void* arg);
    void* getFeatureArg(DpFeatureFlags f);
    
    void setFunctionFlag (unsigned int f) { functionFlag = f; }
    void setEngineFlag (unsigned int f) { engineFlag = f; }
    void setExtConfig (void *ext) { pExConfig = ext; }
    void setRotateDegree(int rot) { rotateDegree = rot; }
    void setFlip(int flip_value) { flip = flip_value; }
    
    unsigned int getFunctionFlag() { return functionFlag; }
    unsigned int getEngineFlag() { return engineFlag; }
    int getRotateDegree() { return rotateDegree; }
    int getFlip() { return flip; }
    bool queryFunction(unsigned int f) { return ((f & functionFlag) > 0) ;}
    void debugInfo();
    void recycle();
    bool operator==(const DpChannel& rhs) const { 
            return this == &rhs; }
            
private:
    typedef KeyMap<void> ArgNode;
    typedef ListPtr<ArgNode> ArgList;
    
    unsigned int functionFlag;
    unsigned int engineFlag;
    void* pExConfig;
    int rotateDegree;
    int flip;
    DpPortOption*   srcPort;
    DpPortOption*   dstPort;
    DpBufferPool*  srcBuffer;
    DpBufferPool*  dstBuffer;

    ArgList mArgList;
};

#endif
