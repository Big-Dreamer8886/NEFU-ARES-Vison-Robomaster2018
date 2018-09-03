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

#pragma once
#include "stdafx.h"
#include <opencv2/opencv.hpp>

//#define T_ANGLE_THRE 10 //�ǶȲ�ֵ
//#define T_ANGLE_THRE180 6 //������˽ǶȲ�ֵ
#define T_SIZE_THRE 5   //



//extern int	brightness;
//extern int	contrast;
//extern int	saturation;
//extern int	hue;
//extern int	exposure;
//extern char key;

//����
#define AimCenter_X 400 //ͼ������xֵ
#define AimCenter_Y 300 //ͼ������yֵ

#define T_ANGLE_THRE 5   //�����������ǶȲ�
#define T_ANGLE_THRE180 3   //�����������ǶȲ�
#define T_ANGLE_THREMIN 3   //����������С�ǶȲ�
#define T_ANGLE_THRE180MIN 2   //����������С�ǶȲ�

#define T_HIGH_RAT 0.2   //�ƴ����߲�� �����߶Ȳ�����������1/3
#define T_HIGH_RAT_ANGLE 0.34   //�ƴ��Ƕȷ�����С�Ƕȸ߲�� �����߶Ȳ�����������1/2


#define T_WHIDTH_RAT 0.4  //�ƴ�������   ������Ȳ�����������3/5
#define T_WHIDTH_RAT_ANGLE 0.55  //�ƴ�������

#define L_WH_RAT 0.8 // ������߱�

#define  BallisticCal_Accuracy 0.1               //������������ (��)
#define  BallisticCal_BallisticSpd 21.5        //�ӵ����ٶ� (m)

#define Yuntai_Y_P  0.003
#define Yuntai_Y_D  0.07

#define Yuntai_P_P  0.001
#define Yuntai_P_D  0.03

#define TeamBlue 0x00
#define TeamRed  0x01


class Armor_Detector
{

public:

	//hsv
	int BLowH = 80;
	int BHighH = 150;

	int BLowS = 40;
	int BHighS = 255;

	int BLowV = 100;
	int BHighV = 255;

	int RLowH = 150;
	int RHighH = 180;
	int RLowH_2 = 0;
	int RHighH_2 = 40;

	int RLowS = 40;
	int RHighS = 255;

	int RLowV = 100;
	int RHighV = 255;

	double solvepnp_x, solvepnp_y, solvepnp_z, Muzzle_compensation;//solvepnp�����xyz��ǹ�ڲ���
	int ourteam;



	int ang_P, ang_Y, Yaw_deviation,lost_canter;
	//Ԥ��
	int prve_armor_center_x;
	int prve_armor_center_y;
	int Deviation_prove_now;
	int	Pitch_deviation ;
	int left_follow_time ;
	int left_time_clearn ;
	int right_follow_time ;
	int right_time_clearn ;
	int follow_direction ;
	//�๹�캯��
	Armor_Detector();
	//����ͷ��ʼ��
	cv::VideoCapture capture_m; //�Ӿ���������ͷ
	bool camaraInit(int device);//�Ӿ���������ͷ��ʼ��
	//װ��ʶ��
	cv::Mat srcimage;//ԭͼ
	cv::Mat dstimage;//hsv��ȡ��ɫͼ
					 //solvepnp
	std::vector<cv::Point3f> corners;
	std::vector<cv::Point3f> corners_big;
	std::vector<cv::Point2f> observation_points;



	std::vector<cv::RotatedRect> VRlt;
	cv::RotatedRect VRlt_last;
	//����
	cv::RotatedRect LastTargetArmor;
	
	//��̨���ƣ����̣�
	int AutoShoot;
	int Offset_Y;
	int Offset_P;
	double Distance;
	double YunTai_P_coreect = 0;//��̨Pich�������
	double BallisticCalculation(double distance); //��������
	void Yuntai_Control(const cv::RotatedRect& tg_armor, int* offset_y, int* offset_p, bool flag);//��̨����
	void Armor_Detector_Solvepnp(cv::RotatedRect dst);//solvepnp���
	void Armor_Detector_pretreatment(cv::Mat src, cv::Mat& dst);
	void Armor_rio11_Detector(cv::Mat master, cv::Mat src, std::vector<cv::RotatedRect>& vRlt, cv::RotatedRect& last_vRlt);
	void autoShoot();
	~Armor_Detector();
};


class Armor_builded //װ�׹���
{
public:
	cv::RotatedRect armorS;
	int build1_No = 0;
	int build2_No = 0;
	int build_features[4];//�ǶȲ�߶������߶Ȳ��Ȳ�
	int vot = 0;
};
