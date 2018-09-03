/******************** ���� ************************/
/*
���Ƕ�����ҵ��ѧARES������ս����Robomaster2018�������ڼ�����Ӿ�����
���������룬������С�������غ��Ӿ�������׼��ȫ�����ݣ�Ӧ���ڲ����������ϡ�
��ע��Ӣ�ۻ��������ڱ������˲ο��Ӿ��������ֵĴ��룬�����벽����������
��һ���ǹ��õģ�����Ȼ���ǵĳɹ���һЩǿ����Ȼ��кܴ�Ĳ�࣬��������
ϣ����ĳЩ�����������ǵĽ��˼·�ܸ������������飨�ر����²������飩�ṩ
һ���Ľ��������ֻ��һ�㣬��ô���ǵĹ���Ҳ��ֵ�õġ����ϣ�����ǵĳɹ���
��������ң�ףԸRobomasterԽ��Խ���--- ����һ��Ϊ�˻����˵���26���ּ���
�뿪У԰�����ˣ�
*/

//ʱ�䣺2018.09.02
//�ص㣺������ҵ��ѧ�ɶ�¥��ѧ���Ƽ�����ʵ����
//���ߣ��������Ī�塢����㡢����ΰ����ʿҢ��


#include"bigSymbol.h"
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

cv::Point3f posTarget[9] =
{
	cv::Point3f(-0.115,0.280,0),
	cv::Point3f(0.21,0.280,0),
	cv::Point3f(0.535,0.280,0),
	cv::Point3f(-0.115,0.53,0),
	cv::Point3f(0.21,0.53,0),
	cv::Point3f(0.535,0.53,0),
	cv::Point3f(-0.115,0.78,0),
	cv::Point3f(0.21,0.78,0),
	cv::Point3f(0.535,0.78,0),
};
//========== ��������ıȽϺ��� =============//
//����ȽϺ���(distance)
bool lessmark(const cv::Rect& s1, const cv::Rect& s2)
{
	return s1.x < s2.x;
}
//����ȽϺ���(distance)
bool lessmark_myRect(const myRect& s1, const myRect& s2)
{
	return s1.similarValue < s2.similarValue;
}
//����ȽϺ���
bool lessmark_rotateArea(const cv::RotatedRect& s1, const cv::RotatedRect& s2)
{
	return s1.size.area()>s2.size.area();
}
bool sortUp(const cv::Rect& s1, const cv::Rect& s2)
{
	return (s1.y) < (s2.y);
}
//����ȽϺ���
bool sortDown(const cv::Rect& s1, const cv::Rect& s2)
{
	return (s1.width*s1.height) > (s2.width*s2.height);
}
//�����ƶ�����Ƚ�
bool lessmark_mypswRect(const mypswRect& s1, mypswRect& s2)
{
	return s1.similarValue < s2.similarValue;
}
//��X��������Ƚ�
bool lessmark_mypswRect1(const mypswRect& s1, mypswRect& s2)
{
	return (s1.boundRect.x) < (s2.boundRect.x);
}
//����ȽϺ��� X+Y
bool lessmark_myPoint(const myPoint& s1, const myPoint& s2)
{
	return s1.sumXY > s2.sumXY;
}
//����ȽϺ��� X+(rows-Y)
bool lessmark_myPoint1(const myPoint& s1, const myPoint& s2)
{
	return s1.sumXY_inv > s2.sumXY_inv;
}
//����ȽϺ���
bool lessmark_myResult(const myResult& s1, const myResult& s2)
{
	return s1.Index < s2.Index;
}


//========== ���캯��-ϵͳ��ʼ�� ===========//
sbol::sbol()
{
	SymbolModel = numArab; //Ĭ�������ΪС��ģʽ
	//ʶ��ģ�ͳ�ʼ��
	knn = cv::ml::KNearest::create();
	svm = cv::ml::SVM::create();
	svm2 = cv::ml::SVM::create();
	trainpassword();//ѵ������
	loadSVM();//����SVMģ��

	psWord[5] = {0}; //��ǰ����
	lstPsWord[5] = {0}; //��һ�ε�����
	hitCnt = 0;//�������

	shootFlag = true;
	//observeFlag = false;
    pChangeFlag = false; //��ҳ��־λ
	stopChange_cnt = 0;
	for (int i = 0; i < 9; i++)
	{
		ArrayResult[i] = 0;
		ArrayResult1[i] = 0;
		ArrayResult2[i] = 0;
		ArrayResult3[i] = 0;
	}

    shootCnt = 0;
	shootCnt1 = 0;
	//ʶ������ʼ��
	myResult temp;
	temp.Index = 0;
	temp.value = 0;
	temp.position = Point(0, 0);
	for (int i = 0; i < 9; i++)
	{
		vrecResult.push_back(temp);
		vlstrecResult.push_back(temp);
	};

    //�����������

	cv::FileStorage fs2("C:\\DATA\\Paragram\\camera.yml", cv::FileStorage::READ);
	fs2["camera_matrix"] >> K_;
	fs2["distortion_coefficients"] >> D_; 
	fs2.release();
	//cout << K_ << endl;
	//cout << D_ << endl;
	//������ĸ��˵����������
	float cornersaa[4][2] = {
		0.0,0.0,
		-0.0130,0.098,
		0.416,0.0,
		0.403,0.098,
		/*0.0,0.0,
		-0.015,0.115,
		0.495,0.0,
		0.480,0.115,*/
	};
	for (int i = 0; i < 4; i++)
	{
		Point3f tmp;
		tmp.x = cornersaa[i][0];
		tmp.y = cornersaa[i][1];
		tmp.z = 0;
		corners.push_back(tmp);
	}
	Depth = 0;
	angleY = 0;
    angleP = 0;
	//��̨�����Ƕȣ���������ͷ����ϵ����̨����ϵ��һ��������
	//һ�������Ƕȣ�
	corectAngY = -2.0;
	corectAngP = -13.0;

	/*Eigen::Vector3f rgb_origin_out(-0.0861561, 0.139061, -0.026996);
	Eigen::Matrix3f rgb_btm;
	rgb_btm << -0.0141455, -0.988972, 0.147423, 0.9997, -0.0169382, -0.0177054, 0.0200073, 0.147128, 0.988915;
    cam_shot = EigenFrom3(rgb_origin_out, rgb_btm);*/

}

