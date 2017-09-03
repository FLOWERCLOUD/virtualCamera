#pragma once
#include "surfaceintegrator.h"

class WIrradianceCacheIntegrator :
	public WSurfaceIntegrator
{
public:
	WIrradianceCacheIntegrator(
		WScene*iscene,WAccelerator*itree,
		unsigned int ioctTreeMaxDepth=10,
		unsigned int inRaysForLambert=50,
		unsigned int inRaysForSpecular=10,
		unsigned int iminSamples=10,
		unsigned int inDirectLightSamples=1,
		unsigned int imaxTracingDepth=5,
		WSampler::WSamplerType samplerType=
		WSampler::SAMPLER_RANDOM,
		float imaxNormalError=0.1,
		float imaxPlanarError=0.1,
		float idistanceErrorFactor=0.5,
		float ismoothFactor=2.0
		);

	virtual ~WIrradianceCacheIntegrator(void);

	WVector3 integrate(WRay&ray);
	//���ò�ֵ������
	void setInterpolatorParams(
		float imaxNormalError=0.1,
		float imaxPlanarError=0.1,
		float ismoothFactor=2.0,
		unsigned int iminSamples=10);

	void setOctTreeParams(unsigned int imaxDepth)
	{octTree.setMaxDepth(imaxDepth);}

	void setSampleParams(
		unsigned int inRaysForLambert=50,
		unsigned int inRaysForSpecular=10,
		unsigned int inDirectLightSamples=1,
		float idistanceErrorFactor=0.4
		);

	void pathTracingParams(unsigned int imaxDepth)
	{maxTracingDepth=imaxDepth;}

	void clearTree();
	void displaySamplePoints(bool isDisplayBox=false);
	void displayTreeNodes();

private:
	WSampler*sampler;
 	WSample2D LambertBSDFSamples;
	WSample2D SpecularBSDFSamples;
 	WSample3D lightSamples;
	WDirectLighting Dlighting;
	WPathIntegrator pathIntegrator;
	//��ֵ��
	WIrradianceInterpolator interpolator;
	WOctTree octTree;

	unsigned int nRaysForLambert;
	unsigned int nRaysForSpecular;
	unsigned int maxTracingDepth;
	unsigned int nDirectLightSamples;
	//������Χ�еĴ�С��׷�ٹ��ߵ�ƽ�����������
	//��Ϊ����ϵ��
	float distanceErrorFactor;


	//ͨ�������в������ֵ���E
	//���ز���ֵ��ʾ��ֵ�Ƿ�ɹ�
	//point��normal�ֱ��ʾҪ���в�ֵ�ĵ��λ�úͷ��߷���
	bool interpolate(
		WVector3&point,
		WVector3&normal,
		WVector3&E);

	//�����µĲ����㣬����������˲���
	//����Eֵ
	WVector3 computeNewSamples(WBSDF*bsdf);

	//�����ӹ��գ�����Lֵ
	WVector3 computeIndirectLight(WBSDF*bsdf);

	//����·�����ٰ취���㣬ֱ�������·�����
	//����·����������������ʱ��ֹͣ,����Lֵ
	void pathTracing(WRay&ray,
		WVector3&indirectLight,WVector3&pathThroughPut);


};
