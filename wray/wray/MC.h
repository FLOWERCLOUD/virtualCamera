#pragma once
//����������ֲ�������
class WMonteCarlo
{
public:
	WMonteCarlo(void);
	virtual ~WMonteCarlo(void);
	//�����������������������0-1������������ز���������ֵ
	//��Բ�β��������ص�λԲ�ڲ�������ֵ
	static float randomFloat();
	static void uniformSampleDisk(const float u1, const float u2, float &x, float &y);
	//�԰��������ʹ�ò���ֵ�������ҷֲ���PDFΪ��Ӧλ�õĸ����ܶȺ���
	static void cosineSampleHemisphere(const float u1, const float u2, WVector3 &sample,float&PDF);
	static void uniformSampleTriangle(float u1,float u2,
		float &u,float &v);
};
