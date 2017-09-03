#pragma once
#include "BSDF.h"
class WMaterial
{
public:
	enum MaterialType{
		MATERIAL_LAMBERT=0,
		MATERIAL_PHONG=1,
		MATERIAL_PERFECTREFLECTION=2,
		MATERIAL_PERFECTREFRACTION=3,
		MATERIAL_METAL=4,
		MATERIAL_DIELECTRIC=5
	};


	WMaterial(
		MaterialType itype,string iName,unsigned int iID,
		bool iisEmissive,WVector3 icolor=WVector3(1)):
	type(itype),name(iName),ID(iID),isEmissive(iisEmissive),
	color(icolor){}

	virtual ~WMaterial(void);
	//由材质创建BSDF,bsdf指针不需要预先新建对象
	//由此函数新建对象
	virtual void buildBSDF(
		WDifferentialGeometry DG,WBSDF*&bsdf)=0;

	virtual void setColor(WVector3 icolor){color=icolor;}
	WVector3 getColor(){return color;}
	virtual void getProperties(vector<float>& properties){};
	WVector3 color;
	MaterialType type;
	string name;
	unsigned int ID;
protected:
	//是否为发光材质
	bool isEmissive;
};
class WLambertMaterial:public WMaterial
{
public:
	WLambertMaterial(string iName,unsigned int iID,
		WVector3 icolor):
	WMaterial(MATERIAL_LAMBERT,iName,iID,false,icolor){}
	~WLambertMaterial(){}

	//设置颜色
//	void setColor(WVector3 icolor){color=icolor;}
	//创建一个LambertBSDF
	void buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf);
	void getProperties(vector<float>& properties);

};
class WPhongMaterial:public WLambertMaterial
{
public:
	WPhongMaterial(string iName,unsigned int iID,
		WVector3 icolor,float ispecular,float iglossiness):
	WLambertMaterial(iName,iID,icolor),
		specular(ispecular),
		glossiness(iglossiness){type=MATERIAL_PHONG;}
	void setParams(float ispecular,
		float iglossiness)
	{specular=ispecular;
	glossiness=iglossiness;}
	void setSpecular(float ispecular)
	{specular=ispecular;}
	void setGlossiness(float iglossiness)
	{glossiness=iglossiness;}
	void buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf);
protected:
	float specular,glossiness;
};
//完全光滑表面的材质
class WPerfectReflectionMaterial:public WMaterial
{
public:
	WPerfectReflectionMaterial(string iName,unsigned int iID,
		WVector3 icolor=WVector3(1)):
	  WMaterial(
		  MATERIAL_PERFECTREFLECTION,iName,iID,false,icolor)
	  {}
	  ~WPerfectReflectionMaterial(){}


//	  void setColor(WVector3 icolor){color=icolor;}
	  void buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf);
	  void getProperties(vector<float>& properties);
private:

};
//完全透明且光滑物体的材质
class WPerfectRefractionMaterial:public WMaterial
{
public:
	WPerfectRefractionMaterial(string iName,unsigned int iID,
		WVector3 icolor=WVector3(1),float iIOR=1.33):
	WMaterial(
		  MATERIAL_PERFECTREFRACTION,iName,iID,false,
		  icolor),IOR(iIOR){}
	~WPerfectRefractionMaterial(){}

	void buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf);
	void setIOR(float ior){IOR=ior;}

	void getProperties(vector<float>& properties);
private:

	float IOR;
};
class WMetalMaterial:public WMaterial
{
public:
	WMetalMaterial(string iName,unsigned int iID,WVector3 Fr,float iexp);
	~WMetalMaterial(){}
	void buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf);
	void refreshColor();
	void setGlossiness(float iglossiness)
	{exp=iglossiness;}
private:
	WVector3 k,eta;
	float exp;
};
class WDielectricMaterial:public WMaterial
{
public:
	WDielectricMaterial(string iName,unsigned int iID,
		WVector3 icolor,float iexp,float iior):
	WMaterial(WMaterial::MATERIAL_DIELECTRIC,iName,iID,
		false,icolor),
	exp(iexp),ior(iior){}
	~WDielectricMaterial(){}
	void buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf);
	void setParams(float iglossiness,float iior)
	{exp=iglossiness;iior=ior;}
	void setGlossiness(float iglossiness)
	{exp=iglossiness;}
	void setIOR(float iior)
	{ior=iior;}
private:
	float exp,ior;
};