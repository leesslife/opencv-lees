#ifndef EYOLOV3_H
#define EYOLOV3_H
#include<fstream>
#include<sstream>
#include<iostream>
#include<string>
#include"opencv2/opencv.hpp"
#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>
using namespace cv;
using namespace dnn;
using namespace std;
class Eyolo{
    private:
        float confThreshold;//Confidence threshold
        float mnsThreshold;//Non-maximum suppression threshold
        int inpWidth;
        int inpHeight;
        vector<string> classes;
        string classesFile;
        string modelConfiguration;
        string modelWeights;
        Net net;
    public:
        Eyolo();
        ~Eyolo();
        int postprocess(Mat& frame,const vector<Mat>& out);
        void drawPred(int classId,float conf,int left,int top,int right,int bottom,Mat& frame);
        vector<String> getOutputNames(const Net& net);
        void yolovInit(tring vocname,string modelConfiguration,string modelWeight);
        void yolovplay();
        int yolovplayOnce(Mat inputFrame);
};
#endif
