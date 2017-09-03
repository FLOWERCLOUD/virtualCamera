#include "StdAfx.h"
#include "EnhancedKD.h"

WEnhancedKD::WEnhancedKD(void)
{
}

WEnhancedKD::~WEnhancedKD(void)
{
	this->clearTree();
}

void WEnhancedKD::buildTree( WScene&scene )
{
	//��ó�������������Ͱ�Χ��
	cout<<"begin to build KD"<<endl;
	scene.getTriangleArray(triangles, totalTriangles);
	cout<<"total triangles: "<<totalTriangles<<endl;
	WBoundingBox sBox = scene.getBBox();

	// ���event
	vector<WBoundingEdge> edgeList;
	edgeList.reserve(totalTriangles * 3 * 2);
	for (int ithTri = 0; ithTri < (int)totalTriangles; ++ithTri)
	{
		// ���������εİ�Χ��
		float triBox[2][3];
		triBox [0][0] = triBox[1][0] = triangles[ithTri].point1.x;
		triBox [0][1] = triBox[1][1] = triangles[ithTri].point1.y;
		triBox [0][2] = triBox[1][2] = triangles[ithTri].point1.z;

		triBox [0][0] = minf(triBox [0][0], triangles[ithTri].point2.x);
		triBox [0][1] = minf(triBox [0][1], triangles[ithTri].point2.y);
		triBox [0][2] = minf(triBox [0][2], triangles[ithTri].point2.z);
		triBox [1][0] = maxf(triBox [1][0], triangles[ithTri].point2.x);
		triBox [1][1] = maxf(triBox [1][1], triangles[ithTri].point2.y);
		triBox [1][2] = maxf(triBox [1][2], triangles[ithTri].point2.z);

		triBox [0][0] = minf(triBox [0][0], triangles[ithTri].point3.x);
		triBox [0][1] = minf(triBox [0][1], triangles[ithTri].point3.y);
		triBox [0][2] = minf(triBox [0][2], triangles[ithTri].point3.z);
		triBox [1][0] = maxf(triBox [1][0], triangles[ithTri].point3.x);
		triBox [1][1] = maxf(triBox [1][1], triangles[ithTri].point3.y);
		triBox [1][2] = maxf(triBox [1][2], triangles[ithTri].point3.z);

		// ���ݰ�Χ������BoundingEdge,������edgeList
		for (int axis = 0; axis <3; ++axis)
		{
			if (triBox [0][axis] == triBox[1][axis])
				edgeList.push_back(WBoundingEdge(WBoundingEdge::BE_PLANAR,WBoundingEdge::EdgeAxis(axis),triBox[0][axis], ithTri));
			else
			{
				edgeList.push_back(WBoundingEdge(WBoundingEdge::BE_START,WBoundingEdge::EdgeAxis(axis),triBox[0][axis], ithTri));
				edgeList.push_back(WBoundingEdge(WBoundingEdge::BE_END,WBoundingEdge::EdgeAxis(axis),triBox[1][axis], ithTri));
			}
		}
	}
	// ��boundingEdge����
	sort(edgeList.begin(), edgeList.end());

	// �½����ڴ�Žڵ���ڴ�
	int approNodes = max(totalTriangles, 5000) * 8;
	kdInteriors = new WSKDInterior[approNodes];	// �ڲ��ڵ�����
	kdLeafs     = new WSKDLeaf[approNodes];		// Ҷ�ڵ�����
	leafTriangles = new WTriangle*[approNodes];	// Ҷ�ڵ�������ָ������
	numInteriorsAllocated = numLeafsAllocated = numLeafTrianglesAllocated = approNodes;

	// �ݹ鹹��KD��
	for (int i = 0; i < 3; i++)
	{
		sceneBox[0][i] = sBox.pMin.v[i];
		sceneBox[1][i] = sBox.pMax.v[i];
	}
	minBoxSize[0] = (sceneBox[1][0] - sceneBox[0][0]) * minBoxFactor;
	minBoxSize[1] = (sceneBox[1][1] - sceneBox[0][1]) * minBoxFactor;
	minBoxSize[2] = (sceneBox[1][2] - sceneBox[0][2]) * minBoxFactor;

	char* triangleMap = new char[totalTriangles];
	// ���������п��ܻ����·���ռ䣬Ϊ��ʹ��ָ����ض�λ������̫����
	// ���������У�ָ��ʵ���ϴ�Ŷ�Ӧλ�õ�ƫ����
	// ����ټ���������ʼ��ַ���ó�ʵ�ʵĵ�ַ
	buildTreeKernel(edgeList, totalTriangles, 0, sceneBox, triangleMap,0,0);
	delete[] triangleMap;
	WSKDInterior* pInteriorEnd = kdInteriors + numInteriors;
	for (WSKDInterior* pInterior = kdInteriors; pInterior < pInteriorEnd; ++pInterior)
	{
		unsigned *child = (unsigned*)&pInterior->child[0];
		unsigned isInterior = *child & 0x80000000;
		*child &= 0x3fffffff;
		*child += isInterior ? (int)kdInteriors : (int)kdLeafs;	
		child++;
		isInterior = *child & 0x80000000;
		*child &= 0x3fffffff;
		*child += isInterior ? (int)kdInteriors : (int)kdLeafs;	
	}

	WSKDLeaf* pLeafEnd = kdLeafs + numLeafs;
	for (WSKDLeaf* pLeaf = kdLeafs; pLeaf < pLeafEnd; ++pLeaf)
	{
		if (!pLeaf->nTriangles)
			continue;
		unsigned* tri = (unsigned*)&(pLeaf->triangle);
		*tri += (unsigned)leafTriangles;
	}
	//buildCacheFriendlyNode();
	buildBasicRopes(kdInteriors, NULL, NULL, NULL, NULL, NULL, NULL);
	buildExtendedRopes();

	buildEnhancedInterior();
}

