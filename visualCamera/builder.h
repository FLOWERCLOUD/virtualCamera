/*
	author : huayunhe
*/
#pragma  once
#include "VirtualCamera.h"

#define PI  3.1415
const int  CAMARA_AGL_BEGIN =  0;
const int  CAMARA_AGL_END   =  4;
const int LOOP_COUNT = 1;
namespace READMODE{
	struct READ_ONCE{};
	struct READ_FRAME{};
	struct READ_FRAME_WITH_INDEX{};
}
namespace CACULATE_VIEW_MODE{
	struct SINGLE_VIEW{};
	struct MUTI_VIEW{};
}
namespace WRITEMODE{
	struct WRITE_SPERCIFIC_FILE{};
	struct WRITE_ALL{};
}
namespace FORMAT{
	struct OBJ{};
	struct PLY{};
}

/*
coordinates
			y /|\
			   |
			   |
			   |
			   |
			   |
			  / \
			/     \
		  /         \
		/             \
   z |/_               _\| x
*/

/*
 horse
 wrapbox
 ( -0.12126 , -0.015558 , -0.77265 )
               |
			  \|/
			   *1
								   ——————————
								 /|                  /|
							   /  |                /  |
							 /    |              /    |
							|——————————|*2  |
							|	  |			     |    |
							|     |              |    |
							|   *1——————— |——|
							|    /               |   /
							|   /                |  /
							|  /                 | /
							| /                  |/
							|——————————|
							                                *2
														   /|\
															|
															( 0.121038 , 0.802649 , 0.53108)
	                     
*/


/*
len 表示的是光线的发射起点到  与光线的方向 (dir) 垂直的投影面 的距离
dir 表示的光线的方向 ，与 眼睛(eye) 无关 ，可以不标准化，因为在iVirtualCamera 中会标准化
*/
class CameraPara
{
public:
	CameraPara()
	{
		//此时的内置成员是随机值
	}
	CameraPara(float* _eye ,float* _dir , float _lenlength)
	{
		for(int i = 0 ; i < 3 ;++i){
			eye[0]= _eye[0];
			eye[1]= _eye[1];
			eye[2]= _eye[2];
			dir[0]= _dir[0];
			dir[1]= _dir[1];
			dir[2]= _dir[2];
			lenlength = _lenlength;
		}
	}
	float eye[3];
	float dir[3];
	float lenlength;

};
/*
在iVirtualCamera 中 会有 2*xWidth*2*yHeight 个格子，然后pixelSize 是每个格子的大小
                     y
                   /|\
					|
					|
					|
					|
    -x <—————— ——————>x
					|
					|
					|
					|
				   \|/
				   -y
*/
class CanvasPara
{
public:
	CanvasPara()
	{
		//此时的内置成员是随机值
	}
	CanvasPara( float _pixelSize , int _xWidth ,int _yHeight)
	{
		pixelSize = _pixelSize;
		xWidth = _xWidth;
		yHeight = _yHeight;
	}
	float pixelSize ;
	int xWidth;
	int yHeight ;

};
class WrapBoxPara
{
public:
	WrapBoxPara()
	{
		for(int i = 0 ; i < 3 ;++i){
			pmin_[i] = 1000.0;
			pmax_[i] = -1000.0;
			pcenter_[i] =0;
		}
	}
	WrapBoxPara( float* _pmin , float* _pmax)
	{
		float center[3] = { (_pmin[0]+_pmax[0])/2 ,(_pmin[1]+_pmax[1])/2 ,(_pmin[2]+_pmax[2])/2 };
		init(_pmin , _pmax ,center);
	}
	WrapBoxPara( float* _pmin , float* _pmax , float* _pcenter)
	{
		init(_pmin , _pmax ,_pcenter);
	}
	void init(float* _pmin , float* _pmax , float* _pcenter)
	{
		for(int i = 0 ; i < 3 ;++i){
			pmin_[i] = _pmin[i];
			pmax_[i] = _pmax[i];
			pcenter_[i] = _pcenter[i];
		}
	}
	void setMaxWrapbox(float* _pmin , float* _pmax)
	{
		for(int i = 0 ; i < 3 ;++i){
			pmin_[i] = pmin_[i]<_pmin[i]?pmin_[i]:_pmin[i];
			pmax_[i] = pmax_[i]> _pmax[i] ? pmax_[i]:_pmax[i];
		}
		float center[3] = { (pmin_[0]+pmax_[0])/2 ,(pmin_[1]+pmax_[1])/2 ,(pmin_[2]+pmax_[2])/2 };
		init( pmin_ , pmax_ , center);
	}
	float pmin_[3];
	float pmax_[3];
	float pcenter_[3];

};

