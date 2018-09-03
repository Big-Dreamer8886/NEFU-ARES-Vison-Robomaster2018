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

#include<opencv2/opencv.hpp>
#include"SerialPort.h"
#include <Eigen\Dense>
struct myRect
{
	cv::Rect boundRect;
	float similarValue;
};

struct mypswRect
{
	cv::Rect boundRect;
	std::vector<cv::Point> contours;
	int value;
	float similarValue;
};

struct myResult
{
	int value; //���ֵ
	int Index;//λ������
	cv::Point position; //λ��
};

struct myPoint
{
	cv::Point pt;
	int sumXY;
	int sumXY_inv;
};
							 
enum symbolModel
{
	numArab,
	numFire
};


class sbol
{
public:
	cv::Mat R, T;
	//solve pnp
	cv::Mat K_;
	cv::Mat D_;
	std::vector<cv::Point3f> corners;
	double Depth;
	double angleY;
	double angleP;
	double corectAngY;
	double corectAngP;

	//�任����
	Eigen::Matrix4d cam_shot;

	//��С��ģʽ
	symbolModel SymbolModel;

	const int trainSample = 1;				  
	const int trainClasses = 9;
	const int sizeX = 24;
	const int sizeY = 36;
	const int featureLen = sizeX * sizeY;
	cv::Mat train_data, train_classes;
	//ʶ����д����ϵͳ
	cv::Ptr<cv::ml::KNearest> knn;
	cv::Ptr<cv::ml::SVM> svm;
	cv::Ptr<cv::ml::SVM> svm2;

	cv::VideoCapture Capture_BigPower;//����ͷ

	cv::Mat sbolImage;//ԭͼ��
	cv::Mat preImage;//Ԥ����ͼ��
	
	cv::Rect digitronLightRect;//�������������
	std::vector<cv::Point2f> observationPoints;
	std::vector<cv::RotatedRect> calibrateRects; //������������
	std::vector<cv::Rect> calFireRects; //������������
	cv::Rect rectShow;

	std::vector<myResult> vrecResult; //ʶ����
	std::vector<myResult> vlstrecResult; //��һ��ʶ����

	int meanGrayValue;

	//���ô��˳��
	int psWord[5]; //��ǰ����
	int lstPsWord[5]; //��һ�ε�����
	int hitCnt;
	int observeCnt; //�۲����
	int errCnt;
	bool shootFlag;
	int shootCnt;
	int shootCnt1;
	//ʶ��ҳ
	//bool observeFlag;
	bool pChangeFlag;
	int stopChange_cnt;
	int ArrayResult[9];
	int ArrayResult1[9];
	int ArrayResult2[9];
	int ArrayResult3[9];
	//Ŀ��ѡ��
	myResult tarGet;

	
	sbol();//ϵͳ��ʼ�����캯��
	Eigen::Matrix4d EigenFrom3(Eigen::Vector3d transl, Eigen::Matrix3d Rmat);
	void sampleInit(cv::Mat& trainData, cv::Mat& trainClass);//KNNѵ������װ��
	void trainpassword();//ѵ������ʶ����
	void loadSVM();//����SVM
	bool camaraInit(int device);//�������ͷ��ʼ��
	void imgPretreatment(const cv::Mat& src, cv::Mat& dst); //ͼ��Ԥ����
	bool findDigitronLightRoi(const cv::Mat& src, cv::Rect& roiRect);//Ѱ���������������
	bool recPassWord(const cv::Mat& src, cv::Rect& dlRect, int* resultArray,std::vector<cv::Point2f>& observationPts,cv::Mat& r,cv::Mat& t,double& depth);//ʶ������
	bool calGirdRect(const cv::Mat& src, std::vector<cv::Point2f>& pts,std::vector<cv::RotatedRect>& gridRect);//������д����ʶ����
	void EliminateEdge(cv::Mat &imgSrc, cv::Mat &dst); //������Ե
	bool calFireRect(const cv::Mat& src, std::vector<cv::Point2f>& pts, std::vector<cv::Rect>& fireRect);//��������ʶ������
	void numRecARab(const cv::Mat& src, std::vector<cv::RotatedRect>& numRects, std::vector<myResult>& vResult);//��д����ʶ��
	void numRecFire(const cv::Mat& src, std::vector<cv::Rect>& numRects, std::vector<myResult>& vResult);
	void resetShootCnt(int* ps_array, int* ls_ps_array, int& count, int& err_count, int& cnt, bool& flag);//���ô��˳��
	//bool recPageChange(std::vector<myResult>& vResult, std::vector<myResult>& vlsResult, bool& flag); //��ҳʶ��
	bool recPageChange();
	void chooseTarget(std::vector<myResult>& vResult, int psValue, myResult& target);
	//������̨�Ƕ�
	cv::Point2f get_pattern_shot_3D(cv::Matx33d K, cv::Point2f target, Eigen::Matrix4d cam_shot, double depth);
	cv::Point2f calAngle(int Index, cv::Mat& r, cv::Mat& t);
	void BigPower();//���������ú���
	void debugShow();

	//=========== ��� ==============//

	//�˵�ӳ��
	void point_To_point(std::vector<cv::Point2f>& src, std::vector<cv::Point>& dst);

};



#pragma once
