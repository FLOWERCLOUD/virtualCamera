#pragma once
#include "MailBox.h"
#include "Scene.h"
#include "Clock.h"
#include "Accelerator.h"
//KD���ڵ�Ľṹ
struct WKDNode
{
	enum NodeType{KDN_XSPLIT=0,KDN_YSPLIT=1,KDN_ZSPLIT=2,KDN_LEAF=3};
	NodeType type;
	union
	{
		float splitPos;				//�ڲ��ڵ㣺�ָ������λ��
		unsigned int nMailBoxes;				//Ҷ�ڵ㣺mailBox�ĸ���
	};
	union
	{
		int aboveChild;				//�ڲ��ڵ㣺һ���ӽڵ��ڽڵ������λ��
		int*mailBoxIndices;			//Ҷ�ڵ㣺mailBox������±�
	};
};
struct WKDToDo
{
	WKDNode*pNode;
	float tMin,tMax;
};

class WKDTree:public WAccelerator
{
	struct BoundingEdge
	{
		float t;
		unsigned int mailBoxNum;
		enum EdgeType{BE_START=0,BE_END=1};
		EdgeType type;
		friend bool operator<(const BoundingEdge&e1,const BoundingEdge&e2){return e1.t<e2.t;}
	};
public:
	WKDTree(void);
	virtual ~WKDTree(void);

	void cleanMailBoxes();			//��ÿ��mailBox��rayID��0
	
	//����mailbox��ӦSubPrimitive�İ�Χ��,����ʱ��
	void drawMailBoxes(
		float R=0.4,
		float G=0.4,
		float B=0.4);			
	void buildTree(WScene&scene);	//�ɳ������ݴ���KD��
	//��ʾ�ڵ��Χ��
	//�˺���Ϊ�ݹ���ã�
	//�������˼��ڵ���Ϣ�Ƿ���ȷ
	void drawTree(unsigned int nthBox=0,
		float R=0.7,
		float G=0.7,
		float B=0.7);	//�ѽڵ㻭����
	//��ʾ��������
	//�˺���Ϊ�ݹ���ã�
	//�������˼��ڵ���Ϣ�Ƿ���ȷ
	void drawScene(unsigned int nthBox=0,bool isFill=true);
	
	//��ʾ�ڵ����Ϣ,����ʱ�ã�
	void displayNodeInfo();
	
	//�󽻺�����������ߣ����DifferentialGeometry
	//���ز���ֵ��ʾ�󽻳ɹ����
	bool intersect(WRay&r,WDifferentialGeometry&DG, 
		int& endNode, int beginNode = 0);
	void clearTree();//����ڴ�ռ�
	//�������Ƿ��ڲ�����Χ�ڸ�������ĳ���������ཻ
	bool isIntersect(WRay&r, int beginNode = 0);

	//���ýڵ��������ԣ�Ӧ���ڽ���KD��֮ǰ������Щ����
	void setNodeAttr(
		unsigned int imaxDepth=10,
		unsigned int imaxSubPsPerNode=3,
		float iisectCost=1.0f,
		float itraversalCost=10.0f
		);
	void displayTime();
protected:

	//mailbox�ĸ�����Ҳ���ǳ���SubPrimitive������
	unsigned int nMailBoxes;		
	
	//�Ѿ������˿ռ��KDNode�����������ָvector��Ԫ�ظ���
	//������mailBoxes�����Ԫ�ظ���
	unsigned int nAllocatedNodes;	

	unsigned int nUsedNodes;		//�Ѿ���������ݵ�KDNode����		
	unsigned int currRayID;			//��ǰ����ID�ţ�ÿ��һ�Σ�ID������
	unsigned int maxDepth;			//KD��������
	unsigned int maxSubPsPerNode;	//ÿ���ڵ������SubPrimitive���������Ҳ����mailBox���������

	float traversalCost;			//����ȷ�����Ż��ֵĲ���
	float isectCost;				//Ҳ������ȷ�����Ż��ֵĲ���

	float process;					//��¼����kd���Ľ���
	
	WKDNode*nodes;
	WMailBox*mailBoxes;
	WBoundingBox sceneBox;			//������Χ��
	
	//�����ڵ�İ�Χ�У�ÿ���ڵ��Ӧһ����Χ�У�����ʱ��
	WBoundingBox*nodeBoxes;
	WClock clock;
	
	void buildMailBoxes(WScene&scene);//ͨ����������mailBox����

	//���ѽ����õ�mailbox����KD��,������������ΪKD�������
	//Ϊ0�Ľڵ�
	//boxes�ǵ�ǰ�ڵ������mailbox���飬nBoxes������Ԫ�ظ���
	//bBox�ǵ�ǰ�ڵ�İ�Χ��,���ڵ�İ�Χ�о��ǳ�����Χ��
	//���ô˺���֮ǰҪ����
	void buildTreeCore(
		unsigned int depth,
		const vector<int>&mBoxIndices,
		const WBoundingBox&bBox,
		vector<WKDNode>&vNodes,
		vector<WBoundingBox>&vBoxes
		);

	//����Ҷ�ڵ�
	void makeLeaf(unsigned int nMBoxes,
		const vector<int>&mailBoxIndices,
		vector<WKDNode>&vNodes);
	//���¶�ȡKD���Ľ���
	inline void updateProcess(const float &depth);
};
