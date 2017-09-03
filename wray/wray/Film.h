#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Filter.h"
//Film�ഢ����Ⱦ�����ص����ɫ
//��ӵ��һ��filter�ӿڣ�
//filter�ӿڸ���ͨ�����������ɫ��������ص����ɫ
//Ȼ��ͨ��film��setColor��������Film��Ӧ���ص����ɫ
class WFilm
{
public:
	WFilm(unsigned int resX=640,unsigned int resY=480,
		unsigned int sampleSize=1,
		WFilter::WFilterType filterType=WFilter::FILTER_BOX,
		WSampler::WSamplerType samplerType=WSampler::SAMPLER_RANDOM,
		unsigned int seed=65536);
	virtual ~WFilm(void);

	//���÷ֱ���
	void setResolution(unsigned int resX,unsigned int resY);
	WVector2 getResolution()
	{return WVector2(resolutionX,resolutionY);}


	//����ָ������λ�õ���ɫ������ԭ������Ļ���½�
	WVector3 getColor(unsigned int x,unsigned int y);
	void setColor(unsigned int x,unsigned int y,float R,float G,float B);
	void accumulateColor(unsigned int x,unsigned int y,float R,float G,float B);


	//��ȡ��ǰ�Ĳ�����λ�ã�λ��ֵ����0-1֮��
	//����PathIntegrator��recursivePathIntegrator��IrradianceIntegrator,
	//�˺���Ӧ���������setSampleColor���ʹ��
	//���ȵ��ô˺����������������ߵ�λ��
	//��ɹ��߸��ٺ�ѷ��ص���ɫֵ����setSampleColor
	//��һ�����صĲ����㶼�����Ժ�
	//setSampleColor�ڱ�Ҫʱ��ͬʱ����������ص����ɫ
	//����MetropolisIntegrator�����ô��ַ���
	//��Ϊ����setColor������ֱ���������ص���ɫ
	void getSamplePosition(float&posX,float&posY);
	//���ò��������ɫ����Ҫʱ������ɫ����
	void setSampleColor(float R,float G,float B);


	//openGL��ͼ����,x y Ϊ�������ص����½�����
	void draw(float x=0,float y=0,float z=0);
	//������ͳһ���ó�һ����ɫ
	void cleanColors(float R=1,float G=1,float B=1);

	bool isFull();//����Ƿ��������ض������������
	//��õ�ǰ������λ��


	void getCurrPos(unsigned int&icurrX,unsigned int&icurrY)
	{icurrX=currX;icurrY=currY;}
	//���õ�ǰҪ��Ⱦ������
	void setCurrPos(unsigned int icurrX,unsigned int icurrY)
	{currX=icurrX;currY=icurrY;}


	void changeSampleSize(unsigned int size);
	void changeSampler(WSampler::WSamplerType type);

	int*getBitPointer(){return bitColors;}
	float* getFloatPointer(){return colors;}

	//�����ӵ�һ�����ؿ�ʼ�ع�
	void nextExposurePass();

private:
	unsigned int resolutionX,resolutionY;
	float*colors;			//������ɫ����,��Χ��0��1
	float*counts;			//���ؼ������飬���ڶԽ��ȡƽ��
	int*bitColors;			//������ɫ����,��Χ��0��255
							//ÿ��Ԫ�ذ���00RRGGBB��˳������
	float*currColor;		//��ǰ��ɫ
	unsigned int currX,currY;//��ǰ��ɫ������ֵ,���½�Ϊ0
	WFilter*filter;			//����������
	void buildColorArray();
	int float2int(float&r,float&g,float&b);		//�˺����Ѹ����͵���ɫֵת��������
};
