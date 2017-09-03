#pragma once
#include "Vector3.h"
#include "Triangle.h"

class WBoundingBox
{
public:
	static float delta;
	WVector3 pMin;
	WVector3 pMax;

	WBoundingBox(void);
	WBoundingBox(const WTriangle&it, bool isBigger = true);
	WBoundingBox(WVector3 ipMin,WVector3 ipMax);
	WBoundingBox(float box[2][3]);
	virtual ~WBoundingBox(void);
	//���з�������bbox��ñ�ģ�ʹ�һ��
	void merge(const WVector3&point,bool isBigger = true);
	void merge(const WBoundingBox&iBox);
	void merge(const WTriangle&t,bool isBigger = true);
	//������һ��bbox���ཻ����
	bool intersect(const WBoundingBox& iBox, WBoundingBox& resultBox);
	//���ģ�͵�ʵ��bbox���꣬����������˵�bbox
	void getTightBound(WVector3& tightMin, WVector3& tightMax);
	void draw()const;
	//����һ������
	float halfArea();
	float area();
	//��ʾ����ֵ
	void displayCoords()const;
	//�������Χ���󽻣����ز�����Χ������Ϊ������
	bool isIntersect(const WRay&r,
		float&tMin,float&tMax);
	//���һ�����ǲ����ڰ�Χ�����棬�����պ��ڰ�Χ�б����ϵĵ�
	bool isInBoxInclusive(const WVector3& point);
	//���һ�����ǲ����ڰ�Χ�����棬�����պ��ڰ�Χ�б����ϵĵ�
	bool isInBoxExclusive(const WVector3& point);
	//����һ���Ѿ�ȷ������bbox����ĵ�
	//ȷ�����Ƿ���һ�����ϣ�����ڣ�����һ��������
	//face��ʾ������
	// 0 Ϊx���棬 1 Ϊx����
	// 2 Ϊy���棬 3 Ϊy����
	// 4 Ϊz���棬 5 Ϊz����
	bool onFace(WVector3& point, int& face);
	//�������Ƿ����Χ���ཻ
	//�ҳ����귶Χ��������,x����Ϊ0��y����Ϊ1��z����Ϊ2
	//tMin,tMaxΪ���߽���Ĳ���ֵ
	int maxAxis();	
	WBoundingBox operator=(const WBoundingBox&box);
	bool operator== (const WBoundingBox& box)const;
private:
	inline void swap(float&t1,float&t2);
};