Eigen::Matrix4d sbol::EigenFrom3(Eigen::Vector3d transl, Eigen::Matrix3d Rmat)
{

	Eigen::Matrix4d out;

	out.topLeftCorner<3, 3>() = Rmat;

	out.topRightCorner<3, 1>() = transl;

	out(3, 0) = 0;

	out(3, 1) = 0;

	out(3, 2) = 0;

	out(3, 3) = 1;

	return out;

}
//KNNѵ������װ��
void sbol::sampleInit(cv::Mat& trainData, cv::Mat& trainClass)
{
	float *PtrainData = (float*)trainData.data;
	float *PtrainClass = (float*)trainClass.data;
	for (int i = 0; i < trainClasses; i++)
	{
		Mat img;
		char fileName[100];
		sprintf_s(fileName, "C:\\DATA\\M\\m%d.jpg", i + 1);
		img = imread(fileName, 0);
		if (!img.data)
		{
			printf("�޷���ȡ����ͼƬ��������ʼ��ʧ�ܣ�\n");
			return;
		}
		resize(img, img, Size(sizeX, sizeY));//��ͼ����һ���Ĵ�С
		threshold(img, img, 125, 255, THRESH_BINARY);
		uchar *Pimage = img.data;
		for (int n = 0; n < featureLen; n++)//������������ͼ�������ֵ
		{
			PtrainData[i*featureLen + n] = (float)Pimage[n];//��ͼ�����������������ֵ)���һ��������
		}
		PtrainClass[i] = (float)(i + 1);//����ǩ
	}
}
//ѵ������
void sbol::trainpassword()
{
	train_data.create(trainClasses*trainSample, featureLen, CV_32FC1);
	train_classes.create(trainClasses*trainSample, 1, CV_32FC1);
	sampleInit(train_data, train_classes);							   //����ѵ������
	knn->train(train_data, cv::ml::ROW_SAMPLE, train_classes);
}
//����SVM
void sbol::loadSVM()
{
	svm->clear(); 
	svm = cv::ml::SVM::load("C:\\DATA\\SVM\\SvmModel.xml");		 //	 SVM_HOG.xml		DATA\\SvmModel			Svm1	
	svm2->clear();
	svm2 = cv::ml::SVM::load("C:\\DATA\\SVM\\biaoqian1.xml");
}

