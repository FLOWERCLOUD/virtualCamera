#include "stdafx.h"
#include "VirtualCamera.h"

iVirtualCamera::iVirtualCamera()
{
	m_acceletator = NULL;
}

iVirtualCamera::~iVirtualCamera()
{
   m_scene.clearScene();
   m_reader.clear();
   m_acceletator = NULL;
}

void iVirtualCamera::readObj(char* fileName)
{
	m_reader.readFile(fileName);
	m_scene.buildScene(m_reader);
	m_acceletator = new WSimpleBVH();
	m_acceletator->buildTree(m_scene);
}
void iVirtualCamera::readPly(char* fileName)
{

}
// should be call before getVirtualPoto
void iVirtualCamera::beforegetVirtualPoto(const char* fileName)
{
	//build tree
	m_reader.clear();
	m_scene.clearScene();
	m_reader.readObjFile(fileName);
	m_scene.buildScene(m_reader);
	if(!m_acceletator)m_acceletator = new WSimpleBVH();
	m_acceletator->buildTree(m_scene);

}
void iVirtualCamera::getVirtualPoto(float eye[3],float dir[3],float lenLength,char* filename,
	                                float pixelSize,int x_Width,int y_Heigth, vector<WVector3>& result_position ,vector<WVector3>& reusult_norm)
{
	//readObj(filename);//build tree
	WVector3 up(0,0,1);
	WVector3 xup(1,0,0);

	WVector3 dir_n(dir[0],dir[1],dir[2]);
	dir_n.normalize();

	WVector3 x_dir;
	x_dir = dir_n.cross(up);
	if( ( abs(x_dir.x) <0.01) && ( abs(x_dir.y) < 0.01) &&  ( abs(x_dir.z) < 0.01) ){  // 避免求叉乘时出现问题
		x_dir = dir_n.cross(xup);
	}
	WVector3 y_dir = x_dir.cross(dir_n);

// 	float pixelSize = 0.1;
// 	int x_Width = 1024;
// 	int y_Heigth = 256;

	WVector3 vc(eye[0],eye[1],eye[2]);
	result_position.clear();
	reusult_norm.clear();
	for (int i = - y_Heigth; i <= y_Heigth; i++)
	{
		for (int j = - x_Width; j <= x_Width; j++)
		{
			float yp = pixelSize * i;
			float xp = pixelSize * j;
			WVector3 ray = dir_n * lenLength + xp *x_dir + yp * y_dir;
			float ray_len = ray.length();
			ray.normalize();
			WDifferentialGeometry DG;

			WRay real_ray(vc,ray,0.0,ray_len - 1e-3);

			bool isIntersect = m_acceletator->intersect(real_ray,DG);

			if (isIntersect)
			{
			    result_position.push_back(DG.position);
				reusult_norm.push_back(DG.normal);
			}

		}
	}

}