class Builder
{
	
public:
	static enum FORMAT_TYPE { OBJ ,PLY};
	static enum READ_STATEGE{ READ_ONE , READ_FRAME ,READ_FRAME_WITH_INDEX_VEC};
	Builder( iVirtualCamera* const _virCamera):virCamera_(_virCamera)
	{

		in_filedir = NULL;
		out_filedir = NULL;
		float lenLength = 15.0;
		float pixelSize = 0.1; //0.05;
		int xWidth = 256;  //1024
		int yHeight = 256;

	}
	virtual ~Builder()
	{
		delete virCamera_;
	}
	void setCamera( const CameraPara& _camera)
	{
		camera_ = _camera;

	}
	void setCanvas( const CanvasPara& _canvas )
	{
		canva_ = _canvas;

	}
	void setWrapbox( const WrapBoxPara& _wrapbox)
	{
		wrapbox_ = _wrapbox;
	}
	void setReadStratege(READMODE::READ_ONCE ,char* _fileDir , char* _fileNameWildCard)
	{
		setReadStratege(READMODE::READ_FRAME() ,_fileDir ,  _fileNameWildCard ,0 , 0);
	}
	void setReadStratege(READMODE::READ_FRAME ,char* _fileDir , char* _fileNameWildCard ,int startId , int endId)
	{
		clearInputFilenameMap();
		for(int i = startId ; i <endId ;++i)
		{
			char filetmp[250];
			char fileformat[250];
			strcpy_s(fileformat,_fileDir);
			strcat_s(fileformat , _fileNameWildCard);
			sprintf_s( filetmp , fileformat , i );
			inputFilename_map[i] = string(filetmp);

		}
	}
	void setReadStratege(READMODE::READ_FRAME_WITH_INDEX ,char* _fileDir , char* _fileNameWildCard ,const vector<int>& indexVec)
	{
		clearInputFilenameMap();
		for(auto vecitr = indexVec.begin()  ; vecitr != indexVec.end() ;++ vecitr)
		{
			char filetmp[250];
			char fileformat[250];
			strcpy_s(fileformat,_fileDir);
			strcat_s(fileformat , _fileNameWildCard);
			sprintf_s( filetmp , fileformat , *vecitr );
			inputFilename_map[ *vecitr] = string(filetmp);
		}
	}

	void setWriteStratege(FORMAT::PLY , char* _fileDir ,char* _outfileName )
	{
		strcpy_s(out_file_buf , _fileDir);
		strcpy_s(out_file_prefix ,_outfileName);
		strcpy_s( outfiledirandnameprefix , _fileDir);
		strcat_s(outfiledirandnameprefix , _outfileName);		
	}
	void setWriteName(int fileid ,const char* adddir =NULL)
	{
		clearOutputFilenameMap(); 
		stringstream ss;
		
		if(adddir)
		{
			ss<<out_file_buf<<adddir<<"\\"<<out_file_prefix;
		}else{
			ss<<outfiledirandnameprefix;
		}
		ss<<"_";
		if(fileid>999){
			ss<<"file"<<fileid;
		}
		else if( fileid > 99){
			ss<<"file0"<<fileid;
		}else if(fileid>9)
		{
			ss<<"file00"<<fileid;
		}else
		{
			ss<<"file000"<<fileid;
		}
		ss<<"_";
		ss<<"view";
		for( auto itr = res_points_position.begin() ; itr != res_points_position.end() ; ++itr)
		{
			ss<<"_";
			ss<< itr->first;
		}
	
		ss<<".ply";
		ss>>outFilename_map[0];


	}
	void setCacalateViewStratege(CACULATE_VIEW_MODE::SINGLE_VIEW , int viewagId)
	{
		clearResultMap();
		buildbyView( 0, viewagId);
	}
	void setCacalateViewStratege(CACULATE_VIEW_MODE::MUTI_VIEW ,int fileid, const vector<int>& viewagIdvec)
	{
		clearResultMap();
		for(auto bitr =  viewagIdvec.begin(); bitr!= viewagIdvec.end() ;++bitr)
		{
			buildbyView( fileid , *bitr);
		}
	}
	

