#include "StdAfx.h"
#include "Material.h"



WMaterial::~WMaterial(void)
{
}

void WLambertMaterial::buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf)
{
	bsdf=new WLambertBSDF(DG,color);
}

void WLambertMaterial::getProperties( vector<float>& properties )
{
	properties.clear();
	properties.push_back(color.x);
	properties.push_back(color.y);
	properties.push_back(color.z);
	properties.push_back(1.0f);
}

void WPhongMaterial::buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf)
{
	bsdf=new WPhongBSDF(DG,color,specular,glossiness);
}
void WPerfectReflectionMaterial::buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf)
{
	bsdf=new WPerfectReflectionBSDF(DG,color);
}

void WPerfectReflectionMaterial::getProperties( vector<float>& properties )
{
	properties.clear();
	properties.push_back(color.x);
	properties.push_back(color.y);
	properties.push_back(color.z);
	properties.push_back(1.0f);
}

void WPerfectRefractionMaterial::buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf)
{
	bsdf=new WPerfectRefractionBSDF(DG,color,IOR);
}

void WPerfectRefractionMaterial::getProperties( vector<float>& properties )
{
	properties.clear();
	properties.push_back(color.x);
	properties.push_back(color.y);
	properties.push_back(color.z);
	properties.push_back(IOR);
}

WMetalMaterial::WMetalMaterial(string iName,unsigned int iID,WVector3 Fr,float iexp):WMaterial(MATERIAL_METAL,iName,iID,false,Fr),exp(iexp)
{
	k=2.0f*(color/(WVector3(1)-color)).sqrtElement();
	WVector3 colorSqrt=color.sqrtElement();
	eta=(WVector3(1)+colorSqrt)/(WVector3(1)-colorSqrt);
// 	k.showCoords();
// 	eta.showCoords();
}
void WMetalMaterial::refreshColor()
{
	k=2.0f*(color/(WVector3(1)-color)).sqrtElement();
	WVector3 colorSqrt=color.sqrtElement();
	eta=(WVector3(1)+colorSqrt)/(WVector3(1)-colorSqrt);
}
void WMetalMaterial::buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf)
{
	bsdf=new WMetalBSDF(DG,eta,k,exp);
}
void WDielectricMaterial::buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf)
{
	bsdf=new WDielectricBSDF(DG,ior,color,exp);
}
