#pragma once
#include <fstream>
#include <sstream>
#include <iostream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <time.h>

struct Output {
	int id;//结果类别id
	float confidence;//结果置信度
	cv::Rect box;//矩形框
};

class YOLO
{
public:
	YOLO() {};
	bool readModel(cv::dnn::Net& net, std::string& netPath, bool isCuda = false);
	bool Detect(const cv::Mat& SrcImg, cv::dnn::Net& net, std::vector<Output>& output);
	int drawPred(const cv::Mat& img, std::vector<Output>& result, std::string& className);
private:
	//计算归一化函数
	float Sigmoid(float x) {
		return static_cast<float>(1.f / (1.f + exp(-x)));
	}
	//设置默认锚定框，参数等于yolov5s.yaml中训练时使用的参数
	const float netAnchors[3][6] = { { 10.0,13.0, 16.0,30.0, 33.0,23.0 },{ 30.0,61.0, 62.0,45.0, 59.0,119.0 },{ 116.0,90.0, 156.0,198.0, 373.0,326.0 } };//{ { 17.0,22.0, 23.0,32.0, 31.0,44.0 },{ 40.0,60.0, 51.0,85.0, 68.0,117.0 },{ 108.0,93.0, 94.0,168.0, 170.0,331.0 } };
	const float netStride[3] = { 8.0, 16.0, 32.0 };
	const int netWidth = 640;//640
	const int netHeight = 640;//640
	double nmsThreshold = 0.2;//非极大值抑制阈值
	double boxThreshold = 0.5;// 0.4;
	double classThreshold = 0.5;// 0.2;
};