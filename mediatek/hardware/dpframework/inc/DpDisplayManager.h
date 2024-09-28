#ifndef _DP_DISPLAY_MANAGER_H_
#define _DP_DISPLAY_MANAGER_H_

#include <DpStream.h>
#include <DpDataType.h>

// ---------------------------------------------------------------------------

class DpDisplayManager
{
public:
    static DpDisplayManager* getInstance();

    int getMaxDisplayNum();

    int getCurrDisplayNum();

    struct DisplayData
    {
        uint32_t width;
        uint32_t height;
        float xdpi;
        float ydpi;
        int64_t refresh;

        DpColorFormat format;
        DpPortOption::DpPort port;

        bool enabled;
    };
    DisplayData* mData;

private:
    DpDisplayManager();
    ~DpDisplayManager() { }

    static DpDisplayManager* instance_;
};

#endif //_DP_DISPLAY_MANAGER_H_
