LD_Opencv=-lopencv_calib3d -lopencv_features2d -lopencv_imgcodecs -lopencv_objdetect -lopencv_stitching -lopencv_video -lopencv_core -lopencv_flann -lopencv_imgproc -lopencv_photo -lopencv_superres -lopencv_videostab -lopencv_dnn -lopencv_highgui -lopencv_ml -lopencv_shape -lopencv_videoio 
LD_HCNETSDK=-lanalyzedata -lHCCoreDevCfg -lHCGeneralCfgMgr -lHCPlayBack -lHCVoiceTalk -lStreamTransClient -lHCAlarm -lHCDisplay -lHCIndustry -lHCPreview -lSystemTransform -lAudioRender -lcrypto -lhcnetsdk -lNPQos -lPlayCtrl -lSuperRender -lHCCore -lhpr
LD_PLAYM4=-lAudioRender -lSuperRender -lPlayCtrl 
LD_DIR=-L/home/lees/M_Paper/opencvyolov3/opencv-lees/PlayCtril -L/home/lees/M_Paper/opencvyolov3/opencv-lees/HCNET/lib -L/home/lees/M_Paper/opencvyolov3/opencv-lees/HCNET/lib/HCNetSDKCom
CPP=g++
CC=gcc

testopencv:hikcv.cpp testHikVision.cpp
	$(CPP) -o testopencv hikcv.cpp testHikVision.cpp $(LD_DIR) $(LD_PLAYM4) $(LD_HCNETSDK) $(LD_Opencv) -pthread