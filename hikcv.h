#ifndef HIKCV_H
#define HIKCV_H
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "HCNetSDK.h"
#include "plaympeg4.h"
#include <opencv2/opencv.hpp>
#include <time.h>
using namespace std;
using namespace cv;
#include "PlayM4.h"
#include "LinuxPlayM4.h"
#include "unistd.h"
class EhikCv
{
    private:
        static LONG nPort;
        static volatile int gbHandling;
        HWND h;
        void DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2);
        void fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser);
        void g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser);
        LONG lUserID;
        LONG lRealPlayHandle;
        static NET_DVR_PREVIEWINFO struPlayInfo;
        NET_DVR_DEVICEINFO_V30 struDeviceInfo;
    public:
        void hikInit();
        void playMat();
        EhikCv();
        ~EhikCv();
        
};
LONG EhikCv::nPort=-1;
volatile int EhikCv::gbHandling=3;
NET_DVR_PREVIEWINFO EhikCv::struPlayInfo={0};
#endif