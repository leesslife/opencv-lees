#include "Eyolov3.h"

Eyolo::Eyolo()
{
    this->confThreshold=0.5;//Confidence threshold
    this->mnsThreshold=0.4;//Non-maximum suppression threshold
    this->inpWidth=416;
    this->inpHeight=416;
    this->classesFile="coco.names";
    this->modelConfiguration="yolov3.cfg";
    this->modelWeights="yolov3.weights";
}
Eyolo::~Eyolo(){}
void Eyolo::yolovInit(string vocname,string modelConfiguration,string modelWeight)
{

    this->classesFile=vocname;
    ifstream ifs(classesFile.c_str());
    string line;
    while(getline(ifs,line)) this->classes.push_back(line);
    //Give the configuration and weight files for model;
    this->modelConfiguration=modelConfiguration;
    this->modelWeights=modelWeight;

    //Load the network
    this->net=readNetFromDarknet(this->modelConfiguration,this->modelWeights);
    this->net.setPreferableBackend(DNN_BACKEND_OPENCV);
    this->net.setPreferableTarget(DNN_TARGET_CPU);
}
void Eyolo::yolovplay(){
    VideoCapture cap;
    Mat frame,blob;
    try{
       cap.open(0);
    }
    catch(...){
        cout<< "Could not open the input image/video stream"<<endl;
    }
    static const string kWinName="Deep learning object detection in OpenCV";
    namedWindow(kWinName,WINDOW_NORMAL);
    while(waitKey(1)<0)
    {
        //get frame from the video
        cap>>frame;
        //Stop the program if reached end of video
        if(frame.empty()){
            cout<<"Done processing!!!"<<endl;
            waitKey(3000);
            break;
        }
        blobFromImage(frame,blob,1/255.0,cvSize(inpWidth,inpHeight),Scalar(0,0,0),true,false);
        this->net.setInput(blob);
        vector<Mat> outs;
        this->net.forward(outs,this->getOutputsNames(net));
        this->postprocess(frame,outs);
        Mat detectedFrame;
        frame.convertTo(detectedFrame,CV_8U);
        inshow(kWinName,frame);
    }
    cap.release();
}
int Eyolo::yolovplayOnce(Mat inputFrame){
    Mat blobOnce;
    blobFromImage(inputFrame,blobOnce,1/255.0,cvSize(inpWidth,inpHeight),Scalar(0,0,0),true,false);
    this->net.setInput(blobOnce);
    vector<Mat> outs;
    this->net.forward(outs,getOutputsNames(net));
    int fire_num=this->postprocess(inputFrame,outs);
    return fire_num;
}
//// Remove the bounding boxes with low confidence using non-maxima suppression
int Eyolo::postprocess(Mat& frame,const vector<Mat>& outs)
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
            if(confidence>this->confThreshold)
            {
                int centerX=(int)(data[0]*frame.cols);
                int centerY=(int)(data[1]*frame.rows);
                int width=(int)(data[2]*frame.cols);
                int height=(int)(data[3]*frame.rows);
                int left=centerX-width/2;
                int top=centerY-height/2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
                boxes.push_back(Rect(left,top,width,height));

            }

        }
    }
    vector<int> indices;
    NMSBoxes(boxes,confidences,this->confThreshold,this->mnsThreshold,indices);
    for(size_t i=0;i<indices.size();++i)
    {
        int idx=indices[i];
        Rect box=boxes[idx];
        drawPred(classIds[idx],confidences[idx],box.x,box.y,box.x+box.width,box.y+box.height,frame);
    }
    return indices.size();
}

void Eyolo::drawPred(int classId,float conf,int left,int top,int right,int bottom,Mat& frame)
{
    //Draw a rectangle displaying the bounding box
    rectangle(frame,Point(left,top),Point(right,bottom),Scalar(255,178,50),3);
    //Get the label for the class name and its confidence
    string label=format("%.2f",conf);
    if(!this->classes.empty())
    {
        CV_Assert(classId <(int)this->classes.size());
        label=class[classId]+":"+label;
    }
    //Display the label at the top of the bounding box
    int baseLine;
    Size labelSize=getTextSize(label,FONT_HERSHEY_SIMPLEX,0.5,1,&baseLine);
    top=max(top,labelSize.height);
    rectangle(frame, Point(left, top - round(1.5*labelSize.height)), Point(left + round(1.5*labelSize.width), top + baseLine), Scalar(255, 255, 255), FILLED);
    putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,0),1);
}

//Get the names of the output layers
vector<String> Eyolo::getOutputsNames(const Net& net)
{
    static vector<String> names;
    if(names.empty())
    { 
        //Get the indices of the output layers, i.e. the layers with unconnected outputs
        vector<int> outLayers=net.getUnconnectedOutLayers();
        //get the names of all the layers in the network
        vector<String> layersNames=net.getLayerNames();
        // Get the names of the output layers in names
        names.resize(outLayers.size());
        for(size_t i=0;i<outLayers.size();++i);
        names[i]=layersNames[outLayers[i]-1];
    }
    return names;
}