//============= ����ͷ��ʼ�� =================//
bool sbol::camaraInit(int device)
{

	Capture_BigPower.open(device);
	//Capture_BigPower.open("C:\\NOW\\2\\2.mp4");
	if (!Capture_BigPower.isOpened())
	{
		printf("�������ͷ��ʧ�ܣ�\n");
		return false;
	}
	else
	{
		//���ø�����ͷ�ֱ���
		//Capture_BigPower.set(CV_CAP_PROP_EXPOSURE, (-7));//�ع� 50  -7
		Capture_BigPower.set(CV_CAP_PROP_FRAME_WIDTH, 960);
		Capture_BigPower.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
		return true;
	}
}
//ͼ��Ԥ����
void sbol::imgPretreatment(const cv::Mat& src, cv::Mat& dst)
{
	Mat imgRed;
	std::vector<Mat> imgChannels;
	split(src, imgChannels);
	imgRed = imgChannels.at(2);
	Scalar meanValue = mean(imgRed);
	imgRed = imgRed - meanValue[0];
	blur(imgRed, imgRed, Size(5, 5));
	double maxValue_gray;
	minMaxLoc(imgRed, 0, &maxValue_gray, 0, 0);
	Mat imgBin;
	threshold(imgRed, imgBin, maxValue_gray*0.7, 255, THRESH_BINARY);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));//����˺���
	cv::dilate(imgBin, imgBin, element);
	cv::dilate(imgBin, imgBin, element);
	//cv::dilate(imgBin, imgBin, element);
	//cv::dilate(imgBin, imgBin, element);
	//imshow("Ԥ����_imgBin:", imgBin);
	dst = imgBin;
}
//��������ܸ���Ȥ����
bool sbol::findDigitronLightRoi(const cv::Mat& src, cv::Rect& roiRect)
{
	roiRect = Rect(0, 0, 0, 0);
	std::vector<std::vector<cv::Point>> contours;//��⵽������
	std::vector<cv::Vec4i> hierarchy;//�������
	Mat imgContours;
	src.copyTo(imgContours);//����һ��ͼ�����ڼ������
	cv::findContours(imgContours, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point(0, 0));
	//ɾ��̫С������ ������������Χ�������
	std::vector<cv::Rect> contoursRect;
	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() < 50)
			continue;
		Rect tempRect = boundingRect(contours[i]);
		contoursRect.push_back(tempRect);
	}
	if (contoursRect.size()<5)
	{
		cout << "error1   ";
		return false;
	}
	//���ݾ��γ����ɸѡ����
	std::vector<cv::Rect> tempRect1;
	for (int i = 0; i < contoursRect.size(); i++)
	{
		float H = (float)contoursRect[i].height;
		float W = (float)contoursRect[i].width;
		float rateHW = H / W;
		if (rateHW>1.01&&rateHW < 2.5)
			tempRect1.push_back(contoursRect[i]);
	}
	if (tempRect1.size()<4)
	{
		cout << "error2   ";
		return false;
	}
	//���ݴ�С���ƶ�ɸѡ����
	std::vector<cv::Rect> tempRect2;
	for (int i = 0; i < tempRect1.size(); i++)
	{
		int Index = i;
		int cnt = 0;
		int W = tempRect1[Index].width;
		int H = tempRect1[Index].height;
		for (int j = 0; j < tempRect1.size(); j++)
		{
			if (j == Index)
				continue;
			int tempW = tempRect1[j].width;
			int tempH = tempRect1[j].height;
			if ((abs(W - tempW) < 25) && (abs(H - tempH)<25))
				cnt++;
		}//ɸѡ���������
		if (cnt >= 3)
			tempRect2.push_back(tempRect1[Index]);
	}
	if (tempRect2.size() < 4)
	{
		cout << "error3   ";
		return false;
	}
	std::vector<myRect> rectVectors;
	for (int i = 0; i < tempRect2.size(); i++)
	{
		myRect tempRect;
		tempRect.boundRect = tempRect2[i];
		Mat imgRoi = src(tempRect.boundRect);
		Mat imgNum = imgRoi.clone();
		resize(imgNum, imgNum, Size(sizeX, sizeY));
		threshold(imgNum, imgNum, 200, 255, THRESH_BINARY);
		Mat testImg(1, featureLen, CV_32FC1);
		float* p = (float*)testImg.data;//pָ��testImg���׵�ַ
		uchar* PtestImg = imgNum.data;//PtestImgָ��imgNum���׵�ַ
		for (int n = 0; n < featureLen; n++)
		{
			p[n] = (float)PtestImg[n];//��imgNum�е�����ֵ��ֲ��testImg�С���ΪimgNum��Mat��K�������������ǰ��д洢��
		}
		Mat dists(1, 1, CV_32FC1, Scalar(0));
		Mat A(1, 1, CV_32FC1, Scalar(0));
		Mat B(1, 1, CV_32FC1, Scalar(0));
		knn->findNearest(testImg, 1, A, B, dists);//testImg�ǰ��д洢��,�����ȸ�����������������*K
		tempRect.similarValue = dists.at<float>(0, 0);//distsָ���ǵ�ǰ���θ����ݼ��ȶԣ��õ�������ٽ�ֵ
		rectVectors.push_back(tempRect);
	}
	sort(rectVectors.begin(), rectVectors.end(), lessmark_myRect);//���������������ƶȽ�������
	//�����������������ȡ�������
	std::vector<cv::Rect> vtempResultRects;
	for (int i = 0; i < 3; i++)
		vtempResultRects.push_back(rectVectors[i].boundRect);
	sort(vtempResultRects.begin(), vtempResultRects.end(), sortUp);
	int rectX, rectY, rectW, rectH;
	//�������������
	/*rectX = rectVectors[0].boundRect.x - 5 * rectVectors[0].boundRect.height;
	if (rectX < 0)
		rectX = 0;
	rectY = rectVectors[0].boundRect.y - rectVectors[0].boundRect.height / 2;
	if (rectY < 0)
		rectY = 0;
	rectW = 10 * rectVectors[0].boundRect.height;
	if ((rectX + rectW)>src.cols)
		rectW = src.cols - rectX;
	rectH = 2 * rectVectors[0].boundRect.height;
	if ((rectY + rectH)>src.rows)
		rectH = src.rows - rectY;*/
	rectX = vtempResultRects[0].x - 5 * vtempResultRects[0].height;
	if (rectX < 0)
	rectX = 0;
	rectY = vtempResultRects[0].y - vtempResultRects[0].height / 2;
	if (rectY < 0)
	rectY = 0;
	rectW = 10 * vtempResultRects[0].height;
	if ((rectX + rectW)>src.cols)
	rectW = src.cols - rectX;
	rectH = 2 * vtempResultRects[0].height;
	if ((rectY + rectH)>src.rows)
	rectH = src.rows - rectY;
	if ((rectW <= 0) || (rectH <= 0))
	{
		cout << "error4   ";
		return false;
	}
	roiRect.x = rectX;
	roiRect.y = rectY;
	roiRect.width = rectW;
	roiRect.height = rectH;
	return true;
}
//ʶ�����벢ȷ���ĸ��˵�
bool sbol::recPassWord(const cv::Mat& src, cv::Rect& dlRect, int* resultArray, std::vector<cv::Point2f>& observationPts, cv::Mat& r, cv::Mat& t,double& depth)
{
	for (int i = 0; i < 5; i++)
		resultArray[i] = 0;
	observationPts.clear();
	depth = 0;
	Mat img;
	img = src(dlRect);

	Mat imgRed;
	std::vector<Mat> imgChannels;
	split(img, imgChannels);
	imgRed = imgChannels.at(2);
	Scalar meanValue = mean(imgRed);
	imgRed = imgRed - meanValue[0];
	blur(imgRed, imgRed, Size(5, 5));
	double maxValue_gray;
	minMaxLoc(imgRed, 0, &maxValue_gray, 0, 0);
	Mat imgBin;
	threshold(imgRed, imgBin, maxValue_gray*0.7, 255, THRESH_BINARY);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));//����˺���
	cv::dilate(imgBin, imgBin, element);
	cv::dilate(imgBin, imgBin, element);
	cv::dilate(imgBin, imgBin, element);
	//imshow("ʶ������_imgBin:", imgBin);

	std::vector<std::vector<cv::Point>> contours;//��⵽������
	std::vector<cv::Vec4i> hierarchy;//�������
	Mat imgContours;
	imgBin.copyTo(imgContours);//����һ��ͼ�����ڼ������
	cv::findContours(imgContours, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point(0, 0));
	if (contours.size() < 5)
	{
		cout << "error5   ";
		return false;
	}
	std::vector<mypswRect> pswRect;
	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() < 50)
			continue;
		mypswRect tempPswRect;
		tempPswRect.boundRect = boundingRect(contours[i]);
		tempPswRect.contours = contours[i];
		Mat imgRoi = imgBin(tempPswRect.boundRect);		    //ͨ������ܵ�������ͼ
		Mat imgNum = imgRoi.clone();
		resize(imgNum, imgNum, Size(sizeX, sizeY));
		threshold(imgNum, imgNum, 200, 255, THRESH_BINARY);
		Mat testImg(1, featureLen, CV_32FC1);
		float* p = (float*)testImg.data;
		uchar* PtestImg = imgNum.data;
		for (int n = 0; n < featureLen; n++)
		{
			p[n] = (float)PtestImg[n];
		}
		Mat dists(1, 1, CV_32FC1, Scalar(0));
		Mat C(1, 1, CV_32FC1, Scalar(0));
		Mat D(1, 1, CV_32FC1, Scalar(0));
		knn->findNearest(testImg, 1, C, D, dists);
		tempPswRect.similarValue = dists.at<float>(0, 0);
		tempPswRect.value =(int) C.at<float>(0, 0);
		pswRect.push_back(tempPswRect);
	}
	if (pswRect.size()<5)
	{
		cout << "error6   " << endl;
		return false;
	}
	sort(pswRect.begin(), pswRect.end(), lessmark_mypswRect);//�����ƶ�����Ƚ�
	std::vector<mypswRect> pswNumRects;
	for (int i = 0; i < 5; i++)
		pswNumRects.push_back(pswRect[i]);
	sort(pswNumRects.begin(), pswNumRects.end(), lessmark_mypswRect1);//����������Ƚ�

	//��������
	for (int i = 0; i < 5; i++)
		resultArray[i] = pswNumRects[i].value;

	//�������������ĸ�����(����Solve pnp ��������ӳ��)
	for (int i = 0; i < pswNumRects.size(); i+=4)
	{
		std::vector<myPoint> vtempMyPoint;
		for (int j = 0; j < pswNumRects[i].contours.size(); j++)
		{
			myPoint tempMyPt;
			tempMyPt.pt = pswNumRects[i].contours[j];
			tempMyPt.sumXY = pswNumRects[i].contours[j].x + pswNumRects[i].contours[j].y;
			tempMyPt.sumXY_inv = pswNumRects[i].contours[j] .x+ (img.rows - pswNumRects[i].contours[j].y);
			vtempMyPoint.push_back(tempMyPt);
		}
		cv::Point2f tempPt;
	    //��SUM(X+(rows-Y))����Ƚ�
		sort(vtempMyPoint.begin(), vtempMyPoint.end(), lessmark_myPoint1);
		tempPt = cv::Point2f(vtempMyPoint[0].pt.x + dlRect.x, vtempMyPoint[0].pt.y + dlRect.y);
		observationPts.push_back(tempPt);
		//��SUM(X+Y)����Ƚ�
		sort(vtempMyPoint.begin(), vtempMyPoint.end(), lessmark_myPoint);
		tempPt = cv::Point2f(vtempMyPoint[0].pt.x + dlRect.x, vtempMyPoint[0].pt.y + dlRect.y);
		observationPts.push_back(tempPt);
	}
	Mat rvec, tvec;
	cv::solvePnP(cv::Mat(corners), cv::Mat(observationPts), K_, D_, rvec, tvec, false);
	Rodrigues(rvec, r);  //��ת����ת��Ϊ��ת����
	t = tvec;//ƽ������
	depth = tvec.at<double>(2, 0);
	/*cv::Point2f Angle;
	cv::Matx33d tempK(K_);
	Angle = get_pattern_shot_3D(tempK, observationPts[1], cam_shot, depth);
	angleY = Angle.x + corectAngY;
	angleP = Angle.y + corectAngP;*/
	//angleY -= angleY / 10;
	//angleP -= angleP / 3;
	return true;
}

