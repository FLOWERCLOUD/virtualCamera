#pragma once

#include "ObjReader.h"
#include "Primitive.h"
#include "Material.h"
#include "Light.h"
class WScene
{
public:
	WScene(void);
	~WScene(void);

#ifdef BLENDER_INCLUDE
	//��Render�ṹ�����ݵ��룬����blender��Ⱦ��ʼ��
	void buildScene(Render* re);
#endif // BLENDER_INCLUDE
	//�˺�����ObjReader�õ������ݵ���Scene��֮��
	void buildScene(WObjReader&reader);
	//�������
	void clearScene();
	//�˺���������������
	void drawScene(bool showNormal=false,bool fillMode=false);
	//�˺���������������������TriangleArray��Ϊ����
	//���Դ˼��TriangleArray�Ƿ��д�
	void drawByTriangleArray(bool showNormal=false,bool fillMode=false,WVector3 color=WVector3(0,0,0));
	//�˺����ؽ����������subPrimitive,������ʾÿ��SubPrimitive�������������,���ô˺���������ؽ�KD��
	void rebuildAllSubPs(unsigned int inFacesPerSubP);
	//��û������ָ�룬�Լ�������ĸ���
	void getPrimitives(WPrimitive*&iprimitives,unsigned int&nPrims);
	void getNthPrimitive(WPrimitive*&iprimitives,unsigned int nthPrim);
	//��ò���ָ�������ָ�룬�Լ����ʵĸ���
	void getMaterials(WMaterial**&imaterials,unsigned int&nMtl);
	//��õ�nthMtl������
	void getNthMaterial(WMaterial*&imaterial,unsigned int nthMtl);
	//���õ�nthMtl������
	void setNthMaterial(WMaterial*imaterial,unsigned int nthMtl);
	//���ػ����������
	unsigned int getPrimNum(){return nPrimitives;}
	//�����ӻ����������
	unsigned int getSubPrimNum(){return nSubPrimitives;}
	//���س����İ�Χ��
	WBoundingBox getBBox(){return sceneBox;}
	//�������������İ�Χ��,��ɫΪ����ɫ
	void drawSceneBBox();
	//��������ӵƹ�
	void addLight(WLight *light);
	WLight* getLightPointer(unsigned int nthLight);
	unsigned int getLightNum();
	//��ʾ��ѡ����������
	void clearSelect();
	void getTriangleArray(WTriangle*&itriangles,unsigned int&nTris);
	void getTriAccelArray(float*& array, int& nElements);

	//���º����Ѳ��ʺ͵ƹ����ݷ��ص���Ӧ����������
	//�������������GPUʹ��
	//1�����ʷ��棬����һ�����������һ����������
	//   ��������洢���ʵ����ͺ���ʼλ�ã���������������ĵڼ������أ�
	//   ��������洢���ֲ��ʵ����ԣ����������ɫ
	//2���ƹⷽ�棬����һ�����������һ����������
	//   ��������洢�ƹ�����ͺ���ʼλ�ã���������������ĵڼ������أ�
	//   ��������洢���ֵƹ�����ԣ�����ǿ�ȣ�λ��
	void getMaterialArrayFloat4Uint2(
		unsigned int*& mtlIDs,  unsigned int& nIDPixels,
		float*& mtlData,		unsigned int& nDataPixels);
	//   ��������洢���ֵƹ�����ԣ�����ǿ�ȣ�λ��
	void getLightArrayFloat4Uint2(
		unsigned int*& lightIDs,  unsigned int& nIDPixels,
		float*& lightData,		unsigned int& nDataPixels);

	//for testing
	void showTriangles(){
		for(int i = 0; i < this->nTriangles; i ++){
			printf("tri%d p1: %f, %f, %f;\n", i, this->triangles[i].point1.x, this->triangles[i].point1.y, this->triangles[i].point1.z);
			printf("tri%d p2: %f, %f, %f;\n", i, this->triangles[i].point2.x, this->triangles[i].point2.y, this->triangles[i].point2.z);
			printf("tri%d p3: %f, %f, %f;\n", i, this->triangles[i].point3.x, this->triangles[i].point3.y, this->triangles[i].point3.z);
		}
	}

	void showMaterials(){
		for(int i = 0; i < this->nMaterials; i ++){
			WVector3 tmp = this->materials[i]->getColor();
			printf("mtl%d: %f, %f, %f;\n", i, tmp.x, tmp.y, tmp.z);
		}
	}
	WBoundingBox sceneBox;					//���������İ�Χ��
private:
	vector<WLight*>lights;					//�洢�ƹ�ָ��
	WMaterial**materials;					//Materialָ������
	unsigned int nMaterials;				//Material����
	WPrimitive*primitives;					//Primitive����
	unsigned int nPrimitives;				//Primitive����
	unsigned int nSubPrimitives;			//SubPrimitive����
	

	WTriangle*triangles;					//����������
	unsigned int nTriangles;				//�����θ���
	void buildTriangleArray();				//��������������
	void clearTriangleArray();				//�������������

	void buildSceneBBox();					//����������Χ��
};