void WEnhancedKD::buildEnhancedInterior()
{
	struct StackElement
	{
		WSKDInterior* pNode;
		int layer;
	};
	ekdInteriors = (WEKDInterior*)allocAligned16(sizeof(WEKDInterior) * numInteriors);
	numEInteriors = 0;
	map<WSKDNode*, WSKDNode*> interiorMap;	// ��ԭ�����ڲ��ڵ��ַ�����ڲ��ڵ��ӳ��
	deque<WSKDInterior*> stack0;
	stack0.push_back(kdInteriors);
	WEKDInterior* dstNode = ekdInteriors;

	while (stack0.size())
	{
		deque<StackElement>stack1;
		StackElement t;
		t.pNode = stack0.back();					// ���ڵ�����ջ
		t.layer = 0;								// ��¼�ڵ����
		stack1.push_back(t);
		stack0.pop_back();						

		while (stack1.size())
		{
			// pop and copy node
			StackElement top = stack1.front();
			WSKDInterior* srcNode = top.pNode;
			int layer = top.layer;
			stack1.pop_front();

			// ����0��ڵ�
			dstNode = &ekdInteriors[numEInteriors++];
			int currID = dstNode - ekdInteriors;
			dstNode->etype = srcNode->type << 2;
			dstNode->type = 0;
			dstNode->splitPlane[0] = srcNode->splitPlane;
			interiorMap[srcNode] = (WSKDNode*)currID;			// �ѵ�0��ڵ��ַ�Ķ�Ӧ��ϵ��ӵ�ӳ��
			for (char l1 = 0; l1 < 2; ++l1)
			{
				char shift = l1 * 2 + 4;
				// ����1��ڵ�
				if (srcNode->child[l1]->type != WSKDNode::KDN_LEAF)
				{	// ��1��ڵ�Ϊ�ڲ��ڵ�
					WSKDInterior* l1Node = (WSKDInterior*)srcNode->child[l1];
					interiorMap[l1Node] = (WSKDNode*)(currID | ((1 + l1) << 29));	// �ѵ�1��ڵ��ַ�Ķ�Ӧ��ϵ��ӵ�ӳ��
					dstNode->etype |= l1Node->type << shift;
					dstNode->splitPlane[l1 + 1] = l1Node->splitPlane;
					// ����2��ڵ�
					for (char l2 = 0; l2 < 2; ++l2)
					{
						char l2ID = l1 * 2 + l2;			// l2ID  = 0 1 2 3
						char l2ID2 = l2ID << 1;				// l2ID2 = 0 2 4 6
						shift = l2ID2 + 8;					// shift = 8 10 12 14
						if (l1Node->child[l2]->type != WSKDNode::KDN_LEAF)
						{	// �ڶ���Ϊ�ڲ��ڵ�,ֱ�Ӹ��Ƶ�Ŀ��ڵ�Ķ�Ӧ����
							WSKDInterior* l2Node = (WSKDInterior*)l1Node->child[l2];
							
							// �ѵ�2��ڵ��ַ�Ķ�Ӧ��ϵ��ӵ�ӳ��
							interiorMap[l2Node] = (WSKDNode*)(currID | (l2ID + 3) << 29);	
							dstNode->etype |= (l2Node->type << shift);
							dstNode->splitPlane[3 + l2ID] = l2Node->splitPlane;

							for (char l3 = 0; l3 < 2; ++l3)
							{	// ���ָ������ڲ��ڵ㣬��һ������������ֱ��ָ��Ҷ�ڵ�
								if (l2Node->child[l3]->type != WSKDNode::KDN_LEAF)
								{
									dstNode->child[l2ID2 + l3] = l2Node->child[l3];
									if (layer < EKD_SUBTREE_DEPTH)
									{
										top.pNode = (WSKDInterior*)l2Node->child[l3];
										top.layer = layer + 1;
										stack1.push_back(top);
									}
									else
									{	// �������²�Ľڵ�
										stack0.push_back((WSKDInterior*)l2Node->child[l3]);
									}
								}
								else
								{
									dstNode->child[l2ID2 + l3] = l2Node->child[l3];
								}
							}
						}
						else
						{	// �ڶ���ΪҶ�ڵ�,��Ŀ��ڵ�Ķ�Ӧ�������ɵ�һ��ڵ������
							dstNode->etype |= (l1Node->type << shift);
							dstNode->splitPlane[3 + l2ID] = l1Node->splitPlane;
							dstNode->child[l2ID2] = dstNode->child[l2ID2 + 1] = l1Node->child[l2];
						}
					}
				}
				else
				{	// ��һ��ڵ�ΪҶ�ڵ�,��Ŀ��ڵ�Ķ�Ӧ�������ɵ�0��ڵ������
					dstNode->etype |= (srcNode->type << shift);
					char l2type = (srcNode->type | (srcNode->type << 2)) << (l1 * 4 + 8);
					dstNode->etype |= l2type;
					dstNode->splitPlane[l1 + 1] = dstNode->splitPlane[3 + l1 * 2] = 
						dstNode->splitPlane[3 + l1 * 2 + 1] = srcNode->splitPlane;
					char l1m4 = l1 << 2;
					dstNode->child[l1m4] = dstNode->child[l1m4 + 1] = dstNode->child[l1m4 + 2] = dstNode->child[l1m4 + 3] = srcNode->child[l1];
				}
			}
		}
	}

	// �滻Ҷ�ڵ�ָ��
	for (WSKDLeaf* pLeaf = kdLeafs; pLeaf < kdLeafs + numLeafs; ++pLeaf)
	{
		for (WSKDNode** rope = &(pLeaf->ropes[0][0]); rope < &(pLeaf->ropes[0][0]) + 6; ++rope)
		{
			map<WSKDNode*, WSKDNode*>::iterator p = interiorMap.find(*rope);
			if (p != interiorMap.end())		// ��ʱropeָ���ڲ��ڵ�
				*rope = p->second;
			else 
			{
				if(*rope)					// ��ʱropeָ��Ҷ�ڵ�
					*rope = (WSKDNode*)(((WSKDLeaf*)*rope - kdLeafs) | (0x7 << 29));
				else						// ��ʱropeΪ��
					*rope = (WSKDNode*)0xffffffff;
			}
		}
	}
	for (WEKDInterior* pInterior = ekdInteriors; pInterior < ekdInteriors + numEInteriors; ++pInterior)
	{
		for (char i = 0; i < 8; ++i)
		{
			map<WSKDNode*, WSKDNode*>::iterator p = interiorMap.find(pInterior->child[i]);
			if (p != interiorMap.end())		// ��ʱָ���ڲ��ڵ�
				pInterior->child[i] = ((int)p->second & 0x1fffffff) + ekdInteriors;
		}
	}
}

