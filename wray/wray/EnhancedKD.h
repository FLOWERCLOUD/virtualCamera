#pragma once
#include "simplekd.h"
#define EKD_SUBTREE_DEPTH 2
struct WEKDInterior : public WSKDNode
{
	// ��������ڲ��ڵ�����һ��
	// etype   [H][L] --> [6 5 4 3][2 1 0 x]
	union
	{
		unsigned short etype;			// �ڵ�ָ�ƽ�����
		unsigned char  etype_c[2];			
	};
	float splitPlane[7];			// �ָ�ƽ��
	WSKDNode* child[8];				// �ӽڵ�
};

class WEnhancedKD :
	public WSimpleKD
{
public:
	WEnhancedKD(void);
	~WEnhancedKD(void);
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
	WEKDInterior* ekdInteriors;			// ��ǿ���ڲ��ڵ�����
	int numEInteriors;					// �ڲ��ڵ���

	// ����������ڲ��ڵ�
	void buildEnhancedInterior();		

	//���� ropes, ��6�������ֱ����ڵ��Χ��6��������ڽڵ�����

};
