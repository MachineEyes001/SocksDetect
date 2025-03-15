#pragma once
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/features2d/features2d.hpp>
//#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
//#include <cmath>
//#include <stdio.h>
//#include <opencv2/imgproc.hpp>
#include "yolo.h"
#include <modbus.h>
#include <bitset>

extern double con;
std::vector<double>Sockdataplus; //定义数据容器

// 袜子类
class Socks
{
public:

    Socks();
    ~Socks();

    std::string ModelpathGet(const int& num);
    int Ribtop_Re(const cv::Mat& g_srcImage, std::string& g_templateImage, const int& num);
    double binaryToDecimal(std::vector<double>& v);
    cv::Mat Centerline(const cv::Mat& screen, const cv::Mat& img, const double& orient, const double& imHeight, const double& imWidth, const double& ribtop_re, const double& ribtop);
    cv::Mat Centerline(const cv::Mat& screen, const cv::Mat& img, const double& orient, const double& imHeight, const double& imWidth);

private:

};

Socks::Socks()
{
}

Socks::~Socks()
{
}

// 获取模型路径
std::string Socks::ModelpathGet(const int& num)
{
    std::string modelpath;
    switch (num) {
    case 8:
        modelpath = "D:/vsdata/socksGUI/model/8.onnx";
        break;
    case 10:
        modelpath = "D:/vsdata/socksGUI/model/10.onnx";
        break;
    case 12:
        modelpath = "D:/vsdata/socksGUI/model/12.onnx";
        break;
    case 14:
        modelpath = "D:/vsdata/socksGUI/model/14.onnx";
        break;
    case 16:
        modelpath = "D:/vsdata/socksGUI/model/16.onnx";
        break;
    case 18:
        modelpath = "D:/vsdata/socksGUI/model/18.onnx";
        break;
    case 20:
        modelpath = "D:/vsdata/socksGUI/model/20.onnx";
        break;
    case 22:
        modelpath = "D:/vsdata/socksGUI/model/22.onnx";
        break;
    case 24:
        modelpath = "D:/vsdata/socksGUI/model/24.onnx";
        break;
    case 26:
        modelpath = "D:/vsdata/socksGUI/model/26.onnx";
        break;
    case 28:
        modelpath = "D:/vsdata/socksGUI/model/28.onnx";
        break;
    case 30:
        modelpath = "D:/vsdata/socksGUI/model/30.onnx";
        break;
    case 32:
        modelpath = "D:/vsdata/socksGUI/model/32.onnx";
        break;
    }
    return modelpath;
}

// 获取用户输入尺寸的位置，未找到目标则返回0
int Socks::Ribtop_Re(const cv::Mat& img, std::string& modelpath, const int& num)
{
    cv::Mat g_srcImage = img.clone();
    std::string className = std::to_string(num); //将用户输入的数字转换为字符串
    YOLO test;
    cv::dnn::Net net;
    if (test.readModel(net, modelpath, true)) {
        std::cout << "read net ok!" << std::endl;
    }
    else {
        return -1;
    }
    std::vector<Output> result;
    // 修改图片尺寸等于模型尺寸
    cv::Size dsize = cv::Size(640, 640);
    resize(g_srcImage, g_srcImage, dsize);
    //cv::cvtColor(img, img, cv::COLOR_BGR2RGB);//通道顺序转换为RGB
    if (test.Detect(g_srcImage, net, result))
    {
        int Y = test.drawPred(g_srcImage, result, className);
        std::cout << Y << std::endl;
        return Y;
    }
    else
    {
        std::cout << "Detect Failed!" << std::endl;
        return 0;
    }
}

// 获取符号位
double Socks::binaryToDecimal(std::vector<double>& v) {
    std::bitset<8> bits; // 使用32位二进制数
    for (int i = 0; i < v.size(); i++) {
        if (v[i] >= 0) {
            bits.set(i); // 如果数为正数，将对应位设为1
        }
    }
    return bits.to_ulong(); // 将二进制数转换成十进制数
}

