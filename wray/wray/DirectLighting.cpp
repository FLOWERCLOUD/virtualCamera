#include "StdAfx.h"
#include "DirectLighting.h"



WDirectLighting::~WDirectLighting(void)
{
}

bool WDirectLighting::isVisible(WVector3 pos1, WVector3 pos2, int* beginNode)
{
	WVector3 delta=pos2-pos1;
	float length=delta.length();
	delta.normalize();
	//ע�⣺����tMin�����������1e-1����Ӱ����Ӱ������
	//ֵ̫С��ʱ�����ڼ�����
	//��ģ�ͱ�������һЩ�ߵ㣬
	//̫���ʱ���ʹ����Ӱ��׼ȷ
	WRay r(pos1,delta,1e-2f,length - 1e-2f);
	int begNode = beginNode ? *beginNode : -1;
	return !tree->isIntersect(r, begNode);
}
WVector3 WDirectLighting::computeDirectLight(WLight *light, WBSDF *bsdf, WSample3D &lightSample, WSample2D &bsdfSample,const WVector3&ro, int* nodeInfo)
{
	float LSu,LSv,LSw;
	lightSample.get3D(LSu,LSv,LSw);
	float BSu,BSv;
	bsdfSample.get2D(BSu,BSv);
	float LPDF;//�Եƹ�������ܶȺ���
	WVector3 lightPosition,intensity;
//	lightPosition.showCoords();
	light->sampleLight(LSu,LSv,LSw,*bsdf,lightPosition,intensity,LPDF);
//	lightPosition.showCoords();
	bool visibilityTest=isVisible(bsdf->DG.position,lightPosition, nodeInfo);
	if(!visibilityTest)
		return WVector3(0);
	WVector3 ri=lightPosition-bsdf->DG.position;
	ri.normalize();
	WVector3 fCos=bsdf->evaluateFCos(ri,ro);
//	fCos.showCoords();
	WVector3 Ld=fCos*intensity/LPDF;

//	Ld.showCoords();
	//���ﻹ�ж�bsdf�����Ĳ���δʵ��

	return Ld;
}
WVector3 WDirectLighting::sampleAllLights(WBSDF *bsdf, WSample3D &lightSample, WSample2D &bsdfSample, const WVector3 &ro, int* nodeInfo)
{	
	WLight*pLight;
	WVector3 color(0);
	unsigned int lightNum=scene->getLightNum();
//	cout<<lightNum<<endl;
	for(unsigned int nthLight=0;nthLight<lightNum;nthLight++)
	{
		pLight=scene->getLightPointer(nthLight);
//		cout<<nthLight<<endl;
		color+=computeDirectLight(pLight,bsdf,lightSample,bsdfSample,ro, nodeInfo);
//		color.showCoords();
	}
//	color.showCoords();
	return color;
}
