#include <string>
#include <iomanip>
#include <sstream>

#include "VideoProcessor.h"
#include "FrameProcessor.h"

int main() {
    VideoProcessor processor;
    // 打开摄像头
    processor.setInput(0);

    // 分别为输入和输出
    processor.displayInput("Input ");
    processor.displayOutput("Output ");

    processor.setDelay(1000. / processor.getFrameRate());

    processor.setFrameProcessor(canny);
    // 开始帧处理过程
    processor.run();
    waitKey();

/*
    VideoCapture capture(0);

    if (capture.isOpened())
    {
    	cout << "camera open successed" << endl;
    }
	Mat frame;
	double rate = capture.get(CV_CAP_PROP_FPS);
    bool stop(false);
    namedWindow("Extracted Frame");


	int delay = 1000/rate;

    while(!stop)
    {

    	if(!capture.read(frame))
		    break;
    	imshow("Extracted Frame",frame);

    	if(waitKey(delay)>=0)
    		stop = true;
    	//capture >> frame;
    	//imshow("capture",frame);
    	//char key = static_cast<char>(cvWaitKey(10));
    	//if(key == 27)
    	//	break
    }
*/
    //TODO 加入识别function
    return 0;
}
