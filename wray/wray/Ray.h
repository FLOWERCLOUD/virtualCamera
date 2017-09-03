#pragma once
#include "Vector3.h"
#include "MathValues.h"
#define RAY_EPSILON 1e-4f
class WRay
{
public:
	WVector3 point;
	WVector3 direction;
	float tMin,tMax;			//tΪ��Ч������Χ
	WRay(void);
	//ע��iDirection������ֱ�Ӵ�������������
	WRay(const WVector3&iDirection);
	WRay(const WVector3&iPoint,const WVector3&iDirection);
	WRay(const WVector3&iPoint,const WVector3&iDirection,float iTMin,float iTMax);
	virtual ~WRay(void);
	void normalizeDir();	//�ѷ���������λ��
	void inverseDir();		//��ת���߷���
	void reflect(WVector3 normal);//���㾵�淴��Ĺ��߷���
	void draw();			//��openGL�������ߣ�����ʱ��
	void drawSegment();		//��openGL�������߲�����Χ�ڵ�һ�Σ�����ʱ��
};