	virtual void setInputFileName(int fileId  = 0){}
	virtual void setOutputFileName(int viewagid = 0 ,int fileId  = 0){}
	void initMaxWrapbox()
	{
		for( auto fileiter = inputFilename_map.begin() ; 
			fileiter !=  inputFilename_map.end() ;++fileiter)
		{
			virCamera_->beforegetVirtualPoto( inputFilename_map[fileiter->first].c_str());
			WScene& scene = virCamera_->getScene();
			WVector3 pmaxvec =scene.getBBox().pMax;
			WVector3 pminvec =scene.getBBox().pMin;
			float pMin[3] =		
			{ //-0.12126 , -0.015558 , -0.77265
				pminvec.x , pminvec.y, pminvec.z
			};
			float pMax[3] = { 
				//0.121038 , 0.802649 , 0.53108
				pmaxvec.x , pmaxvec.y,pmaxvec.z
			};
			float pMiddle[3] = { 

				//(pMin[0] + pMax[0])/2 , (pMin[1] + pMax[1])/2 , (pMin[2] + pMax[2])/2
				0.36345 ,0.9083 ,-0.1067
			};
			wrapbox_.setMaxWrapbox(pMin ,pMax);
		}
		
	}
	virtual void buildbyView(int infileId , int viewId)
	{

	}
	virtual void coutResult() const
	{

	}
	const map<int,vector<WVector3>>& getResult()
	{
		return  res_points_position;
	}
	void clearResultMap()
	{
		res_points_position.clear();
		res_points_norm_.clear();
	}
	void clearInputFilenameMap()
	{
		inputFilename_map.clear();
	}
	void clearOutputFilenameMap()
	{
		outFilename_map[0];
		outFilename_map.clear();
	}
	iVirtualCamera* const virCamera_;
	CameraPara camera_;
	CanvasPara canva_;
	WrapBoxPara wrapbox_;
	


	//float pMin[3];
	//float pMax[3];
	//float pMiddle[3];

	//

	//float eye_new[3];
	//float dir_new[3];
	//float lenLength ;

	//float pixelSize ; //0.05;
	//int xWidth ;  //1024
	//int yHeight;
	char outfiledirandnameprefix[250];

	char in_file_buf[256];
	char out_file_buf[256];
	char out_file_prefix[256];
	char* in_filedir;
	char* out_filedir;

	char* infileName_;
	char* outfileName_; 
	int viewagId_;
	int fileId_;
	map<int ,vector<WVector3>> res_points_position;
	map<int ,vector<WVector3>> res_points_norm_;
	map<int , string> inputFilename_map;
	map<int ,string > outFilename_map;
};
class PlyBuilder : public Builder
{
public:
	PlyBuilder( iVirtualCamera* const _virCamera ):Builder(_virCamera)
	{
		viewagId_ = 0;
		in_filedir = "F:\\VisualcamerAndYaobing\\meshhorses\\meshhorses\\";
		out_filedir = "F:\\VisualcamerAndYaobing\\meshhorses\\meshhorses\\out\\";
		strcpy_s( in_file_buf ,in_filedir);
		strcpy_s( out_file_buf , out_filedir);
		infileName_ = "horse-gallop-%.2d.obj";
		outfileName_ = "horse_View%.3d_file%.4d.ply";
	}