void WEnhancedKD::clearTree()
{
	WSimpleKD::clearTree();
	freeAligned16(ekdInteriors);
	ekdInteriors = NULL;
	numEInteriors = 0;
}

bool WEnhancedKD::intersect( WRay& r,WDifferentialGeometry& DG, int* endNode /*= NULL*/, int beginNode /*= -1*/ )

{
	++numIntersect;
	// ��ʾ����߽Ͻ���������, 0 ��ʾ����ֵС�棬 1��ʾ����ֵ����
	const char nearFace[3] = {r.direction.x < 0.0f, r.direction.y < 0.0f, r.direction.z < 0.0f};

	// ������ߵ�������Χ�еĽ����Լ�������exitFace
	const float invD[3] = {1.0f / r.direction.x, 1.0f / r.direction.y, 1.0f / r.direction.z};
	float tIn  =            (sceneBox[nearFace[0]][0] - r.point.x) * invD[0];
	tIn        = maxf(tIn,  (sceneBox[nearFace[1]][1] - r.point.y) * invD[1]);
	tIn        = maxf(tIn,  (sceneBox[nearFace[2]][2] - r.point.z) * invD[2]);
	float tOut  =             (sceneBox[!nearFace[0]][0] - r.point.x) * invD[0];
	tOut        = minf(tOut,  (sceneBox[!nearFace[1]][1] - r.point.y) * invD[1]);
	tOut        = minf(tOut,  (sceneBox[!nearFace[2]][2] - r.point.z) * invD[2]);
	if (tIn > tOut || tOut < 0.0f || numInteriors == 0)
		return false;

	__m128 mailBox[4];
	mailBox[0] = _mm_castsi128_ps(_mm_set1_epi32(0));
	mailBox[1] = _mm_castsi128_ps(_mm_set1_epi32(0));
	WTriangle** pMainBox = (WTriangle**)&mailBox[0];
	char curMailBox = 0;

	tIn = maxf(tIn, 0.0f);
	float pIn[3] = {r.point.x + r.direction.x * tIn,
		r.point.y + r.direction.y * tIn,
		r.point.z + r.direction.z * tIn};
	float tBest = FLT_MAX;
	WSKDNode* pNode = ekdInteriors;
	WTriangle* bestTriangle = NULL;
	if (beginNode != -1)
		pNode = kdLeafs + beginNode;
	char entryLayer = 0;
	while(pNode)
	{
		while (pNode->type != WSKDNode::KDN_LEAF)
		{	// �ڲ��ڵ�
			++numTraverseInterior_I;
			WEKDInterior* pInterior = (WEKDInterior*)pNode;
			
			const unsigned char axis0 = ((pInterior->etype) >> 2) & 0x3;
			float t = pIn[axis0];
			
			char l1 = t > pInterior->splitPlane[0];
			if (t == pInterior->splitPlane[0])
				l1 = nearFace[axis0];

			const unsigned char axis1 = (pInterior->etype >> (4 + l1 * 2)) & 0x3;
			t = pIn[axis1];
			
			char l2 = t > pInterior->splitPlane[1 + l1];
			if (t == pInterior->splitPlane[1 + l1])
				l2 = nearFace[axis1];

			const unsigned char l1l2 = l1 * 2 + l2;
			const unsigned char axis2 = (pInterior->etype >> (8 + l1l2 * 2)) & 0x3;
			t = pIn[axis2];
			
			char l3 = t > pInterior->splitPlane[3 + l1l2];
			if (t == pInterior->splitPlane[3 + l1l2])
				l3 = nearFace[axis2];
			pNode = pInterior->child[l1l2 * 2 + l3];
		}
		++numTraverseLeaf_I;
		WSKDLeaf* pLeaf = (WSKDLeaf*)pNode;
		WTriangle** tris = pLeaf->triangle;
		int nTris = pLeaf->nTriangles;
		while(nTris--)
		{
			WTriangle* pTri = *tris;
#ifdef SKD_MAILBOX
			__m128 pTri4 = _mm_set_ps1(*reinterpret_cast<float*>(&pTri));
			__m128 tmp14  = _mm_cmpeq_ps(pTri4, mailBox[0]);
			__m128 tmp24  = _mm_cmpeq_ps(pTri4, mailBox[1]);
			tmp14 = _mm_or_ps(tmp14, tmp24);
			if (!_mm_movemask_ps(tmp14))
			{
#endif
				++numTriangleIsect_I;
				float tIsect =  pTri->intersectTest(r);
				if (tIsect < tBest)
				{
					tBest = tIsect;
					bestTriangle = *tris;
				}
#ifdef SKD_MAILBOX
			}
			pMainBox[curMailBox] = pTri;
			curMailBox = (++curMailBox) & 0x7;
#endif
			tris++;
		}
		char exitFace = 0;
		tOut = (pLeaf->box[!nearFace[0]][0] - r.point.x) * invD[0];
		float tTemp = (pLeaf->box[!nearFace[1]][1] - r.point.y) * invD[1];
		if (tTemp < tOut){
			tOut = tTemp;	exitFace = 1;
		}
		tTemp = (pLeaf->box[!nearFace[2]][2] - r.point.z) * invD[2];
		if (tTemp < tOut){
			tOut = tTemp;	exitFace = 2;
		}
		if (tBest <= tOut)
			break;					// �ҵ�����
		pIn[exitFace] = pLeaf->box[!nearFace[exitFace]][exitFace];
		char other = otherAxis[exitFace][0];
		pIn[other] = r.point.v[other] + r.direction.v[other] * tOut;
		other = otherAxis[exitFace][1];
		pIn[other] = r.point.v[other] + r.direction.v[other] * tOut;

		// û�з���Ҫ��Ľ��㣬����ropeת����Ӧ�Ľڵ���
		unsigned int nextID = (unsigned int)pLeaf->ropes[!nearFace[exitFace]][exitFace];
		if (nextID == 0xffffffff)
			break;
		char subNodeID = nextID >> 29;
		nextID &= 0x1fffffff;
		if (subNodeID == 0x7)
			pNode = kdLeafs + nextID;
		else
		{
			++numTraverseInterior_I;
			char subNodeLayer = (subNodeID > 0) + (subNodeID > 2);
			WEKDInterior* pPostNode = ekdInteriors + nextID;
			char axis; float t; char childID;
			switch (subNodeLayer)
			{
			case 0:
				axis = (pPostNode->etype >> 2) & 0x3;
				t = pIn[axis];
				childID = t > pPostNode->splitPlane[subNodeID];
				if (t == pPostNode->splitPlane[subNodeID])
					childID = nearFace[axis];
				subNodeID = subNodeID * 2 + childID + 1;
			case 1:
				axis = (pPostNode->etype >> (2 + subNodeID * 2)) & 0x3;
				t = pIn[axis];
				childID = t > pPostNode->splitPlane[subNodeID];
				if (t == pPostNode->splitPlane[subNodeID])
					childID = nearFace[axis];
				subNodeID = subNodeID * 2 + childID + 1;
			case 2:
				axis = (pPostNode->etype >> (2 + subNodeID * 2)) & 0x3;
				t = pIn[axis];
				childID = t > pPostNode->splitPlane[subNodeID];
				if (t == pPostNode->splitPlane[subNodeID])
					childID = nearFace[axis];
				subNodeID = subNodeID * 2 + childID + 1;
			}
			pNode = pPostNode->child[subNodeID - 7];
		}
	}
	if (!bestTriangle)
		return false;
	if (endNode)
		*endNode = (pNode - kdLeafs) / sizeof(WSKDLeaf);
	bestTriangle->intersect(r,DG);
	return true;
}



