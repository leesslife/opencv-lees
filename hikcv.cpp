#include "hikcv.h"
static Mat dst;
HWND h;
LONG nPort=-1;
LONG lUserID;

pthread_mutex_t mutex;
list<Mat> g_frameList;

FILE *g_pFile=NULL;
void CALLBACK PsDataCallBack(LONG lRealHandle,DWORD dwDataType,BYTE *pPacketBuffer,DWORD nPacketSize,void *pUser)
{
    if(dwDataType ==NET_DVR_SYSHEAD)
    {   
        g_pFile=fopen("/home/ps.dat","wb");
        if(g_pFile==NULL)
        {
            printf("CreateFileHead fail\n");
            return;
        }
        fwrite(pPacketBuffer,sizeof(unsigned char),nPacketSize,g_pFile);
        printf("write head len=%d\n",nPacketSize);
    }
    else
    {
        if(g_pFile!=NULL)
        {
            fwrite(pPacketBuffer,sizeof(unsigned char),nPacketSize,g_pFile);
            printf("write data len=%d\n",nPacketSize);
        }
    }
}
void CALLBACK DecCBFun(int nPort, char * pBuf, int nSize, FRAME_INFO * pFrameInfo, void* nReserved1, int nReserved2)
{
    long lFrameType=pFrameInfo->nType;
    if (lFrameType == T_YV12)
    {
        dst.create(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3);
        Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight / 2, pFrameInfo->nWidth, CV_8UC1, pBuf);
        cvtColor(src, dst, CV_YUV2BGR_YV12);
        pthread_mutex_lock(&mutex);
        g_frameList.push_back(dst);
        pthread_mutex_unlock(&mutex);
    }
    usleep(1000);
}

void CALLBACK g_RealDataCallBack_V30(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{
    DWORD dRet;
    switch (dwDataType)
    {
    case NET_DVR_SYSHEAD: //系统头
        if (!PlayM4_GetPort(&nPort))  //获取播放库未使用的通道号
        {
            break;
        }
        //m_iPort = lPort; //第一次回调的是系统头，将获取的播放库port号赋值给全局port，下次回调数据时即使用此port号播放
        if (dwBufSize > 0)
        {
            if (!PlayM4_SetStreamOpenMode(nPort, STREAME_REALTIME))  //设置实时流播放模式
            {
                dRet=PlayM4_GetLastError(nPort);
                break;
            }
            if (!PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 1024 * 1024)) //打开流接口
            {
                dRet=PlayM4_GetLastError(nPort);
                break;
            }
            if (!PlayM4_Play(nPort, h)) //播放开始
            {
                dRet=PlayM4_GetLastError(nPort);
                break;
            }
            if (!PlayM4_SetDecCallBack(nPort, DecCBFun))
            {   
                dRet=PlayM4_GetLastError(nPort);
                break;
            }
            if(!PlayM4_PlaySound(nPort)){
                dRet=PlayM4_GetLastError(nPort);
                break;
            }
        }
        break;
        case NET_DVR_STREAMDATA:
            if(dwBufSize>0&&nPort!=-1){
                BOOL inData=PlayM4_InputData(nPort,pBuffer,dwBufSize);
                while (!inData){
                    sleep(100);
                    inData=PlayM4_InputData(nPort,pBuffer,dwBufSize);
                    cerr<<"PlayM4_InputData failed \n"<<std::endl;
                }
            }
            break;
        }
}

void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
    char tempbuf[256] = { 0 };
    cout<<"EXCEPTION_RECONNECT="<<EXCEPTION_RECONNECT<<endl;
    switch (dwType)
    {
    case EXCEPTION_RECONNECT:    //预览时重连
        printf("----------reconnect--------%d\n", time(NULL));
        break;
    default:
        break;
    }
}

void *hikInit(void *) 
{
    char IP[]="192.168.1.105";
    char UName[]="admin";
    char PSW[]="admin";
    NET_DVR_Init();
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(1000, true);
    char logdir[]="./sdkLog";
    NET_DVR_SetLogToFile(3,logdir);
    NET_DVR_DEVICEINFO_V30 struDeviceInfo={0};
    lUserID = NET_DVR_Login_V30(IP, 80, UName, PSW, &struDeviceInfo);
    if (lUserID < 0)
    {
        printf("Login error, %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
    }
    NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);
    long lRealPlayHandle;
    NET_DVR_CLIENTINFO ClientInfo={0};
    ClientInfo.lChannel = 1;           //预览通道号
    ClientInfo.lLinkMode = 0;       //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
    ClientInfo.hPlayWnd = 0;         //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
    ClientInfo.sMultiCastIP=NULL;
    lRealPlayHandle=NET_DVR_RealPlay_V30(lUserID,&ClientInfo,g_RealDataCallBack_V30,NULL,0);
    if(lRealPlayHandle<0);
    {
        printf("pyd1---NET_DVR_RealPlay_V30 error \n");
    }
    sleep(-1);
    NET_DVR_Cleanup();
}
int playMat(){
    pthread_t getframe;
    pthread_mutex_init(&mutex,NULL);
    int ret;
    ret=pthread_create(&getframe,NULL,hikInit,NULL);
    if(ret!=0)
    {
        printf("Create pthread error!\n");
    }
    cv::Mat image;
    while(1)
    {
        pthread_mutex_lock(&mutex);
        if(g_frameList.size())
        {
            list<Mat>::iterator it;
            it=g_frameList.end();
            it--;
            image=(*(it));
            if(!image.empty())
            {
                imshow("frame from camera",image);
                waitKey(1);
            }
            g_frameList.pop_front();
        }
        g_frameList.clear();
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}
