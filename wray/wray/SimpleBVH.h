#pragma once
#include "stdafx.h"
//�����ǲ��ǲ���û�ж�ջ���󽻷���
#define STACKLESS_TRAVERSAL
//#define BVH_MAX_TRIANGLES_PER_LEAF = 5;
#define SBVH_SUBTREE_DEPTH	2
struct WBVHNode
{
	//ÿ��Ҷ�ڵ�������������,1 - 4 ֮��ȡֵ
	static const unsigned int maxTrianglesPerLeaf =	4;
	enum NodeType{
		BVHN_XSPLIT=0,BVHN_YSPLIT=1,BVHN_ZSPLIT=2,
		BVHN_LEAF=3};
	union
	{
		float box[6];							//��Χ��
		WTriangle* tris[maxTrianglesPerLeaf];	//����������
	};
	NodeType type;								//�ڵ�����
	union
	{
		unsigned int farNode;					//�ҽڵ�
		unsigned int nTriangles;				//�����θ���
	};

};
//ѹ�����BVH�ڵ㣬��������ת�浽�Դ�
struct WBVHCompressedNode
{
	//ÿ��Ҷ�ڵ�������������
	static const unsigned int maxTrianglesPerLeaf
		=WBVHNode::maxTrianglesPerLeaf;
	union
	{
		float box[6];							//��Χ��
		WTriangle* tris[maxTrianglesPerLeaf];	//����������
	};
	//onHit��ʾ���Χ���ཻʱת���Ľڵ�����
	//���λͬʱ�洢�ڵ����ͣ�
	//Ϊ0ʱ����ʾΪ�ڲ��ڵ㣬����ΪҶ�ڵ�
	//�����Ľڵ����� = onHit & 0x7FFFFFFF
	//�ڵ����� = type & 0x80000000
	union
	{
		int onHit;	    //�����ڲ��ڵ㣬��ʾ����ʱת���Ľڵ�����
		int type;		//�ڵ�����
		int nTris;		//����Ҷ�ڵ㣬��ʾ����������
	};
	union
	{
		int next;		//����Ҷ�ڵ㣬��ʾ��һ��Ӧ�÷��ʵĽڵ�
		int onMiss;		//�����ڲ��ڵ㣬��ʾ������ʱת���Ľڵ�����
	};
};
struct WBin
{
	WBoundingBox box;
	unsigned int nTriangles;
	WBin(){nTriangles=0;}
};


class WSimpleBVH:public WAccelerator
{
public:
	WSimpleBVH(void);
	~WSimpleBVH(void);
	// WAccelerator �ṩ�Ľӿ�
	void buildTree(WScene&scene);
	void clearTree();
	void drawTreeIteractively();
	void drawTree(unsigned int nthBox=0,
				  float R = 0.7, 
				  float G = 0.7, 
				  float B = 0.7);//�ݹ鷽�������ڼ��ڵ�
	bool intersect(WRay&r,WDifferentialGeometry&DG, int* endNode, int beginNode = 0);//�󽻺���
	bool isIntersect(WRay&r, int beginNode = 0);
	void displayNodeInfo();

	// WSimpleBVH ���еĽӿ�
	virtual void displayEnhancedNodeInfo();

protected:
	//����Ϊ�������������ڹ���BVH��

	//ÿ���ڵ㹹����ʱ����Ȱ������ΰ�������λ��
	//�ŵ���Ӧ��С�����棬С������������������������ȣ�
	//��������������/binRatio,�ɼ���ֵԽС������Խ�ߣ�
	//���ǹ���ʱ��ҲԽ��
	static const unsigned int binRatio = 4;
	//��������C=traversalCost+isectCost*(nL*SL+nR*SR)/SP;
	//traversalCost
	//isectCost
	//nL, nRΪ���ҽڵ������θ���
	//SL, SR��SPΪ���ҽڵ㼰���ڵ��Χ�����
	//�������ڵ�ÿ�ڵ���������С�ڸ���������ֹ���֣�
	//�ʺ������Խ�һ������ΪC=nL*SL+nR*SR
	//static const float traversalCost = 1.0f;
	//static const float isectCost = 10.0f;
protected:
	WTriangle* triangles;				//��������������
	unsigned int nTriangles;			//�������ܸ���
	WBoundingBox sceneBox;

	vector<WBVHNode>nodes;				//�ڵ�����
	WBVHCompressedNode* compressedNodes;	//ѹ���Ľڵ�����
	unsigned int nodeMaxDepth;			//��¼BVH�ﵽ��������

	//BVH���ں�
	//triangleIDs�������ε�����
	//box�ǽڵ��Ӧ�İ�Χ��
	//�����ֺ����һ�߽ڵ�������Ϊ0�����ʱ
	//�����ֲ�̬���������������������ڶ���ص�����������ɵ�
	//��ʱֱ���½�һ����֮����һ���������ε�Ҷ�ڵ�
	//��һ����Ҷ�ڵ�
	virtual void buildTreeKernel(
		vector<unsigned int>& triangleIDs,
		WBoundingBox&box ,unsigned int depth=0,bool isIll=false);
	
	//�������е�BVH������ѹ����BVH��
	virtual void buildEnhancedTree();

	virtual void drawTreeRecursivelyKernel(vector<WBVHNode>&nodes,
									unsigned int ithNode);

	//�����������������
	WTriangle&getTriangle(unsigned int nthTriangle)
	{return triangles[nthTriangle];}

	//��ýڵ�İ�Χ��
	void getBBoxFromInterior(WBoundingBox& box, WBVHNode& node)
	{
		box.pMin.x = node.box[0];
		box.pMin.y = node.box[1];
		box.pMin.z = node.box[2];
		box.pMax.x = node.box[3];
		box.pMax.y = node.box[4];
		box.pMax.z = node.box[5];
	}
	void getBBoxFromCompressedNode(
		WBoundingBox& box, WBVHCompressedNode& node)
	{
		box.pMin.x = node.box[0];
		box.pMin.y = node.box[1];
		box.pMin.z = node.box[2];
		box.pMax.x = node.box[3];
		box.pMax.y = node.box[4];
		box.pMax.z = node.box[5];
	}



	//����BVHCompressedNode�Ĳ���
	//����Ƿ�Ϊ�ڲ��ڵ�
	inline bool isInterior(WBVHCompressedNode& node)
	{
		return (node.type & 0x80000000) == 0;
	}
	inline int getOnHitID(WBVHCompressedNode& node)
	{
		return node.onHit & 0x7fffffff;
	}
	inline void setInterior(WBVHCompressedNode& node, int onHitID)
	{
		//��type���λ��0
		node.onHit = onHitID;
		node.type &= 0x7fffffff;
		//onMiss���Կ���ֱ�Ӹ�ֵ���ʲ��ں��������
	}
	inline void setLeaf(WBVHCompressedNode& node, int nTris)
	{
		node.nTris = nTris;
		//��type���λ��1
		node.type |= 0x80000000;
	}
	inline int getNTris(WBVHCompressedNode& node)
	{
		return node.nTris & 0x7fffffff;
	}

};