	virtual void buildbyView(int infileId , int viewId)
	{
		viewagId_ = viewId;
		virCamera_->beforegetVirtualPoto( inputFilename_map[infileId].c_str());
		//WScene& scene = virCamera_->getScene();
		//WVector3 pmaxvec =scene.getBBox().pMax;
		//WVector3 pminvec =scene.getBBox().pMin;
		float pMin[3] =		
		{ //-0.12126 , -0.015558 , -0.77265
			wrapbox_.pmin_[0] , wrapbox_.pmin_[1], wrapbox_.pmin_[2]
		};
		float pMax[3] = { 
			//0.121038 , 0.802649 , 0.53108
			wrapbox_.pmax_[0] ,wrapbox_.pmax_[1] ,wrapbox_.pmax_[2] 
		};
		float pMiddle[3] = { 

			//(pMin[0] + pMax[0])/2 , (pMin[1] + pMax[1])/2 , (pMin[2] + pMax[2])/2
			wrapbox_.pcenter_[0] ,wrapbox_.pcenter_[1] ,wrapbox_.pcenter_[2] 
		};
		float xmaxdis =  (pMax[1] - pMiddle[1])> (pMax[2] - pMiddle[2]) ? (pMax[1] - pMiddle[1]): (pMax[2] - pMiddle[2]);  //求出 y ， z 方向中物体最长的一极
		float eye_x[3] = { pMin[0] - xmaxdis , pMiddle[1] ,pMiddle[2] };  // 初始的照相机 , y ,z 与 物体包围盒的中心 y, z 相同 ， 而x 小于 包围盒的最小x ,并构成45度视锥
		float ymaxdis =  ( pMax[0] - pMiddle[0]) > (pMax[1] - pMiddle[1])? pMax[0] - pMiddle[0]:pMax[1] - pMiddle[1];
		float eye_y[3] =  { pMiddle[0] , pMiddle[1] ,(pMax[2] + ymaxdis) } ;  // 初始照相机旋转90度后 ,x ,y 与 物体包围盒的中心 x, y 相同 ， 而z 大于 包围盒的最大z
		float d_x = eye_x[0] - pMiddle[0]; 
		float d_y = 0;
		float d_z = 0;
		float radius = sqrt( d_x*d_x + d_y*d_y + d_z* d_z );
		float r_xz = sqrt( d_x*d_x  + d_z* d_z );  //1.0563	

		float a = sqrt( d_x*d_x) ; //r_xz; //椭圆的a
		float b = sqrt( (eye_y[2]- pMiddle[2]) *(eye_y[2]- pMiddle[2]) ) ; //a / sqrt((middle[0] - pMin[0])*(middle[0] - pMin[0])) * sqrt((middle[2] - pMin[2])*(middle[2] - pMin[2])); //3 ;//r_xz / (middle[0] - pMin[0]) * (middle[2] - pMin[2]); //椭圆的b
		float ag_rot =  viewagId_ * PI/6 ;
		float mat_rot[2][2] = { cos(ag_rot) , -sin(ag_rot) , sin(ag_rot),cos(ag_rot)};

		float d_x_new = d_x * mat_rot[0][0] + sqrt( a*a - d_x*d_x) * mat_rot[0][1];
		float d_z_new =sqrt( b*b - d_z*d_z)* mat_rot[1][0] + d_z * mat_rot[1][1];  

		float eye_new[3] = { d_x_new  + pMiddle[0] ,eye_x[1] , d_z_new + pMiddle[2] }; //0.3634  0.8999 0.088
		float d_y_new = d_y;

		float r_xz_new = sqrt( d_x_new*d_x_new  + d_z_new*d_z_new );  //视点到包围盒中心的距离 //1.056  3.79 6.03 6.72 5.64 3.13 1.056 3.79(7) 6.03 6.72 5.64 3.14(11)

		float dir_new[3] = { 
			(pMiddle[0] - eye_new[0])/r_xz_new , 0  ,( pMiddle[2]- eye_new[2])/r_xz_new 

		};

		
		virCamera_->getVirtualPoto(eye_new,dir_new,camera_.lenlength,in_file_buf,canva_.pixelSize,canva_.xWidth,canva_.yHeight,res_points_position[viewagId_],res_points_norm_[viewagId_]);
	}
	void setViewAgId(int _viewId)
	{
		viewagId_ = _viewId;
	}
	void setFileId( int _fileId)
	{
		fileId_ = _fileId;
	}
	void setInputFileName(int fileId  = 0)
	{
		char FilePrifix[256];
		strcpy_s (in_file_buf ,in_filedir);
		strcpy_s(FilePrifix , in_file_buf);
		strcat_s(FilePrifix ,infileName_ );
		sprintf_s( in_file_buf ,FilePrifix ,fileId);
		

	}
	void setOutputFileName(int viewagid = 0  ,int fileId  = 0)
	{
	
		char FilePrifix[256];
		strcpy_s( out_file_buf , out_filedir);
		strcpy_s(FilePrifix , out_file_buf);
		strcat_s(FilePrifix ,outfileName_ );
		sprintf_s( out_file_buf ,FilePrifix,  viewagid ,fileId);
	}

