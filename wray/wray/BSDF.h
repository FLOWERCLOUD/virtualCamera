#pragma once
#include "DifferentialGeometry.h"
#include "MC.h"

class WBSDF
{
public:
	enum BSDFType{
		BSDF_LAMBERT=0,
		BSDF_PHONG=1,
		BSDF_PERFECTREFLECTION=2,
		BSDF_PERFECTREFRACTION=3,
		BSDF_METAL=4,
		BSDF_DIELECTRIC=5
	};

	WBSDF(WDifferentialGeometry& iDG,BSDFType itype,
		bool iisEmissive=false):
	DG(iDG),type(itype),isEmissive(iisEmissive){};
	virtual ~WBSDF(void);

	//��BSDF��ֵ,˳���������������
	//ri ro�����ȵ�λ��
	virtual WVector3 evaluateFCos(WVector3&ri,const WVector3&ro)=0;
	//�������������u vֵ��������Ĺ���
	//pdfΪ��Ӧλ�õĸ����ܶ�
	//sampleWi��Ҫ����Ĳ�������
	virtual void sampleRay(
		float u,float v,WVector3&sampleWi,WVector3&wo,float&pdf)=0;
	virtual WVector3 rho(WVector3&wo){return WVector3(0);}

	//����get����
	BSDFType getType(){return type;}
	WVector3 getPosition(){return DG.position;}
	WVector3 getNormal(){return DG.normal;}
	WVector3 getBitangent(){return DG.bitangent;}
	WVector3 getTangent(){return DG.tangent;}
	WVector2 getTexCoord(){return DG.texCoord;}

	//����set����
	//����DG
	void setDifferentialGeometry(
		WDifferentialGeometry&iDG);
	unsigned int getMtlID(){return DG.mtlId;}	
	BSDFType type;
	WDifferentialGeometry DG;
	bool isEmissive;
private:


};

class WLambertBSDF:public WBSDF
{
public:
	WLambertBSDF(WDifferentialGeometry& iDG,WVector3 &icolor):
	  WBSDF(iDG,BSDF_LAMBERT),color(icolor){}
	~WLambertBSDF(){}

	//���ѡ��һ�����߷���
	//���ߵķ���������ҷֲ�
	void sampleRay(
		float u,float v,
		WVector3&sampleWi,WVector3&wo,float&pdf);
	//��ΪBSDF��ֵ����
	//������ɫֵ/PI
	//��Ϊ���������غ㣬Ӧ��Integrate[F*cos(thetaI)*dwi]<=1
	//֤��������BSDF�Ǳ�ʾ����һ�����һ����������ʱ��
	//���������ķ����ķֲ������
	//����˵�������м��������������dwi������ڣ����������
	//��һ���ض�����BSDF����
	//�Ǽ��������������������Ϲ��׵�����ռ���������ı�ֵ
	//F=dLo/dEi=dLo/(Li*cos(thetaI)*dwi)
	//���룬����ʽ�ӻ�ΪIntegrate[dLo/Li]
	//��һ����������Ĺ��߱������յؾ��ȷ��䵽��������ʱ��
	//��������Ϊ1
	//����Ϊ����Lambert���ʣ�������߱����ȵ�ɢ�䵽��������
	//��FΪһ��������������Ϊ1,
	//������Ĺ��߱������յؾ��ȷ��䵽�������򣬼�Ϊ����ɫʱ
	//FΪ1/PI
	//���������գ�����ɫ��Ϊ����ʱ��ֻ���ڴ˻����ϳ���
	//��ɫ�Ĺ����������ɫΪ��1,1,1����
	WVector3 evaluateFCos(WVector3&ri,const WVector3&ro);
	WVector3 rho(WVector3&wo){return color;}
protected:
	WVector3 color;
};

class WPhongBSDF:public WLambertBSDF
{
public:
	WPhongBSDF(WDifferentialGeometry& iDG,WVector3& icolor,
		float ispecular,float iglossiness):
	WLambertBSDF(iDG,icolor),specular(ispecular),glossiness(iglossiness)
	{type=BSDF_PHONG;}
	WVector3 evaluateFCos(WVector3&ri,const WVector3&ro);
private:
	float specular,glossiness;
};