// 绘制轮廓中心线和罗口偏移高度
cv::Mat Socks::Centerline(const cv::Mat& screen, const cv::Mat& img, const double& orient, const double& imHeight, const double& imWidth, const double& ribtop_re, const double& ribtop)
{
    // 寻找最外层轮廓
    std::vector<std::vector<cv::Point>> contours;//双层向量类型（二维数组[i][j]）内部向量（相当于j数组）表示的是每一个轮廓的点
    std::vector<cv::Vec4i> hierarchy;//存储查找到的第i个轮廓的后[i][0]、前[i][1]、父[i][2]、子轮廓[i][3]
    findContours(screen, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point());
    std::vector<double>Sockdata;//通讯数据初始化
    for (int i = 0; i < contours.size(); i++)
    {
        double y1 = 0;
        double x = 0;
        double a = 0;
        // 在原始图像上绘制轮廓的中心等分线，并计算直线与图片高度方向之间的夹角
        cv::Rect boundRect = cv::boundingRect(cv::Mat(contours[i]));
        double angle = atan2(boundRect.width, boundRect.height) * 180 / CV_PI;
        if (angle != 0)
        {
            if (orient > 90)
            {
                line(img, cv::Point(boundRect.x, boundRect.y + boundRect.height), cv::Point(boundRect.x + boundRect.width, boundRect.y), cv::Scalar(0, 0, 255), 5, cv::LINE_AA);//轮廓中心等分线
                line(img, cv::Point(boundRect.x, boundRect.y + boundRect.height), cv::Point(boundRect.x + boundRect.width, boundRect.y + boundRect.height), cv::Scalar(0, 255, 0), 2, cv::LINE_AA);//袜口偏移距离线
                line(img, cv::Point(boundRect.x + boundRect.width, boundRect.y + boundRect.height), cv::Point(boundRect.x + boundRect.width, boundRect.y), cv::Scalar(0, 255, 0), 2, cv::LINE_AA);//袜口偏移高度线
                putText(img, cv::format("-%.1f", angle), cv::Point(boundRect.x + boundRect.width / 2 + 10, boundRect.y + boundRect.height / 2), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
                y1 = (boundRect.y + boundRect.height) / con;
                x = boundRect.width / con;
                a = angle + 90.0;
            }
            else if (orient < 90)
            {
                line(img, cv::Point(boundRect.x, boundRect.y), cv::Point(boundRect.x + boundRect.width, boundRect.y + boundRect.height), cv::Scalar(0, 0, 255), 5, cv::LINE_AA);//轮廓中心等分线
                line(img, cv::Point(boundRect.x, boundRect.y + boundRect.height), cv::Point(boundRect.x + boundRect.width, boundRect.y + boundRect.height), cv::Scalar(0, 255, 0), 2, cv::LINE_AA);//袜口偏移距离线
                line(img, cv::Point(boundRect.x, boundRect.y), cv::Point(boundRect.x, boundRect.y + boundRect.height), cv::Scalar(0, 255, 0), 2, cv::LINE_AA);//袜口偏移高度线
                putText(img, cv::format("+%.1f", angle), cv::Point(boundRect.x + boundRect.width / 2 - 80, boundRect.y + boundRect.height / 2), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
                y1 = (boundRect.y + boundRect.height) / con;
                x = boundRect.width / con;
                a = 90.0 - angle;
            }
            else
            {
                /*std::cout << "该袜子无需整理" << std::endl;*/
                y1 = 0;
                x = 0;
                a = 0;
            }

        }
        else
        {
            /*std::cout << "该袜子无需整理" << std::endl;*/
            y1 = 0;
            x = 0;
            a = 0;
        }

        line(img, cv::Point(0, boundRect.y), cv::Point(imWidth, boundRect.y), cv::Scalar(255, 0, 255), 2, cv::LINE_AA);//袜口偏移起点线
        line(img, cv::Point(0, ribtop_re), cv::Point(imWidth, ribtop_re), cv::Scalar(255, 255, 0), 2, cv::LINE_AA);//罗口要求高度线
        line(img, cv::Point(0, ribtop), cv::Point(imWidth, ribtop), cv::Scalar(255, 255, 0), 2, cv::LINE_AA);//罗口位置线
        putText(img, cv::format("dy=%.1fmm", (ribtop_re - ribtop) / con), cv::Point(imWidth / 4, ribtop), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
        double y2 = ribtop_re / con;
        double dy = (ribtop_re - ribtop) / con;
        Sockdata.assign({ y1,x,y2,dy,a });
        double sym = binaryToDecimal(Sockdata);//获取符号位
        //std::vector<int>Sockdataplus; //定义数据容器
        Sockdataplus.assign({ y1,x,y2,dy,a,sym });
        /*ModbusTCP(Sockdataplus);*/
    }
    return img;
}

// 绘制轮廓中心线
cv::Mat Socks::Centerline(const cv::Mat& screen, const cv::Mat& img, const double& orient, const double& imHeight, const double& imWidth)
{
    // 寻找最外层轮廓
    std::vector<std::vector<cv::Point>> contours;//双层向量类型（二维数组[i][j]）内部向量（相当于j数组）表示的是每一个轮廓的点
    std::vector<cv::Vec4i> hierarchy;//存储查找到的第i个轮廓的后[i][0]、前[i][1]、父[i][2]、子轮廓[i][3]
    findContours(screen, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point());
    std::vector<double>Sockdata;//通讯数据初始化
    for (int i = 0; i < contours.size(); i++)
    {
        double y1 = 0;
        double x = 0;
        double a = 0;
        // 在原始图像上绘制轮廓的中心等分线，并计算直线与图片高度方向之间的夹角
        cv::Rect boundRect = cv::boundingRect(cv::Mat(contours[i]));
        double angle = atan2(boundRect.width, boundRect.height) * 180 / CV_PI;
        if (angle != 0)
        {
            if (orient > 90)
            {
                line(img, cv::Point(boundRect.x, boundRect.y + boundRect.height), cv::Point(boundRect.x + boundRect.width, boundRect.y), cv::Scalar(0, 0, 255), 5, cv::LINE_AA);//轮廓中心等分线
                line(img, cv::Point(boundRect.x, boundRect.y + boundRect.height), cv::Point(boundRect.x + boundRect.width, boundRect.y + boundRect.height), cv::Scalar(0, 255, 0), 2, cv::LINE_AA);//袜口偏移距离线
                line(img, cv::Point(boundRect.x + boundRect.width, boundRect.y + boundRect.height), cv::Point(boundRect.x + boundRect.width, boundRect.y), cv::Scalar(0, 255, 0), 2, cv::LINE_AA);//袜口偏移高度线
                putText(img, cv::format("-%.1f", angle), cv::Point(boundRect.x + boundRect.width / 2 + 10, boundRect.y + boundRect.height / 2), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
                y1 = (boundRect.y + boundRect.height) / con;
                x = boundRect.width / con;
                a = angle + 90.0;
            }
            else if (orient < 90)
            {
                line(img, cv::Point(boundRect.x, boundRect.y), cv::Point(boundRect.x + boundRect.width, boundRect.y + boundRect.height), cv::Scalar(0, 0, 255), 5, cv::LINE_AA);//轮廓中心等分线
                line(img, cv::Point(boundRect.x, boundRect.y + boundRect.height), cv::Point(boundRect.x + boundRect.width, boundRect.y + boundRect.height), cv::Scalar(0, 255, 0), 2, cv::LINE_AA);//袜口偏移距离线
                line(img, cv::Point(boundRect.x, boundRect.y), cv::Point(boundRect.x, boundRect.y + boundRect.height), cv::Scalar(0, 255, 0), 2, cv::LINE_AA);//袜口偏移高度线
                putText(img, cv::format("+%.1f", angle), cv::Point(boundRect.x + boundRect.width / 2 - 80, boundRect.y + boundRect.height / 2), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
                y1 = (boundRect.y + boundRect.height) / con;
                x = boundRect.width / con;
                a = 90.0 - angle;
            }
            else
            {
                /*std::cout << "该袜子无需整理" << std::endl;*/
                y1 = 0;
                x = 0;
                a = 0;
            }

        }
        else
        {
            /*std::cout << "该袜子无需整理" << std::endl;*/
            y1 = 0;
            x = 0;
            a = 0;
        }

        line(img, cv::Point(0, boundRect.y), cv::Point(imWidth, boundRect.y), cv::Scalar(255, 0, 255), 2, cv::LINE_AA);//袜口偏移起点线
        double y2 = 0;
        double dy = 0;
        Sockdata.assign({ y1,x,y2,dy,a });
        double sym = binaryToDecimal(Sockdata);//获取符号位
        //std::vector<int>Sockdataplus; //定义数据容器
        Sockdataplus.assign({ y1,x,y2,dy,a,sym });
        /*ModbusTCP(Sockdataplus);*/
    }
    return img;
}