bool WEnhancedKD::isIntersect( WRay& r, int beginNode /*= -1*/ )
{
	++numIntersectTest;
	// ��ʾ����߽Ͻ���������, 0 ��ʾ����ֵС�棬 1��ʾ����ֵ����
	const char nearFace[3] = {r.direction.x < 0.0f, r.direction.y < 0.0f, r.direction.z < 0.0f};

	// ������ߵ�������Χ�еĽ����Լ�������exitFace
	const float invD[3] = {1.0f / r.direction.x, 1.0f / r.direction.y, 1.0f / r.direction.z};
	float tIn  =            (sceneBox[nearFace[0]][0] - r.point.x) * invD[0];
	tIn        = maxf(tIn,  (sceneBox[nearFace[1]][1] - r.point.y) * invD[1]);
	tIn        = maxf(tIn,  (sceneBox[nearFace[2]][2] - r.point.z) * invD[2]);
	float tOut  =             (sceneBox[!nearFace[0]][0] - r.point.x) * invD[0];
	tOut        = minf(tOut,  (sceneBox[!nearFace[1]][1] - r.point.y) * invD[1]);
	tOut        = minf(tOut,  (sceneBox[!nearFace[2]][2] - r.point.z) * invD[2]);
	if (tIn > tOut || tOut < 0.0f || numInteriors == 0)
		return false;

	__m128 mailBox[4];
	mailBox[0] = _mm_castsi128_ps(_mm_set1_epi32(0));
	mailBox[1] = _mm_castsi128_ps(_mm_set1_epi32(0));
	WTriangle** pMainBox = (WTriangle**)&mailBox[0];
	char curMailBox = 0;

	tIn = maxf(tIn, 0.0f);
	float pIn[3] = {r.point.x + r.direction.x * tIn,
		r.point.y + r.direction.y * tIn,
		r.point.z + r.direction.z * tIn};
	float tBest = FLT_MAX;
	WSKDNode* pNode = ekdInteriors;
	WTriangle* bestTriangle = NULL;
	if (beginNode != -1)
		pNode = kdLeafs + beginNode;
	char entryLayer = 0;
	while(pNode)
	{
		while (pNode->type != WSKDNode::KDN_LEAF)
		{	// �ڲ��ڵ�
			++numTraverseInterior_IT;
			WEKDInterior* pInterior = (WEKDInterior*)pNode;
			const unsigned char axis0 = ((pInterior->etype) >> 2) & 0x3;
			float t = pIn[axis0];
			char l1 = t > pInterior->splitPlane[0];
			if (t == pInterior->splitPlane[0])
				l1 = nearFace[axis0];

			const unsigned char axis1 = (pInterior->etype >> (4 + l1 * 2)) & 0x3;
			t = pIn[axis1];
			char l2 = t > pInterior->splitPlane[1 + l1];
			if (t == pInterior->splitPlane[1 + l1])
				l2 = nearFace[axis1];

			const unsigned char axis2 = (pInterior->etype >> (8 + l1 * 4 + l2 * 2)) & 0x3;
			t = pIn[axis2];
			char l3 = t > pInterior->splitPlane[3 + l1*2 + l2];
			if (t == pInterior->splitPlane[3 + l1*2 + l2])
				l3 = nearFace[axis2];
			pNode = pInterior->child[l1 * 4 + l2 * 2 + l3];
		}
		++numTraverseLeaf_IT;
		WSKDLeaf* pLeaf = (WSKDLeaf*)pNode;
		WTriangle** tris = pLeaf->triangle;
		int nTris = pLeaf->nTriangles;
		while(nTris--)
		{
			WTriangle* pTri = *tris;
#ifdef SKD_MAILBOX
			__m128 pTri4 = _mm_set_ps1(*reinterpret_cast<float*>(&pTri));
			__m128 tmp14  = _mm_cmpeq_ps(pTri4, mailBox[0]);
			__m128 tmp24  = _mm_cmpeq_ps(pTri4, mailBox[1]);
			tmp14 = _mm_or_ps(tmp14, tmp24);
			if (!_mm_movemask_ps(tmp14))
			{
#endif
				++numTriangleIsect_IT;
				float tIsect =  pTri->intersectTest(r);
				if (tIsect < tBest)
					return true;
#ifdef SKD_MAILBOX
			}
			pMainBox[curMailBox] = pTri;
			curMailBox = (++curMailBox) & 0x7;
#endif
			tris++;
		}
		char exitFace = 0;
		tOut = (pLeaf->box[!nearFace[0]][0] - r.point.x) * invD[0];
		float tTemp = (pLeaf->box[!nearFace[1]][1] - r.point.y) * invD[1];
		if (tTemp < tOut){
			tOut = tTemp;	exitFace = 1;
		}
		tTemp = (pLeaf->box[!nearFace[2]][2] - r.point.z) * invD[2];
		if (tTemp < tOut){
			tOut = tTemp;	exitFace = 2;
		}
		if (tOut > r.tMax)
			return false;
		pIn[exitFace] = pLeaf->box[!nearFace[exitFace]][exitFace];
		char other = otherAxis[exitFace][0];
		pIn[other] = r.point.v[other] + r.direction.v[other] * tOut;
		other = otherAxis[exitFace][1];
		pIn[other] = r.point.v[other] + r.direction.v[other] * tOut;

		// û�з���Ҫ��Ľ��㣬����ropeת����Ӧ�Ľڵ���
		unsigned int nextID = (unsigned int)pLeaf->ropes[!nearFace[exitFace]][exitFace];
		if (nextID == 0xffffffff)
			break;
		char subNodeID = nextID >> 29;
		nextID &= ~(0x7 << 29);
		if (subNodeID == 0x7)
			pNode = kdLeafs + nextID;
		else
		{
			++numTraverseInterior_IT;
			char subNodeLayer = (subNodeID > 0) + (subNodeID > 2);
			WEKDInterior* pPostNode = ekdInteriors + nextID;
			char axis; float t; char childID;
			switch (subNodeLayer)
			{
			case 0:
				axis = (pPostNode->etype >> (2 + subNodeID * 2)) & 0x3;
				t = pIn[axis];
				childID = t > pPostNode->splitPlane[subNodeID];
				if (t == pPostNode->splitPlane[subNodeID])
					childID = nearFace[axis];
				subNodeID = subNodeID * 2 + childID + 1;
			case 1:
				axis = (pPostNode->etype >> (2 + subNodeID * 2)) & 0x3;
				t = pIn[axis];
				childID = t > pPostNode->splitPlane[subNodeID];
				if (t == pPostNode->splitPlane[subNodeID])
					childID = nearFace[axis];
				subNodeID = subNodeID * 2 + childID + 1;
			case 2:
				axis = (pPostNode->etype >> (2 + subNodeID * 2)) & 0x3;
				t = pIn[axis];
				childID = t > pPostNode->splitPlane[subNodeID];
				if (t == pPostNode->splitPlane[subNodeID])
					childID = nearFace[axis];
				subNodeID = subNodeID * 2 + childID + 1;
			}
			pNode = pPostNode->child[subNodeID - 7];
		}
	}
	return false;
}

