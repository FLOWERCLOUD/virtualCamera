#pragma once
#include "Vector3.h"

class WVector4
{
public:
	float x,y,z,h;
	//���캯��
	WVector4();//xyz����ʼ��Ϊ0.0f��hΪ1.0f
	WVector4(float i);
	WVector4(const WVector4&v);
	WVector4(const WVector3&v);//h����ʼ��Ϊ1.0f
	WVector4(const WVector2&v);//h����ʼ��Ϊ1.0f
	WVector4(float ix,float iy,float iz,float ih);

	//��������
	WVector4::~WVector4(void);
	//�ӷ�
	WVector4 operator+(const WVector4&i);
	WVector4 operator+=(const WVector4&i);

	//����
	WVector4 operator-(const WVector4&i);
	WVector4 operator-=(const WVector4&i);

	//����
	friend WVector4 operator*(const WVector4&i,float n);
	friend WVector4 operator*(float n,const WVector4&i);

	WVector4 operator*=(float f);

	//����
	friend WVector4 operator/(const WVector4&i,float n);
	friend WVector4 operator/(float n,const WVector4&i);

	WVector4 operator/=(float f);

	//����
	float length();
	float lengthSquared();
	//��λ��
	WVector4 normalize();

	//���
	float dot(const WVector4&i);
};
