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
class pid_cplus
{
public:
	//float Yaw_point=0;//Yԭ�㣻
	//float Pitch_point = 0;//Pԭ�㣻
	//�����ƫ����
	/*int ang_P;
	int ang_Y;*/
	//pid����
	float P;
	float I;
	float D;
	float E;
	float PreE;
	float PrePreE;
	float U;
	float Intergral;
	float Ilimit;
	float Ulimit;
	//���ַ���
	float E_max;
	//pidģʽѡ��
	bool Positional_PID = 0;
	bool Incremental_PID = 1;
	//pid����
	float PID_Control_normal(pid_cplus *pid,float Expected_value, float Actual_value);//, bool Pid_type);
	void PID_Change_Pitch(pid_cplus *pid, float Expected_value, float Actual_value);
	void PID_Change_Yaw(pid_cplus *pid, float Expected_value, float Actual_value);
	void PID_init(pid_cplus *pid,bool PORY);

	pid_cplus();
	~pid_cplus();
};