	virtual void coutResult() const
	{
		//stadardlize
		const float* pmin = wrapbox_.pmin_;  //在const函数里面成员变量是const类型 ，所以 pmin ,pmax 指针要加const修饰
		const float* pmax = wrapbox_.pmax_;
        float maxrange = -1.0;
		for( int i = 0 ; i< 3 ;++i)
		{
			if((pmax[i] - pmin[i])> maxrange)maxrange = pmax[i] - pmin[i];
		}
		std::cout<<"pmin"<< pmin[0]<<" "<<pmin[1]<<" "<<pmin[2]<<std::endl;
		std::cout<<"pmax"<< pmax[0]<<" "<<pmax[1]<<" "<<pmax[2]<<std::endl;
		std::cout<<"max range "<<maxrange;
		int vtxnum = 0;
		auto bitr = res_points_position.begin();
		auto eitr = res_points_position.end();
		for( ; bitr!= eitr ;++bitr){
			auto end =  bitr->second.end();
			for (auto v_it = bitr->second.begin(); v_it != end; v_it++)
			{
				++vtxnum;
			}
		}
		
		//注意，在const 函数中不能调用 map 的[] 方法
		const string& outfilename = (outFilename_map.begin())->second;
		std::fstream op( outfilename,std::ios_base::out); 
		if(op.is_open()){
			op<<"ply"<<std::endl;
			op<<"format ascii 1.0"<<std::endl;
			op<<"element vertex "<< vtxnum<<std::endl;
			op<<"property   float   x"<<std::endl;
			op<<"property   float   y "<<std::endl;
			op<<"property   float   z "<<std::endl;
			op<<"property   float   nx"<<std::endl;
			op<<"property   float   ny "<<std::endl;
			op<<"property   float   nz "<<std::endl;
			op<<"property   uchar red "<<std::endl;
			op<<"property   uchar   green"<<std::endl;
			op<<"property   uchar  blue"<<std::endl;
			op<<"end_header"<<std::endl;		
			bitr = res_points_position.begin();
			auto normitr = res_points_norm_.begin();
			for( ; bitr!= eitr && normitr != res_points_norm_.end();++bitr ,++normitr){

				auto end =  bitr->second.end();
				auto norm_end = normitr->second.end();
				auto v_it = bitr->second.begin();
				auto n_it = normitr->second.begin();
				for ( ; v_it != end && n_it!= norm_end; ++v_it ,++n_it)
				{
					op<<((*v_it).x -pmin[0])/maxrange<<" "<<((*v_it).y -pmin[1])/maxrange<<" "<<((*v_it).z-pmin[2])/maxrange
						<<" "<<(*n_it).x<<" "<<(*n_it).y<<" "<<(*n_it).z<<" "<<255<<" "<<0<<" "<<0			
						<<endl;
				}
			}
			op.close();
		}
		
	}
	


};

class Director
{
	public:
	Director(Builder* _builder):builder_(_builder){}
	virtual ~Director()
	{
		delete builder_;
	}
	virtual void setView(){};
	virtual void direct()
	{

	}
	
	Builder* builder_;
};
class PlyDirector: public Director
{
public:
	PlyDirector(Builder* _builder):Director(_builder)
	{

	}
	~PlyDirector()
	{
	}
	
	void direct()
	{
		/*builder_->setInputFileName( 12);
		for(int i = 0 ;i < 13 ;++i)
		{
			builder_->setOutputFileName(i ,12);
			builder_->buildbyView(i);
			builder_->coutResult();
			builder_->clearResultMap();

		}*/
		builder_->initMaxWrapbox();
		int viewrange = 8;
		for( auto fileiter = builder_->inputFilename_map.begin() ; 
			fileiter !=  builder_->inputFilename_map.end() ;++fileiter)
		{
			for( int viewId = 1 ; viewId < viewrange ;++viewId)
			{
				vector<int> viewarry;
				for(int i = 0 ; i< viewId ;++i)
				{
					viewarry.push_back( i);				
				}
				builder_->setCacalateViewStratege(CACULATE_VIEW_MODE::MUTI_VIEW() ,fileiter->first ,viewarry);
				string adddir;
				stringstream ss;
				if( viewrange>9){
					ss<<"view";
				}else{
					ss<<"view0";
				}			
				ss<< viewId-1;
				ss>>adddir;
				builder_->setWriteName(fileiter->first ,adddir.c_str());
				builder_->coutResult();

			}
		}
		
		
	}
};