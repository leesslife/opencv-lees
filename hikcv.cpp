LONG EhikCV::nPort=-1;
volatile int EhikCv::gbHandling=3;
NET_DVR_PREVIEWINFO EhikCv::struPlayInfo={0};
void EhikCv::DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
    if (this->gbHandling)
    {
        this->gbHandling--;
        return;
    }
    long lFrameType = pFrameInfo->nType;
    if (lFrameType == T_YV12)
    {
        Mat pImg(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3);
        Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, pBuf);
        cvtColor(src, pImg, CV_YUV2BGR_YV12);
    //  Sleep(-1);
        imshow("IPCamera", pImg);
        waitKey(1);
    }
    this->gbHandling = 3;
}

void EhikCv::fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{
    switch (dwDataType)
    {
    case NET_DVR_SYSHEAD: //系统头
        if (!PlayM4_GetPort(&this->nPort))  //获取播放库未使用的通道号
        {
            break;
        }
        //m_iPort = lPort; //第一次回调的是系统头，将获取的播放库port号赋值给全局port，下次回调数据时即使用此port号播放
        if (dwBufSize > 0)
        {
            if (!PlayM4_SetStreamOpenMode(this->nPort, STREAME_REALTIME))  //设置实时流播放模式
            {
                break;
            }
            if (!PlayM4_OpenStream(this->nPort, pBuffer, dwBufSize, 10 * 1024 * 1024)) //打开流接口
            {
                break;
            }
            if (!PlayM4_Play(this->nPort, NULL)) //播放开始
            {
                break;
            }
            if (!PlayM4_SetDecCallBack(this->nPort, DecCBFun))
            {
                break;
            }

        }
        break;
    case NET_DVR_STREAMDATA:   //码流数据
        if (dwBufSize > 0 && this->nPort != -1)
        {
            if (!PlayM4_InputData(this->nPort, pBuffer, dwBufSize))
            {
                cout << "error" << PlayM4_GetLastError(this->nPort) << endl;
                break;
            }
        }
        break;
    default: //其他数据
        if (dwBufSize > 0 && this->nPort != -1)
        {
            if (!PlayM4_InputData(this->nPort, pBuffer, dwBufSize))
            {
                break;
            }
        }
        break;
    }
}

void EhikCv::g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
    char tempbuf[256] = { 0 };
    switch (dwType)
    {
    case EXCEPTION_RECONNECT:    //预览时重连
        printf("----------reconnect--------%d\n", time(NULL));
        break;
    default:
        break;
    }
}

void EhikCv::hikInit() 
{
    NET_DVR_Init();
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);
    this->struDeviceInfo={0};
    this->lUserID = NET_DVR_Login_V30("169.254.42.127", 8000, "admin", "keylab88", &this->struDeviceInfo);
    if (this->lUserID < 0)
    {
        printf("Login error, %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return;
    }
    NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);
    cvNamedWindow("Mywindow", 0);
    //cvNamedWindow("IPCamera", 0);
    this->h = (HWND)cvGetWindowHandle("Mywindow");
    if (h == 0)
    {
        cout << "窗口创建失败" << endl;
    }
    this->struPlayInfo.hPlayWnd = h;         //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
    this->struPlayInfo.lChannel = 1;           //预览通道号
    this->struPlayInfo.dwStreamType = 0;       //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
    this->struPlayInfo.dwLinkMode = 0;         //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
}
void EhikCv::playMat(){
    this->lRealPlayHandle = NET_DVR_RealPlay_V40(this->lUserID, &this->struPlayInfo, this->fRealDataCallBack, NULL);
    if (this->lRealPlayHandle < 0)
    {
        printf("NET_DVR_RealPlay_V40 error\n");
        printf("%d\n", NET_DVR_GetLastError());
        NET_DVR_Logout(this->lUserID);
        NET_DVR_Cleanup();
        return;
    }
    waitKey();
    Sleep(-1);
    return;
}
EhikCv::EhikCv(){
    printf("is OK?\n");
}
EhikCv::~EhikCv(){
    NET_DVR_StopRealPlay(this->lRealPlayHandle);
    NET_DVR_Logout(this->lUserID);
    NET_DVR_Cleanup();
}