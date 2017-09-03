#pragma once
#include "surfaceintegrator.h"

class WDirectLighting :
	public WSurfaceIntegrator
{
public:
	WDirectLighting(WScene *iscene,WAccelerator*itree):
	  WSurfaceIntegrator(iscene,itree){}
	virtual ~WDirectLighting(void);

	//���㳡�����еƹ�Ĺ���
	WVector3 sampleAllLights(
		WBSDF*bsdf,WSample3D&lightSample,
		WSample2D&bsdfSample,const WVector3&ro, int* nodeInfo = NULL);

private:
	//����ѡ���ĵƹ���������ֱ�ӹ���
	//roΪ���߳��䷽��
	WVector3 computeDirectLight(
		WLight *light,WBSDF*bsdf,WSample3D&lightSample,
		WSample2D&bsdfSample,const WVector3&ro, int* nodeInfo = NULL);

	bool isVisible(WVector3 pos1,WVector3 pos2, int* beginNode = NULL);
};
