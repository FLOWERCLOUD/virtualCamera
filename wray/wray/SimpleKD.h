#pragma once
#define SKD_RECOMM_LEAF_TRIS			4
#define SKD_MAX_DEPTH					70
#define SKD_MIN_BOX_FACTOR				5e-4
#define SKD_SUBTREE_DEPTH				1
#define SKD_EXTENDED_ROPES
#define SKD_MAILBOX

#define SKD_INTERIOR_MASK				0x80000000
#define SKD_LEAF_MASK					0x40000000
//Ҷ�ӽڵ����������������
struct WSKDNode
{
	enum NodeType {KDN_XSPLIT=0,KDN_YSPLIT=1,KDN_ZSPLIT=2,KDN_LEAF=3};
	unsigned char type;
};
//�ڲ��ڵ�
struct WSKDInterior : public WSKDNode
{
	float splitPlane;
	WSKDNode* child[2];
};

//Ҷ�ڵ�
struct WSKDLeaf : public WSKDNode
{
//	WBoundingBox box;
	//�ֱ�Ϊ pMin.x pMin.y pMin.z pMax.x pMax.y pMax.z
	float box[2][3];
	//�����ڽڵ������,�ֱ�Ϊ 
	//x ���棬 x���棬 y ���棬 y���棬 z ���棬 z���� 
	WSKDNode* ropes[2][3];
	WTriangle** triangle;
	int nTriangles;
};
struct WSKDNodeGPU
{
	union{
		float splitPlane;	// �ָ�ƽ��
		unsigned int   nodeBodyID;	// Ҷ�ڵ���Ϣ��box��rope������
	};

	// farNode�ĸ�λ����Ϊ
	// 31     30 | 29 ------ 0
	// �ڵ�����     �ҽڵ���������ڵ�����Ϊԭ����+1��
	// �ڵ�������WSKDNode::type��˼һ��
	union{
		unsigned int   farNode;
		int   triID;		// ������������ʼλ��
	};		
};

struct WSKDLeafBodyGPU
{
	float box[6];			// ��Χ��
	int   rope[6];			// ����������NULLʱ�� -1
};

class WSimpleKD:public WAccelerator
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
	WSimpleKD(void);
	~WSimpleKD(void);
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

	// ��ô浽�Կ����������
	void getGPUArray( 
		WSKDNodeGPU*& kdNodesGPU, 
		WSKDLeafBodyGPU*& kdLeafBodysGPU, 
		int*& triArrayGPU, int& numNodesGPU, 
		int& numLeafBodysGPU, int& numTriArrayGPU );

protected:
	WTriangle* triangles;				// ��������������
	unsigned int totalTriangles;		// �������ܸ���
	float sceneBox[2][3];				// ������Χ��

	int treeDepth;						// ��¼KD���ﵽ��������

	WSKDInterior* kdInteriors;			// �ڲ��ڵ�����
	WSKDLeaf*     kdLeafs;				// Ҷ�ڵ�����
	WTriangle** leafTriangles;			// Ҷ�ڵ��������ָ������

	int numInteriors;					// �ڲ��ڵ���
	int numLeafs;						// Ҷ�ڵ���
	int numLeafTriangles;				// ��ǰҶ�ڵ�������ָ������

	int numInteriorsAllocated;
	int numLeafsAllocated;
	int numLeafTrianglesAllocated;

	float KT;							// ����һ���ڵ�Ŀ���
	float KI;							// ��һ���������󽻵Ŀ���

	float minBoxSize[3];				// ��С��Ҷ�ڵ��С�����ڵݹ齨������ֹ����
	float minBoxFactor;					// ��СҶ�ڵ��С�볡����Χ�еı���

	static const char otherAxis[3][2];

	vector<WBoundingBox> nodeBoxes;		// �ڵ��Χ������

	//���� ropes, ��6�������ֱ����ڵ��Χ��6��������ڽڵ�����
	void buildBasicRopes(WSKDNode* ithNode, 
		WSKDNode* ropeX_P, WSKDNode* ropeX_N,
		WSKDNode* ropeY_P, WSKDNode* ropeY_N,
		WSKDNode* ropeZ_P, WSKDNode* ropeZ_N);

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

	void drawTreeRecursive( WSKDNode* node, const WBoundingBox& box );

	//����һ�����Χ���ཻ�Ĺ��ߣ���������ߴӰ�Χ���ĸ������뿪
	int computeExitFace( WSKDLeaf& node, const WRay& r, WVector3& exitPoint, float& farT);

	void buildCacheFriendlyNode();

	// ���������Ӧ�Ŀռ�
	WSKDInterior* requestInterior();
	WSKDLeaf* requestLeaf();
	WTriangle** requestLeafTriangles(int nTri);
};