//============= ���ô��˳�� ==============//
void sbol::resetShootCnt(int* ps_array, int* ls_ps_array, int& count, int& err_count, int& cnt, bool& flag)
{
	int tempCount = 0;
	for (int i = 0; i < 5; i++)
	{
		if (ps_array[i] != ls_ps_array[i])
			tempCount++;
	}
	if ((tempCount == 0) && (flag == true))
	{
		return;
	}
	else if ((tempCount > 0) && (tempCount < 3) && (flag == true))
	{
		for (int i = 0; i < 5; i++)
		{
			ps_array[i] = ls_ps_array[i];
		}
		return;
	}
	else if ((tempCount>=3)&&(flag == true))
	{
		flag = false;
		count = 0;
		err_count = 0;
		return;
	}
	else if (flag == false)
	{
		count++;
		if (tempCount >= 3)
			err_count++;
		if (count == 2)
		{
			if (err_count < 2)
			{
				flag = true;
				for (int i = 0; i < 5; i++)
				{
					ps_array[i] = ls_ps_array[i];
				}
			}
			else
			{
				flag = true;
				for (int i = 0; i < 5; i++)
				{
					ls_ps_array[i] = ps_array[i];
				}
				cnt = 0; //��������
				cout << "��������   ";
			}
		}
	}
}
//�˵�ӳ��
void sbol::point_To_point(std::vector<cv::Point2f>& src, std::vector<cv::Point>& dst)
{
	double a;
	cv::Point tempPt[4];
	a = (src[2].x - src[0].x) / 416;
	tempPt[0] = cv::Point(src[0].x - 328.5*a, src[1].y + 60.2*a);
	tempPt[1] = cv::Point(src[0].x - 328.5*a, src[1].y + 720.2*a);
	tempPt[2] = cv::Point(src[0].x + 727.5*a, src[1].y + 60.2*a);
	tempPt[3] = cv::Point(src[0].x + 727.5*a, src[1].y + 720.2*a);
	for (int i = 0; i < 4; i++)
		dst.push_back(tempPt[i]);
}
//�����������
bool sbol::calGirdRect(const cv::Mat& src, std::vector<cv::Point2f>& pts, std::vector<cv::RotatedRect>& gridRect)
{
	gridRect.clear();
	rectShow = cv::Rect(0, 0, 0, 0);
	//�˵�ӳ��
	std::vector<cv::Point> tempPoints;
	point_To_point(pts, tempPoints);
	//�жϵ��Ƿ���ͼ����
	for (int i = 0; i < tempPoints.size(); i++)
	{
		/*if ((tempPoints[i].x < 0) || (tempPoints[i].x >= src.cols) || (tempPoints[i].y < 0) || (tempPoints[i].y >= src.rows))
		{
			cout << "error7   ";
			return false;
		}*/
		if (tempPoints[i].x < 1)
			tempPoints[i].x = 1;
		if(tempPoints[i].x > (src.cols - 2))
			tempPoints[i].x = src.cols-2;
		if (tempPoints[i].y< 1)
			tempPoints[i].y = 1;
		if (tempPoints[i].y > (src.rows - 2))
			tempPoints[i].y = src.rows - 2;
	}
	cv::Rect tempRect = boundingRect(tempPoints);
	//����1/8
	int rectX, rectY, rectW, rectH;
	rectX = tempRect.x - tempRect.width / 8;
    if (rectX < 0)
		rectX = 0;
	rectY = tempRect.y - tempRect.height / 8;
	if (rectY < 0)
		rectY = 0;
	rectW = tempRect.width + tempRect.width / 4;
	if ((rectX + rectW)>src.cols)
		rectW = src.cols - rectX;
	rectH = tempRect.height + tempRect.height / 4;
	if ((rectY + rectH)>src.rows)
		rectH = src.rows - rectY;
	if ((rectW <= 0) || (rectH <= 0))
	{
		cout << "error8   ";
		return false;
	}
	tempRect = cv::Rect(rectX, rectY, rectW, rectH);
	rectShow = tempRect;
	//����Ȥ����Ԥ����
	Mat img;
	img = src(tempRect);
	Mat imgGray;
	cvtColor(img, imgGray, CV_BGR2GRAY);
	Mat imgBin;
	threshold(imgGray, imgBin, 0, 255, THRESH_OTSU);
	//imshow("��ֵ����", imgBin);
	//waitKey(0);
	std::vector<std::vector<cv::Point>> contours;//��⵽������
	std::vector<cv::Vec4i> hierarchy;//�������
	Mat imgContours;
	imgBin.copyTo(imgContours);//����һ��ͼ�����ڼ������
	cv::findContours(imgContours, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point(0, 0));
	if (contours.size() < 9)
	{
		cout << "error9   ";
		return false;
	}
	std::vector<cv::RotatedRect> contoursRect;
	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() < 150)
			continue;
		cv::RotatedRect tempRotateRect = minAreaRect(contours[i]);
		if (tempRotateRect.size.width < tempRotateRect.size.height)//����Ź�����һ�������Ȥ����Ĵ���εĿ��С�ڸ߶ȣ���������
		{
			float temp = tempRotateRect.size.width;
			tempRotateRect.size.width = tempRotateRect.size.height;
			tempRotateRect.size.height = temp;
			tempRotateRect.angle += 90;
		}
		contoursRect.push_back(tempRotateRect);
	}
	if (contoursRect.size()<9)
	{
		cout << "error10   ";
		return false;
	}
	sort(contoursRect.begin(), contoursRect.end(), lessmark_rotateArea);//�����������
	cv::RotatedRect temp = contoursRect[8];
	for (int i = 0; i < contoursRect.size(); i++)
	{
		float W = contoursRect[i].size.width;
		float H = contoursRect[i].size.height;
		//�������ſ�һЩ �����ʶ���һ���������ǲ���һ����ʶ�𲻵�
		if ((W<temp.size.width*0.5) || (W>temp.size.width*1.5) || (H<temp.size.height*0.5) || (H>temp.size.height*1.5))
		{
			contoursRect.erase(contoursRect.begin() + i);
			i--;
		}
	}
	if (contoursRect.size() < 9)
	{
		cout << "error11   ";
		return false;
	}
	else if (contoursRect.size() > 9)
	{
		sort(contoursRect.begin(), contoursRect.end(), lessmark_rotateArea);//�ٴΰ��������
																			//ѡȡ������ľŸ�
		contoursRect.erase(contoursRect.begin() + 9, contoursRect.end());
	}
	for (int i = 0; i < contoursRect.size(); i++)
	{
		cv::RotatedRect temprotateRect = contoursRect[i];
		temprotateRect.center.x = temprotateRect.center.x + tempRect.x;
		temprotateRect.center.y = temprotateRect.center.y + tempRect.y;
		gridRect.push_back(temprotateRect);
	}
	return true;
}

