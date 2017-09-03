#pragma once
#include "stdafx.h"
#include "Scene.h"
class iVirtualCamera
{
public:
	iVirtualCamera();
	virtual ~iVirtualCamera();

public:
	virtual void readObj(char* fileName);
	virtual void readPly(char* fileName);
	virtual void beforegetVirtualPoto(const char* fileName);
	virtual void getVirtualPoto(float eye[3],float dir[3],float lenLength,char* filename,
		float pixelSize,int x_Width,int y_Heigth, vector<WVector3>& result ,vector<WVector3>& norm);

	virtual WScene& getScene(){return m_scene;}

protected:
	WObjReader m_reader;
	WScene     m_scene;
	WAccelerator* m_acceletator;
};