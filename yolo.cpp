#include "yolo.h"

bool YOLO::readModel(cv::dnn::Net& net, std::string& netPath, bool isCuda)
{
	try {
		net = cv::dnn::readNetFromONNX(netPath);
	}
	catch (const std::exception&) {
		return false;
	}
	//cuda
	if (isCuda) {
		net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
		net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
		std::cout << "GPU" << std::endl;
	}
	//cpu
	else {
		net.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
		net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
		std::cout << "CPU" << std::endl;
	}

	return true;
}

bool YOLO::Detect(const cv::Mat& SrcImg, cv::dnn::Net& net, std::vector<Output>& output)
{
	cv::Mat blob;
	int col = SrcImg.cols, row = SrcImg.rows;
	int maxLen = std::max(col, row);
	cv::Mat netInputImg = SrcImg.clone();
	if (maxLen > 1.2 * col || maxLen > 1.2 * row) {
		cv::Mat resizeImg = cv::Mat::zeros(maxLen, maxLen, CV_8UC3);
		SrcImg.copyTo(resizeImg(cv::Rect(0, 0, col, row)));
		netInputImg = resizeImg.clone();
	}
	cv::dnn::blobFromImage(SrcImg, blob, 1 / 255.0, cv::Size(netWidth, netHeight), cv::Scalar(0, 0, 0), true, false);
	net.setInput(blob);
	std::vector<cv::Mat> netOutputImg;
	net.forward(netOutputImg, net.getUnconnectedOutLayersNames());
	std::vector<int> classIds;//结果id数组
	std::vector<float> confidences;//结果每个id对应置信度数组
	std::vector<cv::Rect> boxes;//每个id矩形框
	float ratio_h = (float)netInputImg.rows / netHeight;//纠正
	float ratio_w = (float)netInputImg.cols / netWidth;//纠正
	int net_width = 1 + 5;  //输出的网络宽度是类别数+5
	float* pdata = (float*)netOutputImg[0].data;
	for (int stride = 0; stride < 3; stride++) {    //stride
		int grid_x = (int)(netWidth / netStride[stride]);
		int grid_y = (int)(netHeight / netStride[stride]);
		for (int anchor = 0; anchor < 3; anchor++) { //anchors
			const float anchor_w = netAnchors[stride][anchor * 2];
			const float anchor_h = netAnchors[stride][anchor * 2 + 1];
			for (int i = 0; i < grid_x; i++) {
				for (int j = 0; j < grid_y; j++) {
					float box_score = Sigmoid(pdata[4]);//获取每一行的box框中含有某个物体的概率
					if (box_score > boxThreshold) {
						cv::Mat scores(1, 1, CV_32FC1, pdata + 5);
						cv::Point classIdPoint;
						double max_class_socre;
						minMaxLoc(scores, 0, &max_class_socre, 0, &classIdPoint);
						max_class_socre = Sigmoid((float)max_class_socre);
						if (max_class_socre > boxThreshold) {
							//rect [x,y,w,h]
							float x = (Sigmoid(pdata[0]) * 2.f - 0.5f + j) * netStride[stride];  //x
							float y = (Sigmoid(pdata[1]) * 2.f - 0.5f + i) * netStride[stride];   //y
							float w = powf(Sigmoid(pdata[2]) * 2.f, 2.f) * anchor_w;   //w
							float h = powf(Sigmoid(pdata[3]) * 2.f, 2.f) * anchor_h;  //h
							int left = (x - 0.5 * w) * ratio_w;
							int top = (y - 0.5 * h) * ratio_h;
							classIds.push_back(classIdPoint.x);
							confidences.push_back(max_class_socre);
							boxes.push_back(cv::Rect(left, top, int(w * ratio_w), int(h * ratio_h)));
						}
					}
					pdata += net_width;//指针移到下一行
				}
			}
		}
	}
	std::vector<int> nms_result;
	cv::dnn::NMSBoxes(boxes, confidences, classThreshold, nmsThreshold, nms_result);
	for (int i = 0; i < nms_result.size(); i++) {
		int idx = nms_result[i];
		Output result;
		result.id = classIds[idx];
		result.confidence = confidences[idx];
		result.box = boxes[idx];
		output.push_back(result);
	}
	if (output.size())
		return true;
	else
		return false;
}

// 标签类等于1的模型，不需要指定检测目标，使用如下方法获取目标位置
int YOLO::drawPred(const cv::Mat& img, std::vector<Output>& result, std::string& className) {
	for (int i = 0; i < result.size(); i++)
	{
		int left, top;
		left = result[i].box.x;
		top = result[i].box.y;
		//rectangle(img, result[i].box, cv::Scalar(0, 0, 255), 2, 8);
		std::string label = className + ":" + std::to_string(result[i].confidence);
		int baseLine;
		cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
		top = cv::max(top, labelSize.height);
		//putText(img, label, cv::Point(left, top - 10), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
		return (top + result[i].box.height / 2) * 1200 / 640;//其中1200是图片原高度，1280是图片训练时指定的模型高度
	}
}