//���BSDFģ����ȫ�⻬����ķ���
class WPerfectReflectionBSDF:public WBSDF
{
public:
	WPerfectReflectionBSDF(WDifferentialGeometry& iDG,
		WVector3& icolor):WBSDF(iDG,WBSDF::BSDF_PERFECTREFLECTION),color(icolor){}
	~WPerfectReflectionBSDF(){};
	void sampleRay(
		float u,float v,
		WVector3&sampleWi,WVector3&wo,
		float&pdf);

	//ע�⣬����Ϊ��ȫ�⻬���棬������߷����ϸ���ѭ���䶨��
	//����������BSDF�Ķ��壬BSDFΪ
	//��������ļ����������
	//��һ���ض����䷽���Ϲ��׵�����ռ���������ı�ֵ
	//�ʶ�����ȫ�⻬���棬bsdf��Ϊһ������������һ��delta����
	//���Բ�����MC�������ƣ�������ͨ��ֱ�Ӽ����������÷����
	WVector3 evaluateFCos(WVector3&ri,const WVector3&ro);
private:
	WVector3 color;
};
//���BSDFģ����ȫ�⻬���������
class WPerfectRefractionBSDF:public WBSDF
{
public:
	WPerfectRefractionBSDF(WDifferentialGeometry& iDG,
		WVector3& icolor,float iIOR):WBSDF(iDG,WBSDF::BSDF_PERFECTREFRACTION),color(icolor),IOR(iIOR){}
	~WPerfectRefractionBSDF(){}
	void sampleRay(
		float u,float v,
		WVector3&sampleWi,WVector3&wo,
		float&pdf);

	WVector3 evaluateFCos(WVector3&ri,const WVector3&ro);
private:
	WVector3 color;
	float IOR;

	//����������߷����Ѱ���ȫ����Ч��
	WVector3 refractionDirection(
		WVector3&i,const WVector3&normal);
};

//�����࣬���ڼ��������ϵ��
//�������Ծ�Ե��
class WFresnelDielectric
{
public:
	WFresnelDielectric(float iIOR,WVector3 inormal);

	//����������߷����Ѱ���ȫ����Ч��
	WVector3 refractionDirection(
		WVector3&i,const WVector3&normal);
	float evaluateF(WVector3&wi);
	void setNormal(WVector3&inormal){normal=inormal;}
private:
	//����rֵ ��etaTOΪ������
	float computeR(float cosWi,float cosWt,float etaTO);
	//IORΪ�����ʣ�IOR=����������/����������
	float IOR;
	WVector3 normal;
};
//�������Ե���
class WFresnelConductor
{
public:
	WFresnelConductor(WVector3 ieta,WVector3 ik,WVector3 inormal);
	WVector3 evaluateF(WVector3&wi);
	void setNormal(WVector3&inormal){normal=inormal;}
private:
	WVector3 eta,k,normal;

};

//ģ���������
class WMetalBSDF:public WBSDF
{
public:
	WMetalBSDF(WDifferentialGeometry iDG,
		WVector3 ieta,WVector3 ik,float iexp):
	  WBSDF(iDG,BSDF_METAL),
		  fresnel(ieta,ik,iDG.normal),
		  exp(iexp){}
	  ~WMetalBSDF(){}
	  void sampleRay(
		  float u,float v,
		  WVector3&sampleWi,WVector3&wo,
		  float&pdf);

	  WVector3 evaluateFCos(WVector3&ri,const WVector3&ro);
private:
	float computeD(const WVector3&wh);
	float computeG(const WVector3&wi,const WVector3&wo,const WVector3&wh);
	float computePDF(WVector3&wi,WVector3&wo);
	WFresnelConductor fresnel;
	float exp;
};

//��BSDF��ʾ��͸����Ե���BSDF
class WDielectricBSDF:public WBSDF
{
public:
	WDielectricBSDF(WDifferentialGeometry iDG,float iior,
		WVector3 icolor,float iexp):
	WBSDF(iDG,BSDF_DIELECTRIC),
		fresnel(iior,iDG.normal),
		color(icolor),exp(iexp){}
	~WDielectricBSDF(){}
	void sampleRay(
		float u,float v,
		WVector3&sampleWi,WVector3&wo,
		float&pdf);

	WVector3 evaluateFCos(WVector3&ri,const WVector3&ro);
private:
	float computeD(const WVector3&wh);
	float computeG(const WVector3&wi,const WVector3&wo,const WVector3&wh);
	float computePDF(WVector3&wi,WVector3&wo);
	WFresnelDielectric fresnel;
	float IOR;
	float exp;
	WVector3 color;
};
