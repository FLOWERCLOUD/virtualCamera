#pragma once
#include "Sample.h"
class WSample;
class WSampler
{
public:
	enum WSamplerType{SAMPLER_RANDOM=0,SAMPLER_STRATIFIED=1};
	WSamplerType type;
	WSampler(unsigned int iseed=65536,WSamplerType itype=WSampler::SAMPLER_RANDOM);

	//���麯�������۶���1D 2D����3D ������
	//����ͨ����������������������䷶Χ��0-1֮���ֵ
	//���ô˺���ʱ�����Ȼ��sample�ĵ�ǰ�����±��0
	virtual void computeSamples(WSample&s)=0;
	void setSeed(unsigned int iseed){seed=iseed;}
	virtual ~WSampler(void);
protected:
	unsigned int seed;
};
//��Sampler���������Sample��ֵ
//����򵥵Ĳ�����
class WRandomSampler:public WSampler
{
public:
	WRandomSampler(unsigned int iseed=65535):WSampler(iseed){}
	void computeSamples(WSample&s);
};
class WStratifiedSampler:public WSampler
{
public:
	WStratifiedSampler(unsigned int iseed=65535):WSampler(iseed){}
	void computeSamples(WSample&s);
private:
	void swapPoint(unsigned int ithPoint,unsigned int jthPoint,WSample&s);
};