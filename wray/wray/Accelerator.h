#pragma once

class WAccelerator
{
public:
	WAccelerator(void);
	virtual ~WAccelerator(void);
	//�󽻺�����������ߣ����ر�ʾ�ཻ�������Ե�DG
	//beginNodeΪ��ʼ����ʱ���ڵĽڵ�
	//endNodeΪ�󽻳ɹ�ʱ�Ľڵ㣬
	//������������SimpleKD��ר�õģ����ڼӿ����ٶ�
	//KDTree SimpleBVH ������Ҫʹ������������
	virtual bool intersect(WRay& r,WDifferentialGeometry& DG, 
						int* endNode = NULL, int beginNode = -1)=0;	
	virtual bool isIntersect(WRay& r, int beginNode = 0) = 0;
	virtual void buildTree(WScene& scene) = 0;
	virtual void clearTree() = 0;
	virtual void drawTree(unsigned int nthBox=0,
							float R = 0.7, 
							float G = 0.7, 
							float B = 0.7) = 0;
	virtual void displayNodeInfo() = 0;
	virtual void resetStatistics();
	virtual void displayIsectStatistics();
	// ����16�ֽڱ߽������ͷ�����
	static void* allocAligned16(const unsigned int bytes);
	static void  freeAligned16(void* ptr);
protected:
	// ����Ϊͳ�������ݵı���
	int numIntersectTest;					// ���󽻲�����
	int numIntersect;						// ������

	int numTraverseInterior_IT;				// �ڲ��ڵ�����������󽻲��ԣ�
	int numTraverseLeaf_IT;					// Ҷ�ڵ�����������󽻲��ԣ�
	int numTriangleIsect_IT;				// ������������(�󽻲���)

	int numTraverseInterior_I;				// �ڲ��ڵ�����������󽻣�
	int numTraverseLeaf_I;					// Ҷ�ڵ�����������󽻣�
	int numTriangleIsect_I;					// ������������(��)

};
