#pragma once
#include "stdafx.h"
#include <atlstr.h>
#include "Primitive.h"
//#include "DisplaySystem.h"
#define OBJ_READ_CHECK_COUNTER 300000
#ifndef FILEREADINGPROCESSMSG
#define FILEREADINGPROCESSMSG WM_USER+1
#endif
struct Wfloat3
{
	Wfloat3(){x=y=z=0;}
	float x,y,z;
};
struct Wfloat2
{
	float u,v;
};
struct WObjTriangle
{
	unsigned int vertIndex[3];			//vertex index--->��������Ƭ�������������Ӧ������ֵ
	unsigned int texcoordIndex[3];		//texture coordinate index--->��������Ƭ������������Ĳ��������Ӧ������ֵ
	unsigned int norIndex[3];			//normal index--->����ͬ�ϡ���
	unsigned int mtlIndex;				//material index
};
struct WObjPrimitive   //--->����һ������
{
	string name;				//primitive name---->���������
	vector<Wfloat3>vertices;	//vertex coordinate---->���嶥��ļ���
	vector<Wfloat2>texcoords;	//texture coordinate---->�����������ļ���
	vector<Wfloat3>normals;		//normal coordinate---->���嶥�㷨�����ļ���
	vector<WObjTriangle>faces;	//faces---->��������Ƭ��ļ���
};
struct WObjMaterial
{
	string name;				//name of material
	Wfloat3 diffuse;			//��������ɫ
	Wfloat3 emission;			//�Է�����ɫ
	Wfloat3 transparency;		//͸����
	float   specular;			//�߹�ǿ��
	bool   isTransparent(){return transparency.x < 1 || transparency.y < 1 || transparency.z < 1;}
	bool   isShiny(){return specular > 0;}
};

class WObjReader
{
public:
	static enum coordType{VERTCOORDS,TEXCOORDS,NORCOORDS};
	static enum indexType{VERTINDICES,TEXINDICES,NORINDICES};
	WObjReader();//{}
	~WObjReader();
	vector<WObjPrimitive>primitives;  //---->����������ļ���
	vector<WObjMaterial>Materials;   //---->�����Ӧ���ʵļ��ϣ������ƺ�ֻ�Ǽ򵥵����������
	//��ȡobj�Լ�������mtl�ļ�
	//obj��mtl�ļ�������ͬһĿ¼
	void readFile(CString fileName);
	//ֻ��ȡobj�ļ�
	void readObjFile(const char*fileName);
	//ֻ��ȡmtl�ļ�
	void readMtlFile(const char*fileName);
	//return vertex buffer
	bool getWireArray(int primitive,WGLWireArray*vB);
	void showObj();
	void showMtl();
	void clear();
	void WObjReader::fillPrimitive(unsigned int nthPrimitive,WPrimitive&pri);
	int getReadingProcess(){return readingProcess;}

/*
	void setThread(CWinThread*thread)
	{readingThread=thread;}*/


//protected:
private:
	//��vector���͵���������ת����float��������
	//pArrayΪ����ָ�룬nFloatsΪ����Ԫ�ظ���
	//type��3��ȡֵ��OBJ_VERTICES,OBJ_TEXCOORDS,OBJ_NORMALS
	//��ʾ���ض�Ӧ���͵�����
	//nthPrimitiveָ��������һ�����������
	void getCoordArray(float*&pArray,unsigned int&nFloats,
		unsigned nthPrimitive,coordType type);
	//��vector���͵���������ת����unsigned int��������
	//������getCoordArray���ƣ�nInt����������Ϊ����*3
	//ע������ֵ��3�����Ƕ�Ӧ���㣬������������±�
	//ע������ֵ��2�����Ƕ�Ӧ��ͼ����������±�
	void getIndexArray(unsigned int*&pArray,unsigned int &nInts,
		unsigned nthPrimitive,indexType type);
	ifstream file;
	float readingProcess;

//	CWinThread*readingThread;
};