//���������������
bool sbol::calFireRect(const cv::Mat& src, std::vector<cv::Point2f>& pts, std::vector<cv::Rect>& fireRect)
{
	fireRect.clear();
	rectShow = cv::Rect(0, 0, 0, 0);
	//�˵�ӳ��
	//�˵�ӳ��
	std::vector<cv::Point> tempPoints;
	point_To_point(pts, tempPoints);
	//�жϵ��Ƿ���ͼ����
	for (int i = 0; i < tempPoints.size(); i++)
	{
		/*if ((tempPoints[i].x < 0) || (tempPoints[i].x >= src.cols) || (tempPoints[i].y < 0) || (tempPoints[i].y >= src.rows))
		{
			cout << "Fire_error7   ";
			return false;
		}*/
		if (tempPoints[i].x < 1)
			tempPoints[i].x = 1;
		if (tempPoints[i].x >(src.cols - 2))
			tempPoints[i].x = src.cols - 2;
		if (tempPoints[i].y< 1)
			tempPoints[i].y = 1;
		if (tempPoints[i].y >(src.rows - 2))
			tempPoints[i].y = src.rows - 2;
	}
	cv::Rect tempRect = boundingRect(tempPoints);
	//����1/10
	int rectX, rectY, rectW, rectH;
	rectX = tempRect.x;
	rectW = tempRect.width;
	rectY = tempRect.y - tempRect.height / 10;
	if (rectY < 0)
		rectY = 0;
	rectH = tempRect.height + tempRect.height / 5;
	if ((rectY + rectH)>src.rows)
		rectH = src.rows - rectY;
	if (rectH <= 0)
	{
		cout << "Fire_error8   ";
		return false;
	}
	tempRect = cv::Rect(rectX, rectY, rectW, rectH);
	rectShow = tempRect;
	//����Ȥ����Ԥ����
	Mat img;
	img = src(tempRect);
	Mat imgRed;
	std::vector<cv::Mat> imgChannels;
	split(img, imgChannels);
	imgRed = imgChannels.at(2);
	Mat imgBin;
   threshold(imgRed, imgBin, 0, 255, THRESH_OTSU);
	cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));//����˺���
	cv::erode(imgBin, imgBin, element1);
	cv::dilate(imgBin, imgBin, element1);
	cv::dilate(imgBin, imgBin, element1);
	cv::dilate(imgBin, imgBin, element1);
	//imshow("imgBin:", imgBin);
	std::vector<std::vector<cv::Point>> contours;//��⵽������
	std::vector<cv::Vec4i> hierarchy;//�������
	Mat imgContours;
	imgBin.copyTo(imgContours);//����һ��ͼ�����ڼ������
	cv::findContours(imgContours, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point(0, 0));
	if (contours.size() < 9)
	{
		cout << "Fire_error9   ";
		return false;
	}
	std::vector<cv::Rect> contoursRect;
	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() < 100)
			continue;
		cv::Rect tempRect1 = boundingRect(contours[i]);
		double rateHW = (double)(tempRect1.height) / (double)(tempRect1.width);
		if (rateHW > 0.7&&rateHW < 2.8)
		{
			contoursRect.push_back(tempRect1);
		}
	}
	if (contoursRect.size() < 9)
	{
		cout << "Fire_error10   ";
		return false;
	}
	else if (contoursRect.size() > 9)
	{
		//ѡȡ������ľŸ�
		sort(contoursRect.begin(), contoursRect.end(), sortDown);
	}
	for (int i = 0; i < 9; i++)
	{
		cv::Rect tempRect2 = contoursRect[i];
		tempRect2.x = tempRect2.x + tempRect.x;
		tempRect2.y = tempRect2.y + tempRect.y;
		fireRect.push_back(tempRect2);
	}
	//imshow("Red_image:", imgRed);
	//imshow("Fire_image:", imgBin);
	return true;
}
//============= ����������ʶ�� =============//
void sbol::numRecARab(const cv::Mat& src, std::vector<cv::RotatedRect>& numRects, std::vector<myResult>& vResult)
{
	vResult.clear();
	for (int i = 0; i < numRects.size(); i++)
	{
		int x = numRects[i].center.x - numRects[i].size.width / 2;
		if (x < 0)
			x = 0;
		int y = numRects[i].center.y - numRects[i].size.height / 2;
		if (y < 0)
			y = 0;
		int w = numRects[i].size.width;
		if ((x + w)>src.cols)//����������ľ��ο�ȴ���ԭͼ����������п�
			w = src.cols - x - 1;
		int h = numRects[i].size.height;
		if ((y + h)>src.rows)//����������ľ��θ߶ȴ���ԭͼ����������иߣ�
			h = src.rows - y - 1;
		if ((w <= 0) || (h <= 0))
			return;
		Mat img = src(Rect(x, y, w, h));
		Mat img_gray;
		cvtColor(img, img_gray, CV_BGR2GRAY);
		//����
		Point center = Point(w / 2, h / 2);
		double angle = numRects[i].angle;//����б�ĽǶȣ�Ϊ����ת
		Mat rotmat = getRotationMatrix2D(center, angle, 1);//���ڻ�ñ任����rotmat,������ת
		Mat img_rotate;
		warpAffine(img_gray, img_rotate, rotmat, img_gray.size());//��תƽ��

		threshold(img_rotate, img_rotate, 0, 255, THRESH_OTSU);
		EliminateEdge(img_rotate, img_rotate);//ȥ����ɫ��Ե
		img_rotate = ~img_rotate;//�����ֱ�ɰ׵ģ�������ɺڵ�
		if (img_rotate.cols <= 15 || img_rotate.rows <= 15)
		{
			cout << "error12   ";
			return;
		}
		Mat imgRoi = img_rotate(Rect(5, 5, img_rotate.cols - 10, img_rotate.rows - 10));//��������С��10������																			//imshow("13423", imgRoi);
		cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));//����˺���
		cv::dilate(imgRoi, imgRoi, element);
		std::vector<std::vector<cv::Point>> contours;//��⵽������
		std::vector<cv::Vec4i> hierarchy;//�������
		cv::Mat imgContours;
		img_rotate.copyTo(imgContours);//����һ��ͼ�����ڼ������
		//imshow("img_rotate:", img_rotate);
		cv::findContours(imgContours, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, cv::Point(0, 0));
		//һ��ֻ��⵽��һ����Χ����
		std::vector<cv::Rect> contoursRect;
		for (int i = 0; i < contours.size(); i++)
		{
			if (contours[i].size() > 30)
				contoursRect.push_back(boundingRect(contours[i]));
		}
		if (!contoursRect.size())
		{
			cout << "error13   ";
			return;
		}
		sort(contoursRect.begin(), contoursRect.end(), sortDown); //�������Ƚϣ���ʵֻ��һ��
		Mat img_num = img_rotate(contoursRect[0]);
		Mat img_test;
		img_num.copyTo(img_test);
		resize(img_test, img_test, Size(sizeX, sizeY)); //���ͳһ��С
		threshold(img_test, img_test, 245, 1, THRESH_BINARY);
		uchar DataBuff[216] = { 0 };
		int adr = 0;
		for (int a = 0; a < sizeY; a += 2)
		{
			for (int b = 0; b < sizeX; b += 2)
			{
				char cnt = 0;
				for (int n = 0; n < 2; n++)
				{
					for (int m = 0; m < 2; m++)
					{
						if (img_test.at<uchar>(a + n, b + m) == 1)
							cnt++;
					}
				}
				DataBuff[adr] = cnt;
				adr++;
			}
		}
		Mat tempImg(1, featureLen / 4, CV_32FC1, Scalar(0));
		float* p = (float*)tempImg.data;
		for (int n = 0; n < featureLen / 4; n++)
		{
			p[n] = (float)DataBuff[n];
		}
		int result = svm->predict(tempImg);
		myResult tempResult;
		tempResult.value = result;
		tempResult.position = Point(numRects[i].center.x, numRects[i].center.y);
		vResult.push_back(tempResult);
	}
	//����
	int sum_x = 0;
	int sum_y = 0;
	Point center_point;
	for (int i = 0; i < vResult.size(); i++)
	{
		sum_x += vResult[i].position.x;
		sum_y += vResult[i].position.y;
	}
	center_point.x = (int)(sum_x / vResult.size());
	center_point.y = (int)(sum_y / vResult.size());
	int w = numRects[0].size.width / 2;
	int h = numRects[0].size.width / 2;
	for (int i = 0; i < vResult.size(); i++)
	{
		int t_err_x = vResult[i].position.x - center_point.x;
		int t_err_y = vResult[i].position.y - center_point.y;
		
		if (t_err_x < 0 && t_err_y < 0 && abs(t_err_x) > w && abs(t_err_y) > h)
			vResult[i].Index = 1;
		else if (t_err_y < 0 && abs(t_err_x) < w&&abs(t_err_y) > h)
			vResult[i].Index = 2;
		else if (t_err_x > 0 && t_err_y < 0 && abs(t_err_x) > w && abs(t_err_y) > h)
			vResult[i].Index = 3;
		else if (t_err_x < 0 && abs(t_err_x) > w&&abs(t_err_y) < h)
			vResult[i].Index = 4;
		else if (abs(t_err_x) < w&&abs(t_err_y) < h)
			vResult[i].Index = 5;
		else if (t_err_x > 0 && abs(t_err_x) > w&&abs(t_err_y) < h)
			vResult[i].Index = 6;
		else if (t_err_x < 0 && t_err_y > 0 && abs(t_err_x) > w && abs(t_err_y) > h)
			vResult[i].Index = 7;
		else if (t_err_y > 0 && abs(t_err_x) < w&&abs(t_err_y) > h)
			vResult[i].Index = 8;
		else if (t_err_x > 0 && t_err_y > 0 && abs(t_err_x) > w && abs(t_err_y) > h)
			vResult[i].Index = 9;
	}
	sort(vResult.begin(), vResult.end(), lessmark_myResult);
	for (int i = 0; i < 9; i++)
		ArrayResult[i] = vResult[i].value;
	return;
}
//������Ե
void sbol::EliminateEdge(Mat &imgSrc, Mat &dst)
{
	for (int i = 0; i < dst.rows; i++)   //��ѭ��
	{
		for (int j = 0; j < dst.cols; j++)
		{
			uchar* data = dst.ptr<uchar>(i);  //��ȡ��i�е��׵�ַ
			if (data[j] == 0)
			{
				data[j] = 255;
			}
			else
			{
				break;
			}
		}
		for (int j = dst.cols - 1; j > -1; j--)
		{
			uchar* data = dst.ptr<uchar>(i);  //��ȡ��i�е��׵�ַ
			if (data[j] == 0)
			{
				data[j] = 255;
			}
			else
			{
				break;
			}
		}
	}

	for (int i = 0; i < dst.cols; i++)   //��ѭ��
	{
		for (int j = 0; j < dst.rows; j++)
		{
			uchar* data = dst.ptr<uchar>(j);  //��ȡ��i�е��׵�ַ
			if (data[i] == 0)
			{
				data[i] = 255;
			}
			else
			{
				break;
			}
		}
		for (int j = dst.rows - 1; j > -1; j--)
		{
			uchar* data = dst.ptr<uchar>(j);  //��ȡ��i�е��׵�ַ
			if (data[j] == 0)
			{
				data[j] = 255;
			}
			else
			{
				break;
			}
		}
	}
}

