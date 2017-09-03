#include "stdafx.h"
#include "builder.h"

int _tmain(int argc, _TCHAR* argv[])
{
	/*init camera*/
	float eye[3]  ={ -1.0,0.0,0.0};
	float eyedir[3] = {-0.1, 0 ,-2.3};
	float lenLength = 15 ;

	float pixelSize = 0.1; //0.05;
	int xWidth = 1024; //1024/4;  //1024
	int yHeight = (int)1024;
	//int yHeight = (int)1024/3*10/4;
	/*user defined wrapbox */
	//float pMin[3] = {-0.12126 , -0.015558 , -0.77265};

	//float pMax[3] = { 0.121038 , 0.802649 , 0.53108}; 
	//float pMiddle[3] = { (pMin[0] + pMax[0])/2 , (pMin[1] + pMax[1])/2 , (pMin[2] + pMax[2])/2	//0.36345 ,0.9083 ,-0.1067
	//};

	Builder* horseBuilder = new PlyBuilder( new iVirtualCamera );
	horseBuilder->setCamera( CameraPara( eye, eyedir,lenLength));
	horseBuilder->setCanvas( CanvasPara(pixelSize , xWidth ,yHeight));
	/*horseBuilder->setWrapbox( WrapBoxPara(pMin ,pMax ,pMiddle) );*/
	//int indexvec[33] = {10,11,12,13 ,14, 121 ,122,123,124,231,232,233,234,341,342,343,344,451,452,453,454,561,562,563,564,671,672,673,674,781,782,783,784};
	//int indexvec[10] = {12,13 ,14,15,16,17,18,19,20,21};
	int indexvec[3] = {0,2,3};
	horseBuilder->setReadStratege( READMODE::READ_FRAME(),"F:\\VisualcamerAndYaobing\\horsebackinter08\\backinter08\\test\\" ,"horse%.2d.obj", 28,33 );
	//horseBuilder->setReadStratege( READMODE::READ_FRAME_WITH_INDEX(),"F:\\VisualcamerAndYaobing\\horse-gallop\\test\\" ,"horse%.1d.obj" ,vector<int>( &(indexvec[0]),&(indexvec[3])) );
	//horseBuilder->setReadStratege( READMODE::READ_FRAME_WITH_INDEX(),"F:\\VisualcamerAndYaobing\\meshhorses\\meshhorses\\" ,"horse-gallop-%.2d.obj" ,vector<int>( &(indexvec[0]),&(indexvec[10])) );
	//horseBuilder->setReadStratege( READMODE::READ_FRAME_WITH_INDEX(),"F:\\VisualcamerAndYaobing\\horsebackinter08\\backinter08\\" ,"%.3d.obj" ,vector<int>( &(indexvec[0]),&(indexvec[33])) );
	//horseBuilder->setWriteStratege(FORMAT::PLY() , "F:\\VisualcamerAndYaobing\\horsebackinter08\\backinter08\\out\\" ,"horse");
	horseBuilder->setWriteStratege(FORMAT::PLY() , "F:\\VisualcamerAndYaobing\\horsebackinter08\\backinter08\\out\\" ,"horse");
	Director* horseDirector = new PlyDirector( horseBuilder);
	horseDirector->direct();

	delete horseDirector;
}