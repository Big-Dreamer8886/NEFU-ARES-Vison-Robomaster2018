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

#include "stdafx.h"
#include "pid_cplus.h"
#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <iostream> 
#include "Armor_Detector.h"
#include "SerialPort.h"  
#include "pid_cplus.h"
#include "omp.h"

extern Armor_Detector armor;


float pid_cplus::PID_Control_normal(pid_cplus *pid, float Expected_value, float Actual_value)//, bool Pid_type)
{

	/*float point_value = 0;
	if (Actual_value<-20)
	{
		point_value = 130;
	}
	else if (Actual_value > 20)
	{
		point_value = -130;
	}
	pid->E = point_value - Actual_value;*/
	pid->E = Expected_value - Actual_value;

	//if (Pid_type)//����ʽ
	//{

	//	U += P * (E - PreE) +
	//		I * (E) +
	//		D * (E - 2 * PreE + PrePreE);
	//}
	//else//λ��ʽ
	//{
	pid->Intergral += pid->E;
	if (pid->Intergral >= pid->Ilimit)
		pid->Intergral = pid->Ilimit;
	else if (pid->Intergral <= -pid->Ilimit)
		pid->Intergral = -pid->Ilimit;
	else
		pid->Intergral = pid->Intergral;
	pid->U = pid->P * pid->E +
		//pid->I * pid->Intergral +
		pid->D * (pid->E - pid->PreE);
	//}


	if (pid->U >= pid->Ulimit)
		pid->U = pid->Ulimit;
	else if (pid->U <= -pid->Ulimit)
		pid->U = -pid->Ulimit;
	else
		pid->U = pid->U;

	pid->PrePreE = pid->PreE;
	pid->PreE = pid->E;

	return pid->U;

}


void pid_cplus::PID_Change_Pitch(pid_cplus *pid, float Expected_value, float Actual_value)
{
	pid->E = Expected_value - Actual_value;
	if (abs(pid->E) > 300)
	{
		pid->P = (pid->E*pid->E / 8000000) + 0.030;
	}
	if (abs(pid->E) > 200)
	{
		pid->P = (pid->E*pid->E / 4000000) + 0.030;
	}
	else if (abs(pid->E) > 100)
	{
		pid->P = (pid->E*pid->E / 2500000) + 0.035;
	}
	else
	{
		pid->P = (pid->E*pid->E / 1500000) + 0.0455;
	}
	//printf("E:%f\n", pid->E);
}



void pid_cplus::PID_Change_Yaw(pid_cplus *pid, float Expected_value, float Actual_value)
{
	/*float point_value = 0;
	if (Actual_value<-20)
	{
	point_value = 130;
	}
	else if (Actual_value > 20)
	{
	point_value = -130;
	}
	pid->E = point_value - Actual_value;*/
	/*if (armor.Offset_Y_error>5)
	{
	Expected_value += armor.VRlt_last.size.width;
	}*/
	pid->E = Expected_value - Actual_value;

	if (abs(pid->E) > 500)
	{
		pid->P = (pid->E*pid->E / 8000000) + 0.01;
	}
	else if (abs(pid->E) > 400)
	{
		pid->P = (pid->E*pid->E / 7000000) + 0.01;
	}
	else if (abs(pid->E) > 300)
	{
		pid->P = (pid->E*pid->E / 5000000) + 0.035;
	}
	else if (abs(pid->E) > 200)
	{
		pid->P = (pid->E*pid->E / 3500000) + 0.04;
	}
	else if (abs(pid->E) > 100)
	{
		pid->P = (pid->E*pid->E / 2500000) + 0.045;
	}
	else
	{
		pid->P = (pid->E*pid->E / 1500000) + 0.045;
	}
	//printf("E����Y:%f\n", pid->E);
}

void pid_cplus::PID_init(pid_cplus *pid, bool PORY)
{
	if (PORY)//P
	{
		pid->P = 1;
		pid->I = 0;
		pid->D = 0.06;
		pid->E = 0;
		pid->PreE = 0;
		pid->PrePreE = 0;
		pid->U = 0;
		pid->Intergral = 200;
		pid->Ilimit = 0;
		pid->Ulimit = 100;
	}
	else//Y
	{
		pid->P = 1;
		pid->I = 0;
		pid->D = 0.06;
		pid->E = 0;//ƫ��
		pid->PreE = 0;//��һ��ƫ��
		pid->PrePreE = 0;//���ϴ�ƫ��
		pid->U = 0;//����ƫ��
		pid->Intergral = 200;//����ƫ��
		pid->Ilimit = 0;//�����޷�
		pid->Ulimit = 50;//����޷�

	}


}

pid_cplus::pid_cplus()
{
}


pid_cplus::~pid_cplus()
{
}
