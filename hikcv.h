#ifndef HIKCV_H
#define HIKCV_H
#include <fstream>
#include "unistd.h"
#include "HCNetSDK.h"
#include "opencv2/opencv.hpp"
#include <time.h>
#include "PlayM4.h"
#include "LinuxPlayM4.h"

using namespace std;
using namespace cv;

#define HPR_ERROR  -1
#define HPR_OK      0
#define USECOLOR    0


void CALLBACK DecCBFun(int nPort, char * pBuf, int nSize, FRAME_INFO * pFrameInfo, void* nReserved1, int nReserved2);
void CALLBACK PsDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser);
void CALLBACK g_RealDataCallBack_V30(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser);
void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser);
void hikInit();
int playMat();
#endif