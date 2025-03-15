#include "BaslerCamera.h"
#include <QDebug>
#include <QMessageBox>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include "socks.hpp"
#include "calculate.hpp"

// 获取用户参数
extern int dx;
extern int ribtop_width;
extern int ribtop_height;
extern int type;
extern int num;
extern int black_h;

BaslerCamera::BaslerCamera(void)
{
	stopState = FALSE;//新增初始化
	mCamera = NULL;
	CameraRuning = FALSE;
	CameraOpening = FALSE;
	nodemap = NULL;
}

BaslerCamera::~BaslerCamera(void)
{
	if (mCamera != NULL)
	{
		delete mCamera;
		mCamera = NULL;
		CameraRuning = false;
		CameraOpening = false;
	}

	Pylon::PylonTerminate();
}

bool BaslerCamera::OpenCamera()
{
	if (!CameraOpening) {
		// 在使用任何pylon方法之前，必须初始化pylon运行时
		Pylon::PylonInitialize();

		try
		{
			// 为首先找到的相机设备创建一个即时相机对象
			mCamera = new Pylon::CInstantCamera(Pylon::CTlFactory::GetInstance().CreateFirstDevice());

			if (mCamera != NULL)
			{
				nodemap = &mCamera->GetNodeMap();

				// 打开相机
				mCamera->Open();

				std::cout << GenApi::CStringPtr(nodemap->GetNode("DeviceVendorName"))->GetValue() << std::endl;

				CameraOpening = true;
			}
		}
		catch (const Pylon::GenericException& e)
		{
			// 错误处理
			QMessageBox::warning(0, "警告", "打开相机发生异常!");
			
			// 从输入缓冲区中删除剩余字符
			std::cin.ignore(std::cin.rdbuf()->in_avail());
			CameraOpening = false;
			return false;
		}
	}

	return true;
}

bool BaslerCamera::StartCamera()
{
	if (CameraOpening && !CameraRuning && mCamera && mCamera->CanWaitForFrameTriggerReady())
	{
		//通过设置grabLoopType参数，使用抓取循环线程启动抓取
		//到GrabLoop_ProvidedByInstantCamera。抓取结果被传递给图像事件处理程序
		//使用GrabStrategy_OneByOne默认抓取策略
		mCamera->StartGrabbing(Pylon::GrabStrategy_OneByOne, Pylon::GrabLoop_ProvidedByUser);

		CameraRuning = true;
		return true;
	}
	else {
		return false;
	}

}

void BaslerCamera::StopCamera()
{
	if (mCamera) {
		stopState = true;
		mCamera->StopGrabbing();
		CameraRuning = false;
	}
}

void BaslerCamera::CloseCamera()
{
	if (mCamera != NULL)
	{
		stopState = true;
		delete mCamera;
		mCamera = NULL;
		nodemap = NULL;
		CameraRuning = false;
		CameraOpening = false;

	}
	Pylon::PylonTerminate();
}

bool BaslerCamera::GetFloatPara(const char* nameNode, double& para)
{
	if (nodemap) {
		GenApi::CFloatPtr tmp(nodemap->GetNode(nameNode));

		para = tmp->GetMin();

		return TRUE;
	}

	return FALSE;
}

bool BaslerCamera::GetIntPara(const char* nameNode, int64_t& para)
{
	if (nodemap) {
		GenApi::CIntegerPtr tmp(nodemap->GetNode(nameNode));
		para = tmp->GetValue();
		return true;
	}

	return false;
}

bool BaslerCamera::GetStringPara(const char* nameNode, std::string& para)
{
	if (nodemap) {
		GenApi::CEnumerationPtr tmp(nodemap->GetNode(nameNode));
		para = tmp->ToString();

		return true;
	}

	return false;
}

bool BaslerCamera::SetFloatPara(const char* nameNode, const double& para)
{
	if (nodemap) {
		GenApi::CFloatPtr tmp(nodemap->GetNode(nameNode));

		if (IsWritable(tmp)) {
			tmp->SetValue(para);
			return true;
		}
		else {
			return false;
		}
	}

	return false;
}

