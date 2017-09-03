#pragma once
#include "simplekd.h"
#define DKD_SUBTREE_DEPTH 3
struct WDKDInterior : public WSKDNode
{
	// ��2����ڲ��ڵ�����һ��
	char dtype[3];					// ����
	float splitPlane[3];			// �ָ�ƽ��
	WSKDNode* child[4];				// �ӽڵ�
};

class WDualKD :public WSimpleKD
{
public:
	WDualKD(void);
	~WDualKD(void);
	void buildTree(WScene&scene);
	void clearTree();
	
	bool intersect(WRay& r,WDifferentialGeometry& DG, 
		int* endNode = NULL, int beginNode = -1);
	bool isIntersect(WRay& r, int beginNode = -1);

	void drawTree(unsigned int nthBox=0,
		float R = 0.7, 
		float G = 0.7, 
		float B = 0.7);
	void displayNodeInfo();
	void drawTriangles();

private:
	WDKDInterior* ekdInteriors;			// ��ǿ���ڲ��ڵ�����
	int numEInteriors;					// �ڲ��ڵ���

	// ����������ڲ��ڵ�
	void buildEnhancedInterior();		

	//���� ropes, ��6�������ֱ����ڵ��Χ��6��������ڽڵ�����

};
