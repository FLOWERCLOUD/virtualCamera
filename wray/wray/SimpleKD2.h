#pragma once
//#define SKD_RECOMM_LEAF_TRIS			3
#define SKD_MAX_DEPTH					70
#define SKD_MIN_BOX_FACTOR				5e-4
//#define SKD_SUBTREE_DEPTH				3

#define SKD_INTERIOR_MASK				0x80000000
#define SKD_LEAF_MASK					0x40000000
//Ҷ�ӽڵ����������������
struct WSKDNode2
{
	enum NodeType {KDN_XSPLIT=0,KDN_YSPLIT=1,KDN_ZSPLIT=2,KDN_LEAF=3};
	unsigned char type;
};
//�ڲ��ڵ�
struct WSKDInterior2 : public WSKDNode2
{
	float splitPlane;
	WSKDNode2* child[2];
};

//Ҷ�ڵ�
struct WSKDLeaf2 : public WSKDNode2
{
//	WBoundingBox box;
	//�ֱ�Ϊ pMin.x pMin.y pMin.z pMax.x pMax.y pMax.z
	float box[2][3];
	//�����ڽڵ������,�ֱ�Ϊ 
	//x ���棬 x���棬 y ���棬 y���棬 z ���棬 z���� 
	WSKDNode2* ropes[2][3];
	//WTriangle** triangle;
	int nTriangles;
	TriAccel* triInfo;
};

class WSimpleKD2:public WAccelerator
{
	//Bounding Edge ���ڰ������ΰ��հ�Χ�зֳ�����
protected:
	struct WBoundingEdge
	{
		enum EdgeType{BE_END = 0,BE_PLANAR = 1, BE_START = 2};
		enum EdgeAxis{BE_X = 0,BE_Y = 1,BE_Z = 2};
		//BE�����ͣ�
		//�ֱ�Ϊ�����ε���ʼ�㣬��ֹ��
		WBoundingEdge(WBoundingEdge::EdgeType t, EdgeAxis a, float pos, int triID):type(t), axis(a), t(pos), triangleID(triID){}
		WBoundingEdge(){}

		char type;
		char axis;
		float t;
		int   triangleID;

		//������������
		//����ȷ��BE����tֵ����
		//��ͬt��BE����BE�����ͺ�������ID�Ĺ�ϵ����
		bool operator<(const WBoundingEdge&e)const;
	};

public:
	WSimpleKD2(void);
	~WSimpleKD2(void);
	void buildTree(WScene&scene);
	void clearTree();

	bool intersect(WRay& r,WDifferentialGeometry& DG, 
		int* endNode = NULL, int beginNode = -1);
	bool intersect4(WRay r[4], WDifferentialGeometry DG[4], int endNode[4] = NULL, int beginNode[4] = NULL);

	bool isIntersect(WRay& r, int beginNode = -1);

	void drawTree(unsigned int nthBox=0,
					float R = 0.7, 
					float G = 0.7, 
					float B = 0.7);
	void displayNodeInfo();
	void drawTriangles();
protected:
	WTriangle* triangles;				// ��������������
	unsigned int totalTriangles;		// �������ܸ���
	float sceneBox[2][3];				// ������Χ��

	int treeDepth;						// ��¼KD���ﵽ��������

	WSKDInterior2* kdInteriors;			// �ڲ��ڵ�����
	WSKDLeaf2*     kdLeafs;				// Ҷ�ڵ�����
	WTriangle** leafTriangles;			// Ҷ�ڵ��������ָ������
	int numInteriors;					// �ڲ��ڵ���
	int numLeafBytes;						// Ҷ�ڵ���
	int numLeafTriangles;				// ��ǰ����

	int numInteriorsAllocated;
	int numLeafBytesAllocated;
	int numLeafTrianglesAllocated;

	float KT;							// ����һ���ڵ�Ŀ���
	float KI;							// ��һ���������󽻵Ŀ���

	float minBoxSize[3];				// ��С��Ҷ�ڵ��С�����ڵݹ齨������ֹ����
	float minBoxFactor;					// ��СҶ�ڵ��С�볡����Χ�еı���

	static const char otherAxis[3][2];

	vector<WBoundingBox> nodeBoxes;		// �ڵ��Χ������

	//���� ropes, ��6�������ֱ����ڵ��Χ��6��������ڽڵ�����
	void buildBasicRopes(WSKDNode2* ithNode, 
		WSKDNode2* ropeX_P, WSKDNode2* ropeX_N,
		WSKDNode2* ropeY_P, WSKDNode2* ropeY_N,
		WSKDNode2* ropeZ_P, WSKDNode2* ropeZ_N);

	//������ǿ�� ropes
	void buildExtendedRopes();
	//�ж� refBox ��ָ�����Ƿ���ȫ�� testBox ��һ����
	bool isOverlap(WBoundingBox& refBox,
				   WBoundingBox& testBox,
				   int refFace);

	void buildTreeKernel( vector<WBoundingEdge>&edges, 
		int nTris,
		int depth, 
		float nodeBox[2][3],
		char triangleMap[],
		int parentIndex,
		char branch);

	void drawTreeRecursive( WSKDNode2* node, const WBoundingBox& box );

	//����һ�����Χ���ཻ�Ĺ��ߣ���������ߴӰ�Χ���ĸ������뿪
	int computeExitFace( WSKDLeaf2& node, const WRay& r, WVector3& exitPoint, float& farT);

	void buildCacheFriendlyNode();

	// ���������Ӧ�Ŀռ�
	WSKDInterior2* requestInterior();
	WSKDLeaf2* requestLeaf(int nTri);
	WTriangle** requestLeafTriangles(int nTri);
};
