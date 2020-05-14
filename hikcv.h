#ifndef HIKCV_H
#define HIKCV_H
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "Windows.h"
#include "HCNetSDK.h"
#include "plaympeg4.h"
#include <opencv2/opencv.hpp>
#include <time.h>
using namespace std;
using namespace cv;
class EhikCv{
    private:
        static LONG nPort;
        static volatile int gbHandling;
        void DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
        void fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser);
        void g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser);
        LONG lUserID;
        LONG lRealPlayHandle;
        static NET_DVR_PREVIEWINFO struPlayInfo;
        NET_DVR_DEVICEINFO_V30 struDeviceInfo;
    public:
        void hikInit();
        EhikCv();
        ~EhikCv();
};
#endif