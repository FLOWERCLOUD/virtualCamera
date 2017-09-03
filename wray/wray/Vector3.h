#pragma once
#include "Vector2.h"

class WVector3
{
public:
	//�����ȿ���ͨ�� x y z ���ʣ�Ҳ�������������
	union
	{
		struct
		{
			float x,y,z;
		};
		float v[4];
	};
//	float x,y,z;
	//���캯��
	WVector3();
	WVector3(float i);
	WVector3(const WVector3&v);
	WVector3(const WVector2&v);
	WVector3(float ix,float iy,float iz);

	//��������
	WVector3::~WVector3(void);
	//�ӷ�
	WVector3 operator+(const WVector3&i)const;
	WVector3 operator+=(const WVector3&i);

	//����
	WVector3 operator-(const WVector3&i)const;
	WVector3 operator-=(const WVector3&i);

	//����
	friend WVector3 operator*(const WVector3&i,float n);
	friend WVector3 operator*(float n,const WVector3&i);
	friend WVector3 operator*(const WVector3&i1,const WVector3&i2);

	WVector3 operator*=(float f);
	WVector3 operator*=(const WVector3&i);

	//����
	friend WVector3 operator/(const WVector3&i,float n);
	friend WVector3 operator/(float n,const WVector3&i);
	friend WVector3 operator/(const WVector3&i,const WVector3&n);

	WVector3 operator/=(float f);
	WVector3 operator=(const WVector3&i);

	bool operator==(const WVector3& i)const;

	//����
	float length()const;
	float lengthSquared()const;
	//��λ��
	WVector3 normalize();
	//���㷴������
	WVector3 reflect(const WVector3 normal)const;
	WVector3 sqrtElement()const;

	//���
	float dot(const WVector3&i)const;
	//���
	WVector3 cross(const WVector3&i)const;

	//���ֵ������ʱ��
	void showCoords()const;
};