bool BaslerCamera::SetIntPara(const char* nameNode, INT64 para)
{
	if (nodemap) {
		GenApi::CIntegerPtr tmp(nodemap->GetNode(nameNode));

		if (IsWritable(tmp)) {
			tmp->SetValue(para);
			return true;
		}
		else {
			return false;
		}
	}

	return false;
}

bool BaslerCamera::SetStringPara(const char* nameNode, Pylon::String_t para)
{
	if (nodemap) {
		GenApi::CEnumerationPtr tmp(nodemap->GetNode(nameNode));

		if (IsWritable(tmp) && IsAvailable(tmp->GetEntryByName(para))) {

			tmp->FromString(para);

			return true;
		}
		else {
			return false;
		}
	}

	return false;
}

bool BaslerCamera::SetCmd(const char* nameNode)
{
	if (nodemap) {
		GenApi::CCommandPtr tmp(nodemap->GetNode(nameNode));
		if (IsWritable(tmp)) {
			tmp->Execute();
		}
	}

	return false;
}

QImage BaslerCamera::MatToQImage(const cv::Mat& mat)
{
	// 根据mat的数据创建QImage
	QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_RGB888);

	// 如果图像是BGR格式，则进行颜色通道的交换
	if (mat.channels() == 3 && mat.type() == CV_8UC3)
	{
		return image.rgbSwapped();
	}

	return image;
}