//============= ��������ʶ�� ===============
void sbol::numRecFire(const cv::Mat& src, std::vector<cv::Rect>& numRects, std::vector<myResult>& vResult)
{
	vResult.clear();
	for (int i = 0; i < numRects.size(); i++)
	{
		Mat img = src(numRects[i]);
		Mat imgGray;
		cvtColor(img, imgGray, CV_BGR2GRAY);
		Scalar meaValue = mean(imgGray);//��Ҷ�ͼ��ľ�ֵ
		imgGray = imgGray - meaValue[0];//�Ҷ�ͼ��-��ɫͨ���ĻҶȾ�ֵ
		double maxValue;
		cv::minMaxLoc(imgGray, 0, &maxValue, 0, 0);//��ͼ����������ֵ
		Mat imgCanny;
		Canny(imgGray, imgCanny, maxValue, maxValue/4);//���ö�̬��ֵ
		//imshow("imgCanny:", imgCanny);
		Mat imgTest;
		imgCanny.copyTo(imgTest);
		cv::resize(imgTest, imgTest, cv::Size(24, 36), (0, 0), (0, 0), cv::INTER_LINEAR); //���ͳһ��С
		threshold(imgTest, imgTest, 100, 1, THRESH_BINARY);
		uchar DataBuff[216] = { 0 };
		int adr = 0;
		for (int a = 0; a < sizeY; a += 2)
		{
			for (int b = 0; b < sizeX; b += 2)
			{
				char cnt = 0;
				for (int n = 0; n < 2; n++)
				{
					for (int m = 0; m < 2; m++)
					{
						if (imgTest.at<uchar>(a + n, b + m) == 1)
							cnt++;
					}
				}
				DataBuff[adr] = cnt;
				adr++;
			}
		}
		Mat tempImg(1, featureLen / 4, CV_32FC1, Scalar(0));
		float* p = (float*)tempImg.data;
		for (int n = 0; n < featureLen / 4; n++)
		{
			p[n] = (float)DataBuff[n];
		}
		int result = svm2->predict(tempImg);
		myResult tempResult;
		tempResult.value = result;
		tempResult.position = Point(numRects[i].x + numRects[i].width / 2, numRects[i].y + numRects[i].height / 2);
		vResult.push_back(tempResult);
	}
	//����
	int sum_x = 0;
	int sum_y = 0;
	Point center_point;
	for (int i = 0; i < vResult.size(); i++)
	{
		sum_x += vResult[i].position.x;
		sum_y += vResult[i].position.y;
	}
	center_point.x = (int)(sum_x / vResult.size());
	center_point.y = (int)(sum_y / vResult.size());
	int w = numRects[0].height;
	int h = numRects[0].height;
	for (int i = 0; i < vResult.size(); i++)
	{
		int t_err_x = vResult[i].position.x - center_point.x;
		int t_err_y = vResult[i].position.y - center_point.y;

		if (t_err_x < 0 && t_err_y < 0 && abs(t_err_x) > w && abs(t_err_y) > h)
			vResult[i].Index = 1;
		else if (t_err_y < 0 && abs(t_err_x) < w&&abs(t_err_y) > h)
			vResult[i].Index = 2;
		else if (t_err_x > 0 && t_err_y < 0 && abs(t_err_x) > w && abs(t_err_y) > h)
			vResult[i].Index = 3;
		else if (t_err_x < 0 && abs(t_err_x) > w&&abs(t_err_y) < h)
			vResult[i].Index = 4;
		else if (abs(t_err_x) < w&&abs(t_err_y) < h)
			vResult[i].Index = 5;
		else if (t_err_x > 0 && abs(t_err_x) > w&&abs(t_err_y) < h)
			vResult[i].Index = 6;
		else if (t_err_x < 0 && t_err_y > 0 && abs(t_err_x) > w && abs(t_err_y) > h)
			vResult[i].Index = 7;
		else if (t_err_y > 0 && abs(t_err_x) < w&&abs(t_err_y) > h)
			vResult[i].Index = 8;
		else if (t_err_x > 0 && t_err_y > 0 && abs(t_err_x) > w && abs(t_err_y) > h)
			vResult[i].Index = 9;
	}
	sort(vResult.begin(), vResult.end(), lessmark_myResult);
	for (int i = 0; i < 9; i++)
		ArrayResult[i] = vResult[i].value;
	return;
}
//============= ʶ��ҳ ==================//
/*bool sbol::recPageChange(std::vector<myResult>& vResult, std::vector<myResult>& vlsResult, bool& flag)
{
	bool tempFlag = false;
	if (vResult.size() != 9)
		return tempFlag;
	int tempCnt = 0;
	for (int i = 0; i < vResult.size(); i++)
	{
		if (vResult[i].value != vlsResult[i].value)
			tempCnt++;
	}
	if ((tempCnt >= 0) && (tempCnt < 5)&&(flag == false))
	{
		return tempFlag;
	}
	if ((tempCnt >= 5) && (flag == false))
	{
		flag = true;
		vlsResult.clear();
		for (int i = 0; i < vResult.size(); i++)
		{
			vlsResult.push_back(vResult[i]);
		}
		return tempFlag;
	}
	if ((tempCnt <= 3) && (flag == true))
	{
		flag = false;
		vlsResult.clear();
		for (int i = 0; i < vResult.size(); i++)
		{
			vlsResult.push_back(vResult[i]);
		}
		tempFlag = true;
		cout << "��ҳ   ";
		return tempFlag;
	}
}*/

