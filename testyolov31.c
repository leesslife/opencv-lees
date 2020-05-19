#include<fstream>
#include<sstream>
#include<iostream>

#include "opencv2/dnn.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

const char* keys=
"{help h usage ? | | Usage examples: \n\t\t./object_detection_yolo.out --image=dog.jpg \n\t\t./object_detection_yolo.out --video=run_sm.mp4}"
"{image i        |<none>| input image   }"
"{video v       |<none>| input video   }"
;
//这里的组件定义了命令行参数
//1||2||3
//1：代表命令行前缀
//2：代表默认参数
//3：代表提示

using namespace cv;
using namespace dnn;
using namespace std;

float confThreshold=0.5//Confidence threshold
float mnsThreshold=0.4//Non-maximum suppression threshold
int inpWidth=416;
int inpHeight=416;
vector<string> classes;
//类字符串容器
// Remove the bounding boxes with low confidence using non-maxima suppression
void postprocess(Mat& frame,const vector<Mat>& out);
// Draw the predicted bounding box
void drawPred(int classId,float conf,int left,int top,int right,int bottom,Mat& frame);
// Get the names of the output layers
vector<String> getOutputNames(const Net& net);

int main(int argc,char** argv)
{
    CommandLineParser parser(argc,argv,keys);
    parser.about("Use this script to run object detection using YOLO3 in OpenCV.");
    if(parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    //Load names of classes
    string classesFile="coco.names"
    ifstream ifs(classesFile,c_str());
    string line;
    while(getline(ifs,line)) classes.push_back(line);
    //Give the configuration and weight files for model;
    String modelConfiguration="yolov3.cfg";
    String modelWeights="yolov3.weights";

    //Load the network
    Net net=readNetFromDarknet(modelConfiguration,modelWeights);
    net.setPreferableBackend(DNN_BACKEND_OPENCV);
    net.setPreferableTarget(DNN_TARGET_CPU);

    //open a video file or an image file or a camera stream.
    string str,outputFile;
    VideoCapture cap;
    VideoWriter video;
    Mat frame,blob;
    try{
        outputFile="yolo_out_cpp.avi";
        if(parser.has("image"))
        {
            //open the image file
            str=parser.get(<String>("iamge");
            ifstream ifile(str);
            if(!ifile) throw("error");
            cap.open(str);
            str.replace(str.end()-4,str.end(),"_yolo_out_cpp.jpg");
            outputFile=str;
        }
        else if(parser.has("video"))
        {
            //open the video file
            str=parser.get<String>("video");
            ifstream ifile(str);
            if(!file) throw("error");
            cap.open(str);
            str.replace(str.end()-4,str.end(),"_yolo_out_cpp.avi");
            outputFile=str;
        }
        //open the webcaom
        else cap.open(parser.get<int>("device"));
    }
    catch(...){
        cout<< "Could not open the input image/video stream"<<endl;
        return 0;
    }
    //Get the video writer initialized to save the  output video
    if(!parser.has("image")){
        video.open(outputFile,VideoWriter::fourcc('M','J','P','G'),28,Size(cap.get(CAP_PROP_FRAME_WIDTH),cap.get(CAP_PROP_FRAME_HEITHT)));
    }
    //Create a window
    static const string kWinName="Deep learning object detection in OpenCV";
    namedWindow(kWinName,WINNDOW_NORMAL);
    while(waitKey(1)<0)
    {
        //get frame from the video
        cap>>frame;
        //Stop the program if reached end of video
        if(frame.empty()){
            cout<<"Done processing!!!"<<end;
            cout<<"Output file is stroed as"<<outputFile<<endl;
            waitKey(3000);
            break;
        }
        //Create 2 4D blob from a frame.
        blobFromImage(frame,blob,1/255.0,cvSize(inpWidth,inpHeight),Scalar(0,0,0),true,false);
        //blobFromImage
        //1)input image
        //2)output image
        //3)1/255.0 像素值的比例缩放
        //4)输入神经网络的图片的尺寸
        //5)各个通道的 减去的均值
        //6)swapRB
        //7)crop
        //Sets the input to the network
        net.setInput(blob);
        // Runs the forward pass to get output of the output layers
        vector<Mat> outs;
        net.forward(outs,getOutputsNames(net));
        // Remove the bounding boxes with low confidence
        postprocss(frame,outs);
        // Put efficiency information. The function getPerfProfile returns the overall time for inference(t) and the timings for each of the layers(in layersTimes)
        vector<double> layersTimes;
        double freq=getTickFrequency()/1000;
        double t=net.getPerfProfile(layersTimes)/freq;
        string label=format("Inference time for a frame:%.2f ms",t);
        putText(frame,label,Point(0,15),FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255));
        //Write the frame with the detection boxes
        Mat detectedFrame;
        frame.convertTo(detectedFrame,CV_8U);
        if(parser.has("image")) imwrite(ouputFile,detectedFrame);
        else video.write(detectedFrame);
        inshow(kWinName,frame);
    }
    cap.release();
    if(!parser.has("image")) video.release();
    return 0;
}

//// Remove the bounding boxes with low confidence using non-maxima suppression
void postprocess(Mat& frame,const vector<Mat>& outs)
{
    vector<int> classIds;
    vector<float> confidences;
    vector<Rect> boxes;
    for(size_t i=0;i<outs.size();++i)
    {
        // Scan through all the bounding boxes output from the network and keep only the
        // ones with high confidence scores. Assign the box's class label as the class
        // with the highest score for the box.
        float* data = (float*)outs[i].data;
        //指向Mat数据区 //yolov3的outs[i]有三个
        for(int j=0;j<outs[i].rows;++j,data+=outs[i].cols)
        {
            Mat scores=outs[i].row(j).colRange(5,outs[i].cols);
            Point classIdPoint;
            double confidence;
            // Get the value and location of the maximum score
            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            //minMaxLoc(inpMat,minval&,maxval&,Point& minloc,Point& maxLoc)
            if(confidence>confThreshold)
            {
                int centerX=(int)(data[0]*frame.cols);
                int centerY=(int)(data[1]*frame*rows);
                int width=(int)(data[2]*frame.cols);
                int height=(int)(data[3]*frame.rows);
                int left=centerX-width/2;
                int top=centerY-height/2;

                classIds.push_back(classIdPoint.x);
                confidence.push_back((float)confidence);
                boxes.push_back(Rect(left,top,width,height));

            }

        }
    }
    vector<int> indices;
    NMSBoxes(boxes,confidences,confThreshold,nmsThreshold,indices);
    for(size_t i=0;i<indices.size();++i)
    {
        int idx=indices[i];
        Rect box=boxes[idx];
        drawPred(classIds[idx],confidences[idx],box.x,box.y,box.x+box.width,box.y+box.height,frame);
    }
}

void drawPred(int classId,float conf,int left,int top,int right,int bottom,Mat& frame)
{
    //Draw a rectangle displaying the bounding box
    rectangle(frame,Point(left,top),Point(right,bottom),Scalar(255,178,50),3);
    //Get the label for the class name and its confidence
    string label=format("%.2f",conf);
    if(!classes.empty())
    {
        CV_Assert(classId <(int)classes.size());
        label=class[classId]+":"+label;
    }
    //Display the label at the top of the bounding box
    int baseLine;
    Size labelSize=getTextSize(label,FONT_HERSHEY_SIMPLEX,0.5,1,&baseLine);
    top=max(top,labelSize.height);
    rectangle(frame, Point(left, top - round(1.5*labelSize.height)), Point(left + round(1.5*labelSize.width), top + baseLine), Scalar(255, 255, 255), FILLED);
    putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,0),1);
}