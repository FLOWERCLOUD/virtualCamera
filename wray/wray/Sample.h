#pragma once
#include "Sampler.h"
class WSample
{
	friend class WSampler;
	friend class WRandomSampler;
	friend class WStratifiedSampler;

	friend class WFilter;
	friend class WBoxFilter;
public:
	enum WSampleType{SAMPLE_1D=0,SAMPLE_2D=1,SAMPLE_3D=2};
	WSampleType type;
	//�����ʱ�򲢲�����ռ�
	WSample(WSampleType dimension,unsigned int size);
	virtual ~WSample(void);
	virtual void allocateSpace()=0;//�����ڴ�ռ�,ͬʱnthPoint��Ϊ0
	void clear();//�����ڴ�ռ�
//	void reset();//�ѵ�ǰ�±긴λ
	void setSize(unsigned int size);
	void setType(WSampleType itype){type=itype;}
	unsigned int getCurrPointIndex(){return nthPoint;}
	unsigned int getTotalPointNum(){return totalPoints;}
	virtual void display()=0;
protected:
	unsigned int size;
	float*pattern;
	unsigned int nthPoint;//��ǰ�Ĳ�����
	unsigned int totalPoints;//�����������
};
class WSample1D:public WSample
{
public:
	WSample1D(unsigned int size);
	void allocateSpace();
	bool get1D(float&x);
	void display();
};
class WSample2D:public WSample
{
public:
	WSample2D(unsigned int size);
	void allocateSpace();
	bool get2D(float&x,float&y);
	void display();
};
class WSample3D:public WSample
{
public:
	WSample3D(unsigned int size);
	void allocateSpace();
	bool get3D(float&x,float&y,float&z);
	void display();
};