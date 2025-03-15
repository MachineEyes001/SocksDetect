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

class Calculate
{
public:

    // 定义掩码区域缩放系数
    //double SAFELINE_Y = 50.0;//X方向缩放系数，正数为内缩，负数为外扩
    //double SAFELINE_X = 60.0;//Y方向缩放系数，正数为内缩，负数为外扩

    Calculate();
    ~Calculate();

    int GetOptimalThreshold(const cv::Mat& image);
    void expand_polygon(std::vector<cv::Point>& pList, std::vector<cv::Point>& out, const int& SAFELINE_Y, const int& SAFELINE_X);
    std::vector<cv::Point> Fragment(const cv::Mat& img, const int& thresholdvalue, const int& amend, const int& ribtop_width, const int& ribtop_height, const int& dx);
    cv::Mat Screen(const cv::Mat& img);
    double Orient(const cv::Mat& screen);
    cv::Mat Tailor(const cv::Mat& img, std::vector<cv::Point>& points_src, const int& dy, const int& dx);

private:

};

Calculate::Calculate()
{
}

Calculate::~Calculate()
{
}

// 寻找二值化最佳阈值
int Calculate::GetOptimalThreshold(const cv::Mat& image)//OTSU （大津法）
{
    //灰度化
    cv::Mat graysrc;
    cvtColor(image, graysrc, cv::COLOR_BGRA2GRAY);
    //图片的宽，高
    int nRows = image.rows;
    int nCols = image.cols;
    int threshold = 0;
    //最佳阈值
    double temp = 0.0;
    //获取
    double AvePix[256];
    int TotalPix[256];
    double nProDis[256];
    double nSumProDis[256];
    //变量初始化
    for (int i = 0; i < 256; i++)
    {
        AvePix[i] = 0.0;
        TotalPix[i] = 0;
        nProDis[i] = 0.0;
        nSumProDis[i] = 0.0;
    }
    //计算0-255 每个像素值的像素点
    for (int i = 0; i < nRows; i++)
    {
        for (int j = 0; j < nCols; j++)
        {
            TotalPix[(int)graysrc.at<uchar>(i, j)]++;
        }
    }
    //计算每个像素值的像素点占总像素点的比例
    for (int i = 0; i < 256; i++)
    {
        nProDis[i] = (double)TotalPix[i] / (nRows * nCols);
    }
    AvePix[0] = 0;
    nSumProDis[0] = nProDis[0];
    //计算背景区域的像素比例与像素平均值
    for (int i = 1; i < 256; i++)
    {
        nSumProDis[i] = nSumProDis[i - 1] + nProDis[i];
        AvePix[i] = AvePix[i - 1] + i * nProDis[i];
    }
    double mean = AvePix[255];
    //遍历像素值,找到类间方差最大的像数值即为最佳阈值
    for (int i = 1; i < 256; i++)
    {
        double Pback = nSumProDis[i];
        double Pfront = 1 - nSumProDis[i];
        double g = 0.0;
        if (fabs(Pback) > 0.000001 && fabs(Pfront) > 0.000001)
        {
            double Mback = AvePix[i];
            double Mfront = (mean - Pback * Mback) / Pfront;
            g = (double)(Pback * Pfront * pow((Mback - Mfront), 2));
            //大津算法 即（背景像素占总像素的比例 * 前景像素占总像素的比例 * （背景平均像素 - 前景平均像素）的平方）
            if (g > temp)
            {
                temp = g;
                threshold = i;
            }
        }
    }
    return threshold;
}

