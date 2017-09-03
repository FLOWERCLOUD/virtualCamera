#pragma  once
#include "VirtualCamera.h"

class CameraAllViewAdpater :public iVirtualCamera
{
public:
		CameraAllViewAdpater( iVirtualCamera& icamera);
		~CameraAllViewAdpater();
		void readObj(char* fileName);
		void readPly(char* fileName);
		void beforegetVirtualPoto(const char* fileName);
		void getVirtualPoto(float eye[3],float dir[3],float lenLength,char* filename,
			float pixelSize,int x_Width,int y_Heigth, vector<WVector3>& result_position,vector<WVector3>& reusult_norm);

 
public:
		iVirtualCamera& ref_camera_;
}