#include "cameraadapter.h"

CameraAllViewAdpater::CameraAllViewAdpater(iVirtualCamera& icamera):ref_camera_(icamera){}

CameraAllViewAdpater::~CameraAllViewAdpater()
{
	m_scene.clearScene();
	m_reader.clear();
	m_acceletator = NULL;
}

void CameraAllViewAdpater::readObj(char* fileName)
{
	m_acceletator = NULL;
}

void CameraAllViewAdpater::readPly(char* fileName)
{

}

void CameraAllViewAdpater::beforegetVirtualPoto(const char* fileName)
{
	//build tree
	m_reader.clear();
	m_scene.clearScene();
	m_reader.readObjFile(fileName);
	m_scene.buildScene(m_reader);
	if(!m_acceletator)m_acceletator = new WSimpleBVH();
	m_acceletator->buildTree(m_scene);

}

void CameraAllViewAdpater::getVirtualPoto(float eye[3],float dir[3],float lenLength,char* filename, float pixelSize,int x_Width,int y_Heigth, vector<WVector3>& result_position,vector<WVector3>& reusult_norm)
{
	
}