// 轮廓缩放
void Calculate::expand_polygon(std::vector<cv::Point>& pList, std::vector<cv::Point>& out, const int& SAFELINE_Y, const int& SAFELINE_X) {

    // 边集，然后归一化
    std::vector<cv::Point2f> dpList, ndpList;
    int count = pList.size();
    for (int i = 0; i < count; i++) {
        int next = (i == (count - 1) ? 0 : (i + 1));
        dpList.push_back(pList.at(next) - pList.at(i));
        float unitLen = 1.0f / sqrt(dpList.at(i).dot(dpList.at(i)));
        ndpList.push_back(dpList.at(i) * unitLen);
        /* cout << "i=" << i << ",pList:" << pList.at(next) << "," << pList.at(i) << ",dpList:" << dpList.at(i) << ",ndpList:" << ndpList.at(i) << endl;*/
    }

    // 计算线
    for (int i = 0; i < count; i++) {
        int startIndex = (i == 0 ? (count - 1) : (i - 1));
        int endIndex = i;
        float sinTheta = ndpList.at(startIndex).cross(ndpList.at(endIndex));
        cv::Point2f orientVector = ndpList.at(endIndex) - ndpList.at(startIndex);//i.e. PV2-V1P=PV2+PV1
        cv::Point2f temp_out;
        temp_out.x = pList.at(i).x + SAFELINE_X / sinTheta * orientVector.x;
        temp_out.y = pList.at(i).y + SAFELINE_Y / sinTheta * orientVector.y;
        out.push_back(temp_out);
    }
    reverse(out.begin(), out.end());
    //cout<<endl<<"out:"<<out<<endl;
}

// 寻找目标区域
std::vector<cv::Point> Calculate::Fragment(const cv::Mat& img, const int& thresholdvalue, const int& type, const int& ribtop_width, const int& ribtop_height, const int& dx)
{
    //灰度化
    cv::Mat graysrc;
    cvtColor(img, graysrc, cv::COLOR_BGRA2GRAY);
    cv::Mat dst;
    if (type == 0) 
    {
        //二值化
        cv::Mat dst1;
        threshold(graysrc, dst1, thresholdvalue, 255, cv::THRESH_BINARY);

        //形态学处理
        cv::Mat element1, dst2;
        element1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(31, 31));
        dilate(dst1, dst2, element1);
        cv::Mat element2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(35, 35)); //getStructuringElement函数返回的是指定形状和尺寸的结构元素
        erode(dst2, dst, element2);
    }

    else if (type == 1)
    {
        //二值化
        cv::Mat dst1;
        threshold(graysrc, dst1, thresholdvalue, 255, cv::THRESH_BINARY_INV);

        //形态学处理
        cv::Mat element1, dst2;
        element1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(31, 31));
        dilate(dst1, dst2, element1);
        cv::Mat element2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(35, 35)); //getStructuringElement函数返回的是指定形状和尺寸的结构元素
        erode(dst2, dst, element2);
    }

    else if (type == 2) 
    {
        //二值化
        cv::Mat dst1_1, dst1_2;
        threshold(graysrc, dst1_1, thresholdvalue, 255, cv::THRESH_BINARY);
        cv::Mat element1_1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(31, 31));
        dilate(dst1_1, dst1_1, element1_1);
        cv::Mat element1_2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(35, 35));
        erode(dst1_1, dst1_2, element1_2);

        cv::Mat dst2_1, dst2_2;
        threshold(graysrc, dst2_1, thresholdvalue+15, 255, cv::THRESH_BINARY);
        cv::Mat element2_1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(21, 21));
        erode(dst2_1, dst2_1, element2_1);
        cv::Mat element2_2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(45, 45));
        dilate(dst2_1, dst2_2, element2_2);

        dst = dst1_2 - dst2_2;
    }
    //Canny(dst3, dst3, 50, 150, 3);//辅助区域检测

    //筛选轮廓
    std::vector<std::vector<cv::Point>> contours1;//双层向量类型（二维数组[i][j]）内部向量（相当于j数组）表示的是每一个轮廓的点
    std::vector<cv::Vec4i> hierarchy1;//存储查找到的第i个轮廓的后[i][0]、前[i][1]、父[i][2]、子轮廓[i][3]
    findContours(dst, contours1, hierarchy1, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    cv::Mat dst3 = cv::Mat::zeros(img.size(), CV_8U);
    std::vector<double> g_dConArea(contours1.size());
    for (int i = 0; i < contours1.size(); i++)
    {
        g_dConArea[i] = contourArea(contours1[i]);
        /* cout << "用轮廓面积计算函数计算出来的第" << i << "个轮廓的面积为：" << g_dConArea[i] << endl;*/
    }
    int max = 0;
    for (int i = 1; i < contours1.size(); i++) {
        if (g_dConArea[i] > g_dConArea[max]) {
            max = i;
        }
    }
    //cout << "max=" << g_dConArea[max] << endl;
    //cout << "idx=" << max << endl;
    drawContours(dst3, contours1, max, cv::Scalar(255), cv::FILLED, 8, hierarchy1);

    // 寻找第一次筛选后的袜子区域的最外层轮廓
    std::vector<std::vector<cv::Point>> contours2;//双层向量类型（二维数组[i][j]）内部向量（相当于j数组）表示的是每一个轮廓的点
    std::vector<cv::Vec4i> hierarchy2;//存储查找到的第i个轮廓的后[i][0]、前[i][1]、父[i][2]、子轮廓[i][3]
    findContours(dst3, contours2, hierarchy2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point());
    std::vector<cv::Point> points_src;
    for (int i = 0; i < contours2.size(); i++)
    {
        // 得到轮廓的外接正矩形的点集
        cv::Rect boundRect = cv::boundingRect(cv::Mat(contours2[i]));

        // 顺时针输入一个多边形
        points_src.push_back(cv::Point(boundRect.x - dx + boundRect.width, boundRect.y + boundRect.height)); //1
        points_src.push_back(cv::Point(boundRect.x - dx + boundRect.width - ribtop_width, boundRect.y + boundRect.height)); //2
        points_src.push_back(cv::Point(boundRect.x - dx + boundRect.width - ribtop_width, boundRect.y + boundRect.height - ribtop_height)); //3
        points_src.push_back(cv::Point(boundRect.x - dx + boundRect.width, boundRect.y + boundRect.height - ribtop_height)); //4
        points_src.push_back(cv::Point(boundRect.x, boundRect.y));
    }
    return points_src;
}

