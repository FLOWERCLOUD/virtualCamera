// visualCamera.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "VirtualCamera.h"
//#include <pcl/io/io.h>
//#include <pcl/io/pcd_io.h>
//#include <pcl/point_types.h>

//using namespace pcl;
#define PI  3.1415
const int  CAMARA_AGL_BEGIN =  0;
const int  CAMARA_AGL_END   =  4;
const int LOOP_COUNT = 1;
const char* in_filename = "F:\\VisualcamerAndYaobing\\meshes\\mesh_%.4d.obj";  
const char* out_filename = "F:\\VisualcamerAndYaobing\\meshes\\mesh-mutiview %.3d.xyz";

//int _tmain(int argc, _TCHAR* argv[])
//{
//	for( int loop = 0;loop < LOOP_COUNT ;++loop){
//
//		/**/for (int fileId = CAMARA_AGL_BEGIN; fileId < CAMARA_AGL_END; fileId ++)
//	{
//	//int fileId = 14;
//		iVirtualCamera virCamera;
//		char in_file_buf[2048];
//		char out_file_buf[2048];
//
//		///inout file
//		//sprintf(in_file_buf,"C:\\Users\\kobe\\Desktop\\Trajectory_data\\human_dance\\%.2d.obj",fileId);//dancer girl
//		//sprintf(in_file_buf,"F:\\models\\lhnmodels\\meshes (3)\\meshes\\mesh_%.4d.obj",fileId);//hip_hop boy
//		//sprintf(in_file_buf,"C:\\Users\\kobe\\Desktop\\animation data\\tiaowu_OBJ_Seq\\tiaowu.%.4d.obj",fileId);
//		//sprintf(in_file_buf,"F:\\models\\lhnmodels\\meshes (4)\\meshes\\mesh_%.4d.obj",fileId);//mesh4_down_up
//		//sprintf(in_file_buf,"F:\\models\\modelsNa\\horse-gallop\\horse-gallop-%.2d.obj",fileId);//horse
//		//sprintf(in_file_buf,"C:\\Users\\kobe\\Desktop\\animation data\\tiaowu_OBJ_Seq\\tiaowu.%.4d.obj",fileId);//animation data
//		//sprintf(in_file_buf,"F:\\VisualcamerAndYaobing\\horse-gallop\\horse-gallop-%.2d.obj",loop);//dancer girl
//		sprintf( in_file_buf ,in_filename, loop);
//		//sprintf(in_file_buf,"C:\\Users\\kobe\\Desktop\\Trajectory_data\\Scanning virtual camera data\\horse\\interpolation\\afterInterAll\\horse%.3d.obj",fileId);//  interpolation horse
//
//
//		///output file
//		//sprintf(out_file_buf,"C:\\Users\\kobe\\Desktop\\animation data\\scanning1212_f292_307\\dancer_girl%.3d.xyz",fileId);//animation data
//		//sprintf(out_file_buf,"C:\\Users\\kobe\\Desktop\\Trajectory_data\\Scanning virtual camera data\\dancer girl\\dancer_girl%.3d.xyz",fileId);
//		//sprintf(out_file_buf,"C:\\Users\\kobe\\Desktop\\Trajectory_data\\Scanning virtual camera data\\mesh4_down\\down_up%.3d.xyz",fileId);
//		//sprintf(out_file_buf,"C:\\Users\\kobe\\Desktop\\Trajectory_data\\Scanning virtual camera data\\horse\\horse%.2d.xyz",fileId);//horse
//		//sprintf(out_file_buf,"C:\\Users\\kobe\\Desktop\\Trajectory_data\\Scanning virtual camera data\\animation data\\tiaowu%.3d.xyz",fileId);//animation data
//
//		//sprintf(out_file_buf,"C:\\Users\\kobe\\Desktop\\Trajectory_data\\Scanning virtual camera data\\animation data(201_300)\\tiaowu%.3d.xyz",fileId);//animation data
//		//sprintf(out_file_buf,"F:\\VisualcamerAndYaobing\\horse-gallop\\horse-gallop-mutiview-%.3d.xyz"
//		sprintf(out_file_buf,
//			//"F:\\VisualcamerAndYaobing\\meshes\\mesh-mutiview %.3d.xyz"
//			out_filename
//           #ifdef test
//			%d-%d-%d-%d.xyz",0,1,2
//		   #endif
//			,fileId);//dancer girl 
//		//sprintf(out_file_buf,"E:\\models\\lhnmodels\\meshes (1)\\dancer001.xyz",fileId);//dancer girl 
//
//		std::fstream op(out_file_buf,std::ios_base::out);  //std::ios_base::out std::ios::app
//
//		//float eye[3] = {0.3,0.9,4.0};           //正面
//		//float eye[3] = {4.6,0.9,-0.11};             //左侧
//		//float eye[3] = { -1.0,0.9,0.1};          //右侧
//		float eye2[3] = { 
//#ifdef _horse_
//			-1.0,0.9,0.1
//#endif
//			-0.6, 0.9 ,-0.1
//		};          //右侧
//
//		float pMin[3] = {
//#ifdef _horse_
//			-0.12126 , -0.015558 , -0.77265
//#endif
//			0.1169,0.0386 , -0.2924
//		};
//		float pMax[3] = { 
//#ifdef _horse_
//			0.12103 , 0.80264 , 0.53108
//#endif
//			0.61 ,1.778 , 0.079
//			
//		};
//		float pMiddle[3] = { 
//#ifdef _horse_
//			0 , 0.3935 ,-0.2416
//#endif
//			(pMin[0] + pMax[0])/2 , (pMin[1] + pMax[1])/2 , (pMin[2] + pMax[2])/2
//			//0.36345 ,0.9083 ,-0.1067
//		};
//		float eye_x[3] = { pMin[0] - (pMax[1] - pMiddle[1]) , pMiddle[1] ,pMiddle[2] };  // 初始的照相机 , y ,z 与 物体包围盒的中心 y, z 相同 ， 而x 小于 包围盒的最小x ,并构成45度视锥
//		float eye_y[3] =  { pMiddle[0] , pMiddle[1] ,pMax[2] + (pMax[1] - pMiddle[1])  } ;  // 初始照相机旋转90度后 ,x ,y 与 物体包围盒的中心 x, y 相同 ， 而z 大于 包围盒的最大z
//		float d_x = eye_x[0] - pMiddle[0]; 
//		float d_y = 0;
//		float d_z = 0;
//		float radius = sqrt( d_x*d_x + d_y*d_y + d_z* d_z );
//		float r_xz = sqrt( d_x*d_x  + d_z* d_z );  //1.0563	
//
//		float a = sqrt( d_x*d_x) ; //r_xz; //椭圆的a
//		float b = sqrt( (eye_y[2]- pMiddle[2]) *(eye_y[2]- pMiddle[2]) ) ; //a / sqrt((middle[0] - pMin[0])*(middle[0] - pMin[0])) * sqrt((middle[2] - pMin[2])*(middle[2] - pMin[2])); //3 ;//r_xz / (middle[0] - pMin[0]) * (middle[2] - pMin[2]); //椭圆的b
//		float ag_rot =  fileId * PI/6;
//		float mat_rot[2][2] = { cos(ag_rot) , -sin(ag_rot) , sin(ag_rot),cos(ag_rot)};
//
//		float d_x_new = d_x * mat_rot[0][0] + sqrt( a*a - d_x*d_x) * mat_rot[0][1];
//		float d_z_new =sqrt( b*b - d_z*d_z)* mat_rot[1][0] + d_z * mat_rot[1][1];  
//
//		float eye_new[3] = { d_x_new  + pMiddle[0] ,eye_x[1] ,d_z_new + pMiddle[2]}; //0.3634  0.8999 0.088
//		float d_y_new = d_y;
//
//		float r_xz_new = sqrt( d_x_new*d_x_new  + d_z_new*d_z_new );  //视点到包围盒中心的距离 //1.056  3.79 6.03 6.72 5.64 3.13 1.056 3.79(7) 6.03 6.72 5.64 3.14(11)
//		//float eye[3] = {0.3,0.9,-4.0};            //背面
//		//float eye[3] = {2.0,0.9,-0.15};
//		//float dir[3] = {-0.01,0.0,-2.30};       //正面
//		//float dir[3] = {-1.0,0.0,0.0};         //左侧
//		//float dir[3] = {1.0,0.0,0.0};    
//		//右侧
//		//float dir2[3] = {1.0,0.0,0.0}; 
//		float dir_new[3] = { 
//			//-0.1, 0 ,-2.3
//			//0 , 0 , -0.95
//			(pMiddle[0] - eye_new[0])/r_xz_new , 0  ,( pMiddle[2]- eye_new[2])/r_xz_new 
//			//(middle[0] - eye_new[0])/r_xz_new , 0  ,(middle[2]- eye_new[2])/r_xz_new 
//		};
//		//float dir[3] = {0.001,0.0,1.0};      //背面
//		//float dir[3] = {0.00001,0.0,1.0}; 
//		//float dir[3] = {-1.5,0.0,0.0};
//		//float dir[3] = {0.0,0.0,-2.0};
//		float lenLength = 15.0;
//		float pixelSize = 0.1; //0.05;
//		int xWidth = 256;  //1024
//		int yHeight = 256;
//
//
//		WScene& scene = virCamera.getScene();
//		//vector<WVector3> res_points;
//		//vector<WVector3> res_points2;
//		vector<WVector3> res_points_new;
//		//virCamera.getVirtualPoto(eye,dir,lenLength,in_file_buf,pixelSize,xWidth,yHeight,res_points);
//		//virCamera.getVirtualPoto(eye2,dir2,lenLength,in_file_buf,pixelSize,xWidth,yHeight,res_points2);
//		virCamera.getVirtualPoto(eye_new,dir_new,lenLength,in_file_buf,pixelSize,xWidth,yHeight,res_points_new);
//		//WVector3 pMiddle;
//		//pMiddle.x = 0;
//		//pMiddle.y = 0.3935;
//		//pMiddle.z = -0.2416;
//
//
//		/*pcl::PointXYZ point;
//		pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<PointXYZ>());*/
//#ifdef xyzone
//		for (auto v_it = res_point
//			    s.begin(); v_it != res_points.end(); v_it++)
//		{
//			/**/
//			if (op.is_open())
//			{
//			/*op<<(*v_it).x<<" "<<(*v_it).y<<" "<<(*v_it).z<<" "
//			<<-1<<" "<<1<<" "<<1<<" "<<50<<" "<<50<<" "<<50<<endl;*/
//				op<<(*v_it).x<<" "<<(*v_it).y<<" "<<(*v_it).z<<endl;
//			}
//
//			/*point.x=(*v_it).x;
//			point.y=(*v_it).y;
//			point.z=(*v_it).z;
//			cloud->points.push_back(point);*/
//		} 
// 
//
//		 
//		for (auto v_it = res_points2.begin(); v_it != res_points2.end(); v_it++)
//		{
//			
//			if (op.is_open())
//			{
//			
//				op<<(*v_it).x<<" "<<(*v_it).y<<" "<<(*v_it).z<<endl;
//			}
//
//		}
//#endif
//
//		for (auto v_it = res_points_new.begin(); v_it != res_points_new.end(); v_it++)
//		{
//
//			if (op.is_open())
//			{
//
//				op<<(*v_it).x<<" "<<(*v_it).y<<" "<<(*v_it).z
//				#ifdef normal_out 
//					<<" "<<0<<" "<<0<<" "<<0<<" "<<0<<" "<<0<<" "<<0
//				#endif					
//					<<endl;
//			}
//
//		}
//		/*cloud->width=(int)cloud->points.size();
//		cloud->height=1;
//		pcl::io::savePCDFileASCII("ZYL.pcd",*cloud);*/
//		//op.close();
//	}
//
// }
//
//	
//	
//	
//	return 0;
//}