//=============Ŀ��ѡ�� ==================//
void sbol::chooseTarget(std::vector<myResult>& vResult, int psValue, myResult& target)
{
	if (vResult.size() != 9)
		return;
	int cnt = 0;
	for (int i = 0; i < vResult.size(); i++)
	{
		if (vResult[i].value == psValue)
			cnt++;
	}
	if (cnt == 0)
	{
		for (int i = 1; i < 10; i++)
		{
			int cnt1 = 0;
			for (int j = 0; j < 9; j++)
			{
				if (i == vResult[j].value)
				{
					cnt1++;
					if (cnt1 == 2)
					{
						target = vResult[j];
						return;
					}
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < vResult.size(); i++)
		{
			if (vResult[i].value == psValue)
			{
				target = vResult[i];
				return;
			}
		}
	}
}

cv::Point2f sbol:: get_pattern_shot_3D(cv::Matx33d K, cv::Point2f target, Eigen::Matrix4d cam_shot, double depth)
{
	Eigen::Vector4d points;
	const double cx = K(0, 2);
	const double cy = K(1, 2);
	const double fx = K(0, 0);
	const double fy = K(1, 1);

	//�õ�pattern-cam����
	int u = target.x;
	int v = target.y;
	Eigen::Vector4d point;
	point[3] = 1;
	point[2] = depth;
	point[0] = (u - cx)*point[2] / fx;
	point[1] = (v - cy)*point[2] / fy;
	point[0] += 0.14;
	point[1] += 0.135;
	point[2] += 0.105;
	points= point;
	//�õ�pattern-shot�����
	//points = cam_shot * points;
	//�õ��Ƕ�
	cv::Point2f angle;
	angle.x = (float)(atan(points(0) / points(2)) / 3.1415926 * 180);
	angle.y = (float)(atan(points(1) / points(2)) / 3.1415926 * 180);
	return angle;
}

cv::Point2f sbol::calAngle(int Index, cv::Mat& r, cv::Mat& t)
{
	Eigen::Vector4d pointW;
	pointW[0] = posTarget[Index].x;
	pointW[1] = posTarget[Index].y;
	pointW[2] = posTarget[Index].z;
	pointW[3] = 1;

	Eigen::Vector3d rgb_origin_out(t.at<double>(0,0), t.at<double>(1, 0), t.at<double>(2, 0));
	Eigen::Matrix3d rgb_btm;
	rgb_btm << r.at<double>(0,0), r.at<double>(0, 1), r.at<double>(0, 2), r.at<double>(1, 0), r.at<double>(1, 1), r.at<double>(1, 2), r.at<double>(2, 0), r.at<double>(2, 1), r.at<double>(2, 2);
	cam_shot = EigenFrom3(rgb_origin_out, rgb_btm);
	Eigen::Vector4d pointC;
	pointC = cam_shot * pointW;
	//������ͷ����ϵԭ��ƽ�Ƶ���̨����ϵԭ��
	pointC[0] += 0.14;
	pointC[1] += 0.135;
	pointC[2] += 0.105;
	//������̨�Ƕ�
	cv::Point2f angle;
	angle.x = (float)(atan(pointC(0) / pointC(2)) / 3.1415926 * 180);
	angle.y =(float)(atan(pointC(1) / pointC(2)) / 3.1415926 * 180);
	return angle;
}
//============= ���������ú��� ================//
void sbol::BigPower()
{
	Mat ImageGray;
	cvtColor(sbolImage, ImageGray, CV_BGR2GRAY);
	Scalar meanValue = mean(ImageGray);
	meanGrayValue = meanValue(0);
	cout << "&&&&&&&&&&&&&&&&&&&& meanGrayValue:" << meanGrayValue << endl;
	//ͼ��Ԥ����
	imgPretreatment(sbolImage, preImage);
	if (findDigitronLightRoi(preImage, digitronLightRect))
	{
		//KNNʶ������
		if (recPassWord(sbolImage, digitronLightRect, psWord, observationPoints, R, T,Depth))
		{
			//���û���˳��
			resetShootCnt(psWord, lstPsWord, observeCnt, errCnt, hitCnt, shootFlag);//���ô��˳��
			if (shootFlag == true)
			{
				if (SymbolModel == numArab)
				{
					if (calGirdRect(sbolImage, observationPoints, calibrateRects))
						numRecARab(sbolImage, calibrateRects, vrecResult);
					if (vrecResult.size() != 9)
						Depth = 0;
				}
				else if (SymbolModel == numFire)
				{
					if (calFireRect(sbolImage, observationPoints, calFireRects))//��������ʶ������
						numRecFire(sbolImage, calFireRects, vrecResult);
					if (vrecResult.size() != 9)
						Depth = 0;
				}
				if (recPageChange())//ʶ���˻�ҳ
				{
					//Ŀ������ѡ��(����Ŀ��)
					chooseTarget(vrecResult, psWord[hitCnt], tarGet);
					cv::Point2f tg = cv::Point2f(tarGet.position.x, tarGet.position.y);
					cv::Point2f Angle;
					cv::Matx33d tempK(K_);
					Angle = get_pattern_shot_3D(tempK, tg, cam_shot, Depth);
					shootCnt1++;
					if (shootCnt1 > 1)
					{
						angleY = Angle.x + corectAngY;
						angleP = Angle.y + corectAngP;
						shootCnt++;
						//angleY -= angleY / 10;
						//angleP -= angleP / 3;
						hitCnt++;
						if (hitCnt == 5)
							hitCnt = 0;
					}
					//Angle = calAngle(tarGet.Index-1, R, T);
				
				}
			}
		}
	}
	debugShow();
	std::cout << endl;
}

void sbol::debugShow()
{
	if (digitronLightRect.x >= 0)
	{
		//cv::rectangle(sbolImage, digitronLightRect, Scalar(0, 0, 255), 2);

		char c_number[] = "00000";
		for (int i = 0; i< 5; i++)
		{
			c_number[i] = psWord[i] + '0';
		}
		int passWordValue = atoi(c_number);	
		char s[30];
		sprintf_s(s, "PassWord:%d", passWordValue);
		putText(sbolImage, s, Point(50, 50), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);
	}
	if (rectShow.width > 0)
	{
		cv::Rect tempRect = cv::Rect(rectShow.x, rectShow.y + 25, rectShow.width, rectShow.height);
		cv::rectangle(sbolImage, tempRect, Scalar(255, 0, 0), 2);
	}
	for (int i = 0; i < calibrateRects.size(); i++)
	{
		cv::Rect tempRect = calibrateRects[i].boundingRect();
		cv::rectangle(sbolImage, tempRect, Scalar(0, 0, 255), 2);
		if (vrecResult.size() == 9)
		{
			char s1[5];
			sprintf_s(s1, "%d", vrecResult[i].value);
			putText(sbolImage, s1, Point(vrecResult[i].position.x-tempRect.width/2 - 5, vrecResult[i].position.y - tempRect.height / 2 - 5), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);
		}
	}
	for (int i = 0; i < calFireRects.size(); i++)
	{
		cv::Rect tempRect = calFireRects[i];
		cv::rectangle(sbolImage, tempRect, Scalar(0, 0, 255), 2);
		if (vrecResult.size() == 9)
		{
			char s1[5];
			sprintf_s(s1, "%d", vrecResult[i].value);
			putText(sbolImage, s1, Point(vrecResult[i].position.x - tempRect.width / 2 - 5, vrecResult[i].position.y - tempRect.height / 2 - 5), cv::FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 2);
		}
	}
	for (int i = 0; i < observationPoints.size(); i++)
		circle(sbolImage, observationPoints[i], 2, Scalar(255, 0, 0), 2);
	imshow("momoDebug:", sbolImage);
}
//ʶ��ҳ
bool sbol::recPageChange()
{
	bool flag = false;
	if (!pChangeFlag)
	{
		stopChange_cnt++;
		if (stopChange_cnt > 3)
		{
			int err_cnt_max = 0;
			int err_array[3] = { 0 };
			for (int i = 0; i < 9; i++)
			{
				if (ArrayResult[i] != ArrayResult1[i])
				{
					err_array[0]++;
				}
			}
			for (int i = 0; i < 9; i++)
			{
				if (ArrayResult[i] != ArrayResult2[i])
				{
					err_array[1]++;
				}
			}
			for (int i = 0; i < 9; i++)
			{
				if (ArrayResult[i] != ArrayResult3[i])
				{
					err_array[2]++;
				}
			}
			for (int i = 0; i < 3; i++)
			{
				if (err_array[i]>err_cnt_max)
				{
					err_cnt_max = err_array[i];
				}
			}
			if (err_cnt_max > 4)
			{
				pChangeFlag = true;
			}
		}
	}
	else
	{
		int err_cnt = 0;
		for (int i = 0; i < 9; i++)
		{
			if (ArrayResult[i] != ArrayResult1[i])
			{
				err_cnt++;
			}
		}
		if (err_cnt <=3)
		{
			flag = true;
			pChangeFlag = false;
			stopChange_cnt = 0;
			cout<< "======================= ��ҳ�ˣ�" << endl;
		}
	}
	for (int i = 0; i < 9; i++)
	{
		ArrayResult3[i] = ArrayResult2[i];
		ArrayResult2[i] = ArrayResult1[i];
		ArrayResult1[i] = ArrayResult[i];
	}
	return flag;
}