void WEnhancedKD::drawTree( unsigned int nthBox/*=0*/, float R /*= 0.7*/, float G /*= 0.7*/, float B /*= 0.7*/ )
{
	WSimpleKD::drawTree(nthBox, R, G, B);
	return;
}

void WEnhancedKD::displayNodeInfo()
{
	printf("oriNodes------------------------------------------\n");
	deque<WSKDInterior*> stack0;
	stack0.push_back(kdInteriors);
	int num0 = 0;

	deque<WEKDInterior*> stack1;
	stack1.push_back(ekdInteriors);
	int num1 = 0;
	while (stack0.size())
	{
		WSKDInterior* pNode = stack0.front();
		stack0.pop_front();
		num0++;
		//printf("split %d, t = %f\n", pNode->type, pNode->splitPlane);
		for (char i = 0; i < 2; ++i)
			if (pNode->child[i]->type != WSKDNode::KDN_LEAF)
			{
				stack0.push_back((WSKDInterior*)pNode->child[i]);
			}
	}

	printf("newNodes------------------------------------------\n");
	while (stack1.size())
	{
		WEKDInterior* pNode = stack1.front();
		stack1.pop_front();
		num1 += 7;/*
		printf("split %d, t = %f\n", (pNode->etype >> 2) & 0x3, pNode->splitPlane[0]);
		printf("split %d, t = %f\n", (pNode->etype >> 4) & 0x3, pNode->splitPlane[1]);
		printf("split %d, t = %f\n", (pNode->etype >> 6) & 0x3, pNode->splitPlane[2]);
		printf("split %d, t = %f\n", (pNode->etype >> 8) & 0x3, pNode->splitPlane[3]);
		printf("split %d, t = %f\n", (pNode->etype >> 10) & 0x3, pNode->splitPlane[4]);
		printf("split %d, t = %f\n", (pNode->etype >> 12) & 0x3, pNode->splitPlane[5]);
		printf("split %d, t = %f\n", (pNode->etype >> 14) & 0x3, pNode->splitPlane[6]);*/
		for (char i = 0; i < 8; ++i)
			if (pNode->child[i]->type != WSKDNode::KDN_LEAF)
				stack1.push_back((WEKDInterior*)pNode->child[i]);
	}
	printf("num0 = %d     num1 = %d", num0, num1);
	return;
}

void WEnhancedKD::drawTriangles()
{

}
