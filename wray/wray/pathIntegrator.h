#pragma once
#include "surfaceintegrator.h"

class WPathIntegrator :
	public WSurfaceIntegrator
{
public:
	WPathIntegrator(WScene*scene,WAccelerator*tree,
		unsigned int ipathDepth=1,
		WSampler::WSamplerType samplerType=WSampler::SAMPLER_RANDOM,float imultiplier=1);
	virtual ~WPathIntegrator(void);

	//¹âÏß×·×Ùº¯Êý
	WVector3 integrate(WRay&ray);
	void setPathMaxDepth(unsigned int idepth);
	void displayTime();
private:
	WClock timer;
	WSampler*sampler;
	WSample2D BSDFSamples;
	WSample3D lightSamples;
	WDirectLighting Dlighting;
	unsigned int pathMaxDepth;
	float multiplier;
	void clearSamples();
	void allocateSamples();
	void computeSamples();
};
