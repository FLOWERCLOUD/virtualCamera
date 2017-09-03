#pragma once
#include "Vector3.h"
class WLight
{
public:
	enum LightType{
		LIGHT_POINT=0,
		LIGHT_RECTANGLE=1,
		LIGHT_OBJECT=2
	};
	LightType type;

	WLight(LightType itype,bool iisPoint):
	type(itype),isPoint(iisPoint){};
	virtual ~WLight(void);
	//�Ƿ���Դ
	bool isPoint;

	virtual void sampleLight(
		float u1,float u2,float u3,WBSDF&bsdf,
		WVector3&iposition,WVector3&iintensity,float&PDF)=0;
	virtual void draw()=0;
	//�����������,
	//�˺������ڰ����еƹ�����Դ����һ��float4������
	//����properties�Ĵ�Сһ��Ϊ4�ı���
	virtual void getProperties(vector<float>& properties){};
	//�����������Ĵ�С
	virtual void getPropertySize(int& nFloat4s){}
};

class WPointLight:public WLight
{
public:
	WPointLight(WVector3 iintensity,WVector3 iposition):
	  WLight(LIGHT_POINT,true),intensity(iintensity),
	  position(iposition){}
	~WPointLight(){}

	void setIntensity(WVector3 iintensity)
	{intensity=iintensity;}
	void setPosition(WVector3 iposition)
	{position=iposition;}
	void sampleLight(float u1,float u2,float u3, WBSDF&bsdf,WVector3&iposition,WVector3&iintensity,float&PDF);
	void draw();
	//����������飬�����ƹ��ǿ�ȣ��ƹ��λ��
	//���Ե�˳���ǣ�ǿ�� r g b a �� λ�� x y z w
	//a wΪ 1.0f
	virtual void getProperties(vector<float>& properties);
private :
	WVector3 intensity;
	WVector3 position;
};

class WRectangleLight:public WLight
{
public:
	WRectangleLight(WVector3 iposition,WVector3 idirection,
		WVector3 up,float width,float height,
		WVector3 iintensity,bool iisDoubleSide=true);
	void setIntensity(WVector3 iintensity)
	{intensity=iintensity;}
	void setPosition(WVector3 iposition)
	{position=iposition;}
	void sampleLight(float u1,float u2,float u3,WBSDF&bsdf, WVector3&iposition,WVector3&iintensity,float&PDF);
	void draw();
	//����������飬�����ƹ��ǿ�ȣ��ƹ��λ��
	//���Ե�˳���ǣ�ǿ�� r g b a �� λ�� x y z w
	//a wΪ 1.0f
	virtual void getProperties(vector<float>& properties);
private:
	WVector3 position;
	WVector3 direction;
	WVector3 x,y;
	WVector3 intensity;
	float area;
	bool isDoubleSide;
};
//����ƹ�
//�������ƹ�Ĳ����ǣ�
//1������һ������ƹ����
//2����Ҫ�������ӵ��ƹ��������
//3�����õƹ����ĺ������޸�Ҫ�������Ĳ���
/*
class ObjectLight:public WLight
{
public:
	ObjectLight(WVector3 iintensity,Scene*iscene,bool iisDoubleSide):WLight(LIGHT_OBJECT,false),intensity(iintensity),scene(iscene),isDoubleSide(iisDoubleSide){}
	void addPrimitive(unsigned int nthPrimitive);
	void sampleLight(
		float u1,float u2,float u3,WBSDF&bsdf,
		WVector3&iposition,WVector3&iintensity,float&PDF);
	void draw(){};
	void clear();
private:
	Scene*scene;
	WVector3<Triangle>faces;
	WVector3 intensity;
	bool isDoubleSide;
};*/
