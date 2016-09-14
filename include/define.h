/******************************************************************************
 *
版权所有 (C), 2016, 中国科学院沈阳自动化研究所，一室，旋翼飞行机器人课题组
******************************************************************************
版 本 号   : v1.0
作	  者   : 肖斌
******************************************************************************/

#include "declare.h"
#include <sensor_msgs/LaserScan.h>
#include <geometry_msgs/TransformStamped.h>

//图像缩小系数
//#define  SHRINK_LOWEST_VALUE  0.9248555
//#define  SHRINK_LOWEST_VALUE  1.2
#define  SHRINK_LOWEST_VALUE  0.46
#define  SHRINK_HIGHEST_VALUE 1.0

//图像缩小
double shrink = 1;

basicOCR* KNNocr;
int cameraNo = 0;
int imageProcessedNo = 0;
double time0,time1,time2;
Mat g_rawImage;
Mat g_rawSaveImage;
Mat srcImg;
Mat rawCameraImg;
Mat g_rectResultImg;

bool copyIsRunningFlag = false;
bool rawImgHasCopiedOut = true;
bool rawImgHasSavedFlag = true;
bool resultImgHasSavedFlag = true;
bool g_rectResultImgUpdated = false;

//矩形长宽比例阈值
float maxSideLengthRatioAllowed = 2.5f;
float rectClassifyThres = 0.5f;
//识别信息
extern float g_PrecisionRatio;
extern int g_AccuracyAmount;

class MultiThreadListener
{
public:
    MultiThreadListener()
    {
        sub1 = pointGrayCameraProcessNode.subscribe("imu/attitude", 3, &MultiThreadListener::AttitudeSubCallBack,this);
        image_transport::ImageTransport imageProcessNode_it(pointGrayCameraProcessNode);
        sub2 = imageProcessNode_it.subscribe("vision/camera_image", 1, &MultiThreadListener::MainImageProcessing,this);
        image_transport::ImageTransport imageProcessNode_result_it(pointGrayCameraProcessNode);
        sub3 = imageProcessNode_result_it.subscribe("vision/result_image", 1, &MultiThreadListener::SaveResultImage,this);
    }
    void AttitudeSubCallBack(const geometry_msgs::TransformStamped::ConstPtr& att_msg);
    void MainImageProcessing( const sensor_msgs::ImageConstPtr& msg );
    void SaveResultImage( const sensor_msgs::ImageConstPtr& msg );

private:
    ros::NodeHandle pointGrayCameraProcessNode;
    ros::Subscriber sub1;
    image_transport::Subscriber sub2;
    image_transport::Subscriber sub3;
};

extern vector<VisionResult>  g_visionResult;
//视频存储
VideoWriter outputVideo;
vector<VisionResult>  visionResult;
vector<VisionResult>  incompleteRectResult;
vector<VisionResult>  lastFrameResult;
vector< vector<VisionResult> >  lastValidResult;
Attitude3D attitude3d;
//可能的各矩形信息
vector<RectMark> rectPossible;
//矩形分类存储,同一类代表同一个物理标志
vector< vector<RectMark> > rectCategory;
//透视变换后的矩形图像
vector<Mat> rectCandidateImg;
//存储的透视变换后图像
//vector<Mat> vecPerspectiveImg;
//透视变换后的二值化roi图像
//vector<Mat> vecPossibleDigitBinaryImg;


void InitRawImgSubscriber( );
void CameraImageSubCallback(const sensor_msgs::ImageConstPtr& msg);
void SaveResultImage( const sensor_msgs::ImageConstPtr& msg );
void MainImageProcessing( const sensor_msgs::ImageConstPtr& msg );
void AttitudeSubCallBack(const geometry_msgs::TransformStamped::ConstPtr& att_msg);
void* AttitudeSubThread(void*);

void ShowGuidanceImage(Mat& gray_image_left, Mat& gray_image_right, Mat& depth8, Mat& pseudo_color_img);
//发布数字识别结果
void DigitResultPublish( vector<VisionResult> & visionResult );
//相机坐标系到NEG坐标系；
void CameraCoordinate2NegCoordinate( vector<VisionResult>& vision_results, const Attitude3D& attitude3d);