void BaslerCamera::run()
{
	if (!this->StartCamera())
		return;

	stopState = false;

	// 获取模型路径
	Socks model;
	std::string modelpath = model.ModelpathGet(num);
	// pylonimg转opencvimg初始化
	Pylon::CImageFormatConverter formatConverter;//新建pylon ImageFormatConverter对象
	formatConverter.OutputPixelFormat = Pylon::PixelType_BGR8packed;//确定输出像素格式
	Pylon::CPylonImage pylonImage;//创建一个Pylonlmage后续将用来创建OpenCV images
	cv::Mat openCvImage;//新建一个OpenCV image对象

	while (!stopState) 
	{

		try 
		{
			while (mCamera->IsGrabbing())
			{
				// 等待接收和恢复图像，超时时间设置为5000 ms
				mCamera->RetrieveResult(5000, ptrGrabResult, Pylon::TimeoutHandling_ThrowException);

				// 如果图像抓取成功
				if (ptrGrabResult->GrabSucceeded())
				{
					// 添加图像处理代码
					formatConverter.Convert(pylonImage, ptrGrabResult);//将抓取的缓冲数据转化成pylon image.
					openCvImage = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t*)pylonImage.GetBuffer());//将 pylon image转成OpenCV image.

					// 图像处理
					if (type == 0)//白色
					{
						Socks whitesock_S;//声明袜子对象
						Calculate whitesock_C;//声明计算对象

						// 获取二值化最佳阈值
						int thresholdvalue = whitesock_C.GetOptimalThreshold(openCvImage);

						// 获取图像宽高
						double imHeight = ptrGrabResult->GetHeight();//row表示行，rows表示行的总数，即图像的高
						double imWidth = ptrGrabResult->GetWidth();//col表示列，cols表示列的总数，即图像的宽

						// 判断罗口高度是否达标
						int ribtop_re = whitesock_S.Ribtop_Re(openCvImage, modelpath, num);

						// 处理图像，筛选袜板区域
						std::vector<cv::Point> fragmentdata = whitesock_C.Fragment(openCvImage, thresholdvalue, type, ribtop_width, ribtop_height, dx);
						cv::Mat fragment = whitesock_C.Tailor(openCvImage, fragmentdata, 60, 60);

						// 转换为灰度图像
						cv::Mat gray;
						cvtColor(fragment, gray, cv::COLOR_BGR2GRAY);

						// 定义Gabor滤波器参数
						//const int ksize = 21; // 滤波器大小,设为奇数
						//const double sigma = 4.6; // 高斯函数标准差
						//const double theta = 0; // Gabor滤波器的方向
						//const double lambda = 10.0; // 波长
						//const double gamma = 1.0; // 滤波器高宽比
						//const double psi = 0; // 相位偏移量
						// 创建Gabor滤波器
						cv::Mat kernel = cv::getGaborKernel(cv::Size(21, 21), 4.6, 0, 10.0, 1.0, 0);

						// 对灰度图像进行Gabor滤波
						cv::Mat filtered1;
						filter2D(gray, filtered1, CV_32F, kernel);

						// 将32位浮点型图片转换为8位无符号整形（Canny边缘检测要求输入图像为单通道8位图像）
						cv::Mat filtered2;
						filtered1.convertTo(filtered2, CV_8U);

						// 将滤波后的图像转换为二值图像
						cv::Mat binary;
						threshold(filtered2, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

						// 形态学处理
						cv::Mat element_e, element_d;
						cv::Mat dil, ero;
						element_d = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(4, 4));
						dilate(binary, dil, element_d);
						element_e = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)); //getStructuringElement函数返回的是指定形状和尺寸的结构元素
						erode(dil, ero, element_e);

						// 二次截取目标区域
						cv::Mat tailor = whitesock_C.Tailor(ero, fragmentdata, 60, 70);

						// 纹理筛选
						cv::Mat screen = whitesock_C.Screen(tailor);

						// 判断袜口偏移方向
						double orient = whitesock_C.Orient(screen);

						if (ribtop_re != 0)
						{
							// 获取罗口坐标
							cv::Point point = fragmentdata[0];
							int ribtop = point.y;
							// 绘制轮廓图像的中心等分线和罗口偏移高度
							resultimg = whitesock_S.Centerline(screen, openCvImage, orient, imHeight, imWidth, ribtop_re, ribtop);
						}
						else
						{
							// 绘制轮廓图像的中心等分线
							resultimg = whitesock_S.Centerline(screen, openCvImage, orient, imHeight, imWidth);
						}
					}
					else if (type == 1)//黑色
					{
						Socks blacksock_S;//声明袜子对象
						Calculate blacksock_C;//声明计算对象

						// 高斯滤波，去除噪声
						GaussianBlur(openCvImage, openCvImage, cv::Size(3, 3), 0);

						// 获取二值化最佳阈值
						int thresholdvalue = blacksock_C.GetOptimalThreshold(openCvImage);

						// 获取图像宽高
						double imHeight = ptrGrabResult->GetHeight();//row表示行，rows表示行的总数，即图像的高
						double imWidth = ptrGrabResult->GetWidth();//col表示列，cols表示列的总数，即图像的宽

						// 处理图像，筛选袜板区域
						std::vector<cv::Point> fragmentdata = blacksock_C.Fragment(openCvImage, thresholdvalue, type, ribtop_width, ribtop_height, dx);
						cv::Mat fragment = blacksock_C.Tailor(openCvImage, fragmentdata, 60, 60);

						// 转换为灰度图像
						cv::Mat gray;
						cvtColor(fragment, gray, cv::COLOR_BGR2GRAY);

						// 创建Gabor滤波器
						cv::Mat kernel = getGaborKernel(cv::Size(19, 19), 4.4, 0, 10.0, 1.0, 0);

						// 对灰度图像进行Gabor滤波
						cv::Mat filtered1;
						filter2D(gray, filtered1, CV_32F, kernel);

						// 将32位浮点型图片转换为8位无符号整形（Canny边缘检测要求输入图像为单通道8位图像）
						cv::Mat filtered2;
						filtered1.convertTo(filtered2, CV_8U);

						// 将滤波后的图像转换为二值图像
						cv::Mat binary;
						threshold(filtered2, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

						// 形态学处理
						cv::Mat element_e, element_d, dil, ero;
						element_d = getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
						dilate(binary, dil, element_d);
						element_e = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)); //getStructuringElement函数返回的是指定形状和尺寸的结构元素
						erode(dil, ero, element_e);

						// 二次截取目标区域
						cv::Mat tailor = blacksock_C.Tailor(ero, fragmentdata, 60, 70);

						// 纹理筛选
						cv::Mat screen = blacksock_C.Screen(tailor);

						// 判断袜口偏移方向
						double orient = blacksock_C.Orient(screen);

						cv::Point socks_y1 = fragmentdata[0];
						cv::Point socks_y2 = fragmentdata[4];
						int socks_h = socks_y1.y - socks_y2.y;

						if (socks_h != black_h)
						{
							// 获取罗口坐标
							int ribtop = socks_y1.y;
							int ribtop_re = black_h - socks_h + ribtop;
							// 绘制轮廓图像的中心等分线和罗口偏移高度
							resultimg = blacksock_S.Centerline(screen, openCvImage, orient, imHeight, imWidth, ribtop_re, ribtop);
						}
						else
						{
							// 绘制轮廓图像的中心等分线
							resultimg = blacksock_S.Centerline(screen, openCvImage, orient, imHeight, imWidth);
						}
					}
					else if (type == 2)//灰色
					{
						Socks graysock_S;//声明袜子对象
						Calculate graysock_C;//声明计算对象

						// 获取二值化最佳阈值
						int thresholdvalue = 80;// graysock_C.GetOptimalThreshold(img);

						// 获取图像宽高
						double imHeight = ptrGrabResult->GetHeight();//row表示行，rows表示行的总数，即图像的高
						double imWidth = ptrGrabResult->GetWidth();//col表示列，cols表示列的总数，即图像的宽

						// 判断罗口高度是否达标
						int ribtop_re = graysock_S.Ribtop_Re(openCvImage, modelpath, num);

						// 处理图像，筛选袜板区域
						std::vector<cv::Point> fragmentdata = graysock_C.Fragment(openCvImage, thresholdvalue, type, ribtop_width, ribtop_height, dx);
						cv::Mat fragment = graysock_C.Tailor(openCvImage, fragmentdata, 60, 60);
						std::cout << fragmentdata;

						// 转换为灰度图像
						cv::Mat gray;
						cvtColor(fragment, gray, cv::COLOR_BGR2GRAY);

						// 创建Gabor滤波器
						cv::Mat kernel = cv::getGaborKernel(cv::Size(21, 21), 4.7, 0, 10.0, 1.0, 0);

						// 对灰度图像进行Gabor滤波
						cv::Mat filtered1;
						filter2D(gray, filtered1, CV_32F, kernel);

						// 将32位浮点型图片转换为8位无符号整形（Canny边缘检测要求输入图像为单通道8位图像）
						cv::Mat filtered2;
						filtered1.convertTo(filtered2, CV_8U);

						// 将滤波后的图像转换为二值图像
						cv::Mat binary;
						threshold(filtered2, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

						// 形态学处理
						cv::Mat element_e, element_d, dil, ero;
						element_d = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
						dilate(binary, dil, element_d);
						element_e = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(4, 4)); //getStructuringElement函数返回的是指定形状和尺寸的结构元素
						erode(dil, ero, element_e);

						// 二次截取目标区域
						cv::Mat tailor = graysock_C.Tailor(ero, fragmentdata, 60, 80);

						// 纹理筛选
						cv::Mat screen = graysock_C.Screen(tailor);

						// 判断袜口偏移方向
						double orient = graysock_C.Orient(screen);

						if (ribtop_re != 0)
						{
							// 获取罗口坐标
							cv::Point point = fragmentdata[0];
							int ribtop = point.y;
							// 绘制轮廓图像的中心等分线和罗口偏移高度
							resultimg = graysock_S.Centerline(screen, openCvImage, orient, imHeight, imWidth, ribtop_re, ribtop);
						}
						else
						{
							// 绘制轮廓图像的中心等分线
							resultimg = graysock_S.Centerline(screen, openCvImage, orient, imHeight, imWidth);
						}
					}
					
					// 创建QImage对象
					img = MatToQImage(resultimg);
					// 发出图像显示信号
					emit showImageSignal(img);
				}
			}
		}
		catch (const Pylon::GenericException& e) {
			qDebug() << e.GetDescription();
		}
	}

	qDebug() << "Exit";
}