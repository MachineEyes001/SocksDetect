#pragma once
/*********************************************
*   说明：Basler相机sdk对接库。
**********************************************/
#ifndef _BASLER_CAMERA_H_
#define _BASLER_CAMERA_H_

#include <pylon/PylonIncludes.h>
#include <QThread>
#include <QImage>
#include <opencv2/opencv.hpp>

class BaslerCamera : public QThread
{
	Q_OBJECT

public:
	BaslerCamera(void);
	~BaslerCamera(void);

	bool OpenCamera();
	bool StartCamera();
	void StopCamera();
	void CloseCamera();
	bool GetFloatPara(const char* nameNode, double& para);
	bool GetIntPara(const char* nameNode, int64_t& para);
	bool GetStringPara(const char* nameNode, std::string& para);
	bool SetFloatPara(const char* nameNode, const double& para);
	bool SetIntPara(const char* nameNode, INT64 para);
	bool SetStringPara(const char* nameNode, Pylon::String_t para);
	bool SetCmd(const char* name);
	bool stopState;
	QImage MatToQImage(const cv::Mat& mat);

protected:
	void run();

private:
	Pylon::CInstantCamera* mCamera;

	GenApi::INodeMap* nodemap;

	bool CameraRuning;
	bool CameraOpening;

	Pylon::CGrabResultPtr ptrGrabResult;

	QImage img;
	cv::Mat resultimg;

signals:
	void showImageSignal(QImage);
};

#endif