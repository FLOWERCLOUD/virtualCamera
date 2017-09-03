#pragma once

class WRecursivePathIntegrator:public WSurfaceIntegrator
{
public:
	WRecursivePathIntegrator(WScene*scene,WAccelerator*tree,
		unsigned int ipathDepth=1,unsigned int inBranch=2,
		WSampler::WSamplerType samplerType=WSampler::SAMPLER_RANDOM,float imultiplier=1);
	virtual ~WRecursivePathIntegrator(void);

	//����׷�ٺ���
	WVector3 integrate(WRay&ray);
	void setPathMaxDepth(unsigned int idepth);
	void displayTime();
private:
	WClock timer;
	WSampler*sampler;
	WSample2D BSDFSamples;
	WSample3D lightSamples;
	//����ֱ�ӹ��յĶ���
	WDirectLighting Dlighting;
	//�������
	unsigned int maxDepth;
	//��֧��
	unsigned int nBranchs;
	float multiplier;
	void clearSamples();
	void allocateSamples();
	void computeSamples();
	//����׷�ٵݹ麯��
	WVector3 integrateCore(WRay ray,unsigned int depth,int beginNode = 0);
};
