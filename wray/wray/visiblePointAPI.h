#pragma once
#include "stdafx.h"


class VisiblePoint
{
public:
  VisiblePoint();
  ~VisiblePoint();

public:
	void readObj(char* fileName);
	bool intersect(float pnt[3], float dir[3], float t);
	bool intersect(float pnt[3], float dir[3], float t,float firstPoint[3]);
	WScene &getScene(){return myScene;}
private:
	WObjReader reader;
	WScene myScene;
	WAccelerator* myAccelerator;
};


