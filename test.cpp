#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>


void putTextRotate(cv::Mat& img, const cv::Point2f& centerPoint, cv::Scalar color, const std::string& text, const int& rotate = -1)
{
    // Create a temporary image for the text
    cv::Mat textImg = cv::Mat::zeros(img.size(), img.type());
    cv::putText(textImg, text, centerPoint, cv::FONT_HERSHEY_SIMPLEX, 1.0, color, 2);

    // Calculate the rotation matrix
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(centerPoint, rotate, 1.0);

    // Rotate the text
    cv::Mat rotatedText;
    cv::warpAffine(textImg, rotatedText, rotationMatrix, img.size());

    // Merge the rotated text with the original image
    cv::addWeighted(img, 1.0, rotatedText, 0.1, 0.0, img);
}

#if 0
int main()
{
    cv::Scalar color(236, 236, 236);

    std::string text = "Maxvision";
    int rotateAngle = 45;
    int spacing = 150;



    cv::Mat image = cv::imread("F://source//opencv-demo//build//Release/1.jpg");
    if (image.empty()) {
        return -1;
    }
    int rows = image.rows;
    int cols = image.cols;

    // 在图片上叠加多个水印
    for (int y = spacing; y < rows; y += spacing) {
        for (int x = spacing; x < cols; x += spacing) {
            cv::Point2f center(x, y);
            putTextRotate(image, center, color, text, rotateAngle);
        }
    }

    cv::Mat image1 = cv::imread("F://source//opencv-demo//build//Release/2.jpg");
    if (image1.empty()) {
        return -1;
    }
    rows = image1.rows;
    cols = image1.cols;

    // 在图片上叠加多个水印
    for (int y = spacing; y < rows; y += spacing) {
        for (int x = spacing; x < cols; x += spacing) {
            cv::Point2f center(x, y);
            putTextRotate(image1, center, color, text, rotateAngle);
        }
    }

    cv::Mat image2 = cv::imread("F://source//opencv-demo//build//Release/3.jpg");
    if (image2.empty()) {
        return -1;
    }
    rows = image2.rows;
    cols = image2.cols;

    // 在图片上叠加多个水印
    for (int y = spacing; y < rows; y += spacing) {
        for (int x = spacing; x < cols; x += spacing) {
            cv::Point2f center(x, y);
            putTextRotate(image2, center, color, text, rotateAngle);
        }
    }

    cv::imwrite("white_image.jpg", image);
    cv::imwrite("ir_image.jpg", image1);
    cv::imwrite("uv_image.jpg", image2);
    // 显示结果图像
    cv::imshow("white image", image);
    cv::imshow("ir image", image1);
    cv::imshow("uv image", image2);
    cv::waitKey(0);

    return 0;
}
#else
int main() {
    // 创建一个VideoCapture对象来打开默认摄像头
    cv::VideoCapture cap(0); // 参数0通常是默认摄像头

    if (!cap.isOpened()) {
        std::cerr << "Error: open camera failed" << std::endl;
        return -1;
    }

    cv::namedWindow("Camera Output", cv::WINDOW_NORMAL); // 创建一个窗口
    cv::Mat frame; // 用于存储每一帧的图像

    while (true) {
        // 读取一帧
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: get frame failed" << std::endl;
            break; // 退出循环如果无法读取帧
        }

        // 显示帧
        cv::imshow("Camera Output", frame);

        // 等待1毫秒，然后检查是否有按键按下，如果是'q'则退出循环
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    // 释放VideoCapture对象
    cap.release();
    // 销毁所有OpenCV窗口
    cv::destroyAllWindows();

    return 0;
}
#endif

