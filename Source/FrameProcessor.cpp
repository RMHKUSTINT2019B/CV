//
// Created by KASHUN SHUM on 2018/10/14.
//

#include <opencv2/imgproc/imgproc.hpp>
#include "FrameProcessor.h"

using namespace cv;

void processFrame(const Mat& img, Mat& out);

//TODO 添加帧的处理函数
// 对视频的每帧做Canny算子边缘检测（just example）
Mat canny(const Mat& img) {
    cv::Mat out, blur, edged;
    // 先要把每帧图像转化为灰度图
    cvtColor(img, out, CV_BGR2GRAY);
    GaussianBlur(out, blur, Size(5, 5), 0, 0);
    // 调用Canny函数
    Canny(out, edged, 50, 255);
    // 对像素进行翻转
    threshold(edged, out, 230, 255, THRESH_BINARY_INV);
    return out;
}

/*
//灰度化
void gry(Mat& img, Mat& out)
{
	Mat image_gry = imread(img, IMREAD_GRAYSCALE);
	if (image_gry.empty())
		return -1;
	namedWindow("image_gry", WINDOW_AUTOSIZE);
	imshow("image_gry",image_gry);
}

//二值化
Mat image_bin;
threshold(image_gry, image_bin, 50, 255, THRESH_BINARY);
//imshow("image_bin", image_bin);


//膨胀233
Mat image_dil;
Mat element = getStructuringElement(MORPH_RECT, Size(20,20));
dilate(image_bin, image_dil, element);
imshow("image_dil", image_dil);


vector<vector<Point> > contours_out;
vector<Vec4i> hierarchy;


findContours(image_dil, contours_out, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
dilate(image_bin, image_dil, element);

drawContours(image_bin, contours_out, RETR_EXTERNAL, CHAIN_APPROX_NONE);
imshow("location", image_bin);
imshow("image_dil2", image_dil);

// re-arrange location according to the real position in the original image
const size_t size = contours_out.size();
vector<Rect> num_location;
for (int i = 0; i < contours_out.size(); i++)
{
num_location.push_back(boundingRect(Mat(contours_out[i])) );// 转换为矩形轮廓
}
sort(num_location.begin(), num_location.end(), cmp); // 重排轮廓信息


char rectnum[255];
char filenamew[255];
int tube_num = 0;
sprintf(filenamew, "location");
vector<Mat> tube;
char _itoa(int tube_num, char rectnum[255], int);

for (int i = 0; i < contours_out.size(); i++)
{
if (!IsAllWhite(image_dil(num_location.at(i))))
{
tube.push_back(image_dil(num_location.at(i)));
imshow(string(_itoa(tube_num, rectnum, 10)), tube.at(tube_num));
sprintf(rectnum, "%s%d.jpg", filenamew, tube_num);
imwrite(rectnum, tube.at(tube_num));
tube_num++;
}
}

cout << tube_num << endl;
for (int i = 0; i < tube_num; i++)
{
cout << "tube:" << i << endl;
cout << TubeIdentification(tube.at(i)) << endl;;
}

waitKey(0);
destroyAllWindows();

*/
/*
        cvNamedWindow("image_org");
        cvNamedWindow("image_gry");
        cvNamedWindow("image_bin");

        CvCapture* capture = cvCreateCameraCapture(0);
        IplImage* frame;

        while(true)
        {
            frame = cvQueryFrame(capture);
            //if (frame == NUll)
              //      break;
            IplImage *dst_grey = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
            IplImage *dst_grey = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);



            cvShowImage("win",frame);
            char c = cvWaitKey(50);

            if (c ==27)
                    break;

        }
        cvReleaseCapture(&capture);
        cvDestroyWindow("win");

*/