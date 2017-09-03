#pragma once

class WVector2
{
public:
	float x;
	float y;
	//���캯��	
	WVector2();
	WVector2(float i);
	WVector2(const WVector2&v);
	WVector2(float ix,float iy);

	//��������
	WVector2::~WVector2(void){}
	//�ӷ�
	WVector2 operator+(const WVector2&i2);
	WVector2 operator+=(const WVector2&i2);

	//����
	WVector2 operator-(const WVector2&i2);
	WVector2 operator-=(const WVector2&i2);

	//����
	friend WVector2 operator*(const WVector2&i1,float n);
	friend WVector2 operator*(float n,const WVector2&i1);

	WVector2 operator*=(float f);

	//����
	friend WVector2 operator/(const WVector2&i1,float n);
	friend WVector2 operator/(float n,const WVector2&i1);

	WVector2 operator/=(float f);

	//����
	float length();
	float lengthSquared();

	//��λ��
	WVector2 normalize();

	//���
	float dot(const WVector2&i);

	//���ֵ������ʱ��
	void showCoords();
};
