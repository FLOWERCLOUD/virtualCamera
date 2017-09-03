#include "StdAfx.h"
#include"visiblePointAPI.h"

VisiblePoint::VisiblePoint()
{

}
VisiblePoint::~VisiblePoint()
{
	myScene.clearScene();
	reader.clear();
	myAccelerator = NULL;
}
void VisiblePoint::readObj(char* fileName) 
{
	reader.readFile(fileName);
	myScene.buildScene(reader);
	//myAccelerator = new WMultiBVH();
	myAccelerator = new WSimpleBVH();
	myAccelerator->buildTree(myScene);
}

bool VisiblePoint::intersect(float pnt[3], float dir[3], float t)
{
	WVector3 start(pnt[0],pnt[1],pnt[2]); 
	WVector3 end(dir[0],dir[1],dir[2]);
	WVector3 dr = end - start;

	float len = dr.length();
	dr.normalize();

	WRay r(start,dr,0.0,len - 1e-3);
	return myAccelerator->isIntersect(r);
}
