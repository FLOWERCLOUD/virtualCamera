#pragma once

class WSurfaceIntegrator
{
public:
	WSurfaceIntegrator(WScene*iscene,WAccelerator*itree):
	  scene(iscene),tree(itree){};
	virtual ~WSurfaceIntegrator(void);
	virtual WVector3 integrate(WRay&ray){return WVector3(0);}
	virtual void displayTime(){}
protected:
	WScene*scene;
	WAccelerator*tree;
};
