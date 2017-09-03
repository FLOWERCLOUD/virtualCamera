#pragma once
#define PER_TRIANGLE_INTERSECT
#define PER_TRIANGLE_INTERSECT_TEST
#define MBVH_SUBTREE_DEPTH				1
struct WMultiBVHLeaf
{
	// �����ṹһ��ռ176���ֽ�
	// �����������󽻵�����,һ��160�ֽ�

#if !defined(PER_TRIANGLE_INTERSECT) && !defined(PER_TRIANGLE_INTERSECT_TEST)
	__m128 nu,nv,np,
		   pu,pv,
		   e0u,e0v,e1u,e1v;
	__m128i ci;
#endif
	// ������ָ�룬һ��16�ֽ�
	WTriangle* triangle[4];
};
struct WMultiRSBVHLeaf;
struct WMultiBVHNode
{
	// �ĸ���Χ�У�һ��__m128��������ĸ���Χ��һ������������
	// ����Χ�ж�Ӧ�Ľڵ㲻���ڣ����Χ����Ϊ�Ƿ�ֵ�����Ϊ0��
	__m128 bMin[3];
	__m128 bMax[3];
	// ���ӽڵ����Ҷ�ڵ��ָ��
	union
	{
		WMultiBVHNode* child[4];
		WMultiBVHLeaf* leaf[4];
		WMultiRSBVHLeaf* rsLeaf[4];
	};
	// Ԥ�ȼ��������˳����Թ��߷����������ޣ���8�����
	unsigned char traversalOrder[8];
	// ��־λ����ʾ��Ӧ�ӽڵ�����ͣ�1ΪҶ�ڵ㣬0Ϊ�ڲ��ڵ�
	char isLeaf[4];
};

class WMultiBVH: public WSimpleBVH
{
public:
	WMultiBVH();
	~WMultiBVH();
	// WAccelerator �ṩ�Ľӿ�
	bool intersect(WRay& r,WDifferentialGeometry& DG, 
		int* endNode = NULL, int beginNode = -1);	
	bool isIntersect(WRay& r, int beginNode = -1);
	void clearTree();
	void drawTree(unsigned int nthBox=0,
		float R = 0.7, 
		float G = 0.7, 
		float B = 0.7);
	void displayNodeInfo();

protected:
	// ����SimpleBVH�Ĺ�������,�������ڹ����Ĳ�����ʽ��BVH
	void buildEnhancedTree();

	struct StackNode
	{
		WMultiBVHNode* node;
		float t;
	};
	WMultiBVHNode* multiInteriors;			// �ڲ��ڵ�����
	WMultiBVHLeaf* multiLeafs;				// Ҷ�ڵ�����
	int numInteriors;						// �ڲ��ڵ���
	int numLeafs;							// Ҷ�ڵ���


	// �����Ĳ����ڵ�
	// ithNode		��ʾ����ԭ���ĵڼ����ڵ㹹���ڵ�
	// currInterior ��ʾ��ǰ���½����ڲ��ڵ�����
	// currLeaf		��ʾ��ǰ���½���Ҷ�ڵ�����
	void buildEnhancedTreeKernel(int ithNode, 
		int& currInterior, int& currLeaf, int curDepth = 0);

	// �������нڵ�ʹ�û������������
	void buildCacheFriendlyKernel();

	// ���ýڵ��Χ�У� ithChildָ���ӽڵ����
	inline void setBBoxToNode(WMultiBVHNode& node, const WBoundingBox& bBox, int ithChild);

	// ����ԭ����Ҷ�ڵ������Ĳ�����Ҷ�ڵ�
	void makeLeaf(WMultiBVHLeaf& dstLeaf, WBVHNode& srcLeaf);

	// ���ݽڵ�����Ԥ�ȼ������ʾ��˳��ı�־λ
	// srcNode ����Ϊ�ڲ��ڵ�
	void computeTraversalOrder(WMultiBVHNode& dstNode, 
		const WBVHNode& srcParent, 
		const WBVHNode& srcLeft, 
		const WBVHNode& srcRight);

	// ����ӽڵ������εİ�Χ��
	void getBBoxFromSrcLeaf(WBVHNode& leaf, WBoundingBox& box);

	// ����������,��drawTree����
	virtual void drawTreeRecursive(WMultiBVHNode* node);

	// ���ӽڵ�Ϊ��ʱ���丸�ڵ��洢����һ����Ч�İ�Χ��
	// �����Χ����Զ������κι����ཻ
	static const WBoundingBox invalidBox;		
};