// 轮廓筛选
cv::Mat Calculate::Screen(const cv::Mat& img)
{
    //筛选轮廓
    std::vector<std::vector<cv::Point>> contours;//双层向量类型（二维数组[i][j]）内部向量（相当于j数组）表示的是每一个轮廓的点
    std::vector<cv::Vec4i> hierarchy;//存储查找到的第i个轮廓的后[i][0]、前[i][1]、父[i][2]、子轮廓[i][3]
    findContours(img, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    cv::Mat dst;
    dst = cv::Mat::zeros(img.size(), CV_8U);
    std::vector<double> g_dConArea(contours.size());
    for (int i = 0; i < contours.size(); i++)
    {
        g_dConArea[i] = contourArea(contours[i]);
        /* cout << "用轮廓面积计算函数计算出来的第" << i << "个轮廓的面积为：" << g_dConArea[i] << endl;*/
    }
    int max = 0;
    for (int i = 1; i < contours.size(); i++) {
        if (g_dConArea[i] > g_dConArea[max]) {
            max = i;
        }
    }
    //cout << "max=" << g_dConArea[max] << endl;
    //cout << "idx=" << max << endl;
    drawContours(dst, contours, max, cv::Scalar(255), cv::FILLED, 8, hierarchy);
    return dst;
}

// 判断轮廓矩
double Calculate::Orient(const cv::Mat& screen)
{
    cv::Moments m = moments(screen, true);//moments()函数计算出三阶及一下的矩
    cv::Point2d center(m.m10 / m.m00, m.m01 / m.m00);//此为重心
    //计算方向
    double a = m.m20 / m.m00 - center.x * center.x;
    double b = m.m11 / m.m00 - center.x * center.y;
    double c = m.m02 / m.m00 - center.y * center.y;
    double degree = cv::fastAtan2(2 * b, (a - c)) / 2;//此为形状的方向
    //cout << 2 * b << endl;
    //cout << a - c << endl;
    //cout << "角度是：" << degree << endl;

    return degree;
}

// 截取目标区域
cv::Mat Calculate::Tailor(const cv::Mat& img, std::vector<cv::Point>& points, const int& dy, const int& dx)
{
    std::vector<cv::Point> points_src;
    points_src.assign(points.begin(), points.end() - 1);
    cv::Mat smoothed = cv::Mat::zeros(img.size(), CV_8U);
    // 顺时针输出一个扩展多边形
    std::vector<cv::Point> points_expand;
    expand_polygon(points_src, points_expand, dy, dx);
    rectangle(smoothed, cv::Point(points_expand[0]), cv::Point(points_expand[2]), cv::Scalar(255), cv::FILLED, 8);
    cv::Mat crop(img.rows, img.cols, CV_8U, cv::Scalar(0));
    img.copyTo(crop, smoothed); //将原图像拷贝至遮罩图层
    return crop;
}
