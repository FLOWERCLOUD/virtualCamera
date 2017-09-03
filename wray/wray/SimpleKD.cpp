#include "StdAfx.h"
#include "SimpleKD.h"
const char WSimpleKD::otherAxis[3][2] = {{1,2},{0,2},{0,1}};
WSimpleKD::WSimpleKD(void)
{
	KT = 1;
	KI = 80;
	triangles = NULL;
	kdInteriors = NULL;
	kdLeafs = NULL;
	leafTriangles = NULL;
	numInteriors = numLeafs = treeDepth = totalTriangles = numLeafTriangles = 0;
	numInteriorsAllocated = numLeafsAllocated = numLeafTrianglesAllocated = 0;
	minBoxFactor = SKD_MIN_BOX_FACTOR;
}

WSimpleKD::~WSimpleKD(void)
{
	clearTree();
}

void WSimpleKD::buildTree( WScene&scene )
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
	int approNodes = max(totalTriangles, 5000) * 6;
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
	buildCacheFriendlyNode();
	buildBasicRopes(kdInteriors, NULL, NULL, NULL, NULL, NULL, NULL);
#ifdef SKD_EXTENDED_ROPES
	buildExtendedRopes();
#endif
}
WSKDInterior* WSimpleKD::requestInterior()
{
	if (numInteriors < numInteriorsAllocated)
		return kdInteriors + (numInteriors++);
	// �ڲ��ڵ�ռ䲻�������·��䣬�����������ö�Ӧ��ָ��
	numInteriorsAllocated *= 2;
	WSKDInterior* tempInterior = new WSKDInterior[numInteriorsAllocated];
	memcpy((void*)tempInterior, (void*)kdInteriors, sizeof(WSKDInterior) * numInteriors);
	delete[] kdInteriors;
	kdInteriors = tempInterior;
	return kdInteriors + (numInteriors++);
}
WSKDLeaf* WSimpleKD::requestLeaf()
{
	if (numLeafs < numLeafsAllocated)
		return kdLeafs + (numLeafs++);
	numLeafsAllocated *= 2;
	WSKDLeaf* tempLeaf = new WSKDLeaf[numLeafsAllocated];
	memcpy((void*)tempLeaf, (void*)kdLeafs, sizeof(WSKDLeaf) * numLeafs);
	delete[] kdLeafs;
	kdLeafs = tempLeaf;
	return kdLeafs + (numLeafs++);
}
WTriangle** WSimpleKD::requestLeafTriangles(int nTri)
{
	if (numLeafTriangles + nTri >= numLeafTrianglesAllocated)
	{
		numLeafTrianglesAllocated *= 2;
		WTriangle** tempTri = new WTriangle*[numLeafTrianglesAllocated];
		memcpy((void*)tempTri, (void*)leafTriangles, sizeof(WTriangle*) * numLeafTriangles);
		delete[] leafTriangles;
		leafTriangles = tempTri;
	}
	WTriangle** ret = leafTriangles + numLeafTriangles;
	numLeafTriangles += nTri;
	return ret;
}
void WSimpleKD::buildTreeKernel( vector<WBoundingEdge>&edges, 
								int nTris,
								int depth, 
								float nodeBox[2][3],
								char triangleMap[],
								int parentIndex,
								char branch)
{
	//printf("depth: %d\n", depth);
	// ���������������Լ���Χ������
	treeDepth = depth > treeDepth ? depth : treeDepth;
	//nodeBoxes.push_back(WBoundingBox(nodeBox));

	// �����Χ�д�С
	float boxSize[3];
	boxSize[0] = (nodeBox[1][0] - nodeBox[0][0]);
	boxSize[1] = (nodeBox[1][1] - nodeBox[0][1]);
	boxSize[2] = (nodeBox[1][2] - nodeBox[0][2]);
	// ׼����¼���λ�õ�����
	float bestSAH = FLT_MAX;		// ��ÿ���
	float bestT   = 0;
	char  bestSide = 0;				// �ָ�ƽ���ϵĽڵ����
	char  bestAxis = 0;
	int   bestLTri = nTris, bestRTri = 0;
	vector<WBoundingEdge>::iterator bestP = edges.end();// ��ѷָ�ƽ��

	//printf("begin to find best plane\n");
	// �ҳ���ѵķָ�ƽ�棬��3��ά�ȷֱ�����
	vector<WBoundingEdge>::iterator pE = edges.begin();
	vector<WBoundingEdge>::iterator pEnd = edges.end();
	for (int axis = 0; axis < 3; ++axis)
	{
		int NL = 0, NP = 0, NR = nTris;
		// BoundingEdge �Ȱ�������������ͬ�����ղ���tֵ������ͬtֵ������������
		for (; pE != pEnd && pE->axis == axis;)
		{
			// ���ﵱǰ�����һ��BoundingEdge
			// ����ͳ���ڵ�ǰ����λ�õ�����BE����
			vector<WBoundingEdge>::iterator p = pE;
			int N[3] = {0, 0, 0};		// N[0] N[1] N[2] --> nEnd nPlanar nStart
			for (;pE != pEnd && pE->t == p->t && pE->axis == axis;++pE)
			{
				triangleMap[pE->triangleID] = 2;
				N[pE->type]++;
			}

			NP = N[1];			// ��ǰλ�õ�ƽ��߶���������������
			NR -= N[1] + N[0];	// ��ǰλ�õĽ����߹鵽��ڵ�

			// ���ڵõ���ǰ�ָ�ƽ�����ҵ������θ���
			// Ҫ������ѵķָ�
			float PL, PR;				// ���а�Χ���������ߵĸ���
			if (boxSize[axis] == 0.0f)	// ��Χ�к��Ϊ0ʱ����Ϊ�������߸������
				PL = PR = 1.0f;
			else
			{	/*						// �������Ϊ�������ߵı������
				PL = (p->t - nodeBox[0][axis]) / boxSize[axis];
				PR = (nodeBox[1][axis] - p->t) / boxSize[axis];*/
				float halfArea = boxSize[0] * boxSize[1] + boxSize[1] * boxSize[2] + boxSize[2] * boxSize[0];
				float w = boxSize[otherAxis[axis][0]];
				float h = boxSize[otherAxis[axis][1]];
				float lL = abs(p->t - nodeBox[0][axis]);
				float lR = abs(nodeBox[1][axis] - p->t);
				PL = (w*h + h*lL + lL*w) / halfArea;
				PR = (w*h + h*lR + lR*w) / halfArea;
			}


			// �ֱ���ƽ��߹鵽���������ӽڵ�������ѡ�����ŵ�һ��
			float SAH0 = KT + KI * (PL * (NL + NP) + PR * NR);
			float SAH1 = KT + KI * (PL * NL + PR * (NP + NR));
			if ((NL + NP == 0 && PL != 0) || (NR == 0 && PR != 0))
				SAH0 *= 0.8f;
			if ((NR + NP == 0 && PR != 0) || (NL == 0 && PL != 0))
				SAH1 *= 0.8f;
			char side = SAH0 < SAH1 ? 0 : 1;
			float SAH = minf(SAH0, SAH1);

			// ���ָ��ŵķָ�����
			if (SAH < bestSAH)
			{
				bestP = p;  bestSAH = SAH;  bestSide = side;
				bestT = p->t; bestAxis = axis;
				bestLTri = SAH0 < SAH1 ? (NL + NP) : NL;
				bestRTri = SAH0 < SAH1 ? NR : (NR + NP);
			}
			NL += N[1] + N[2];
		}
	}
	// ������ѵķָ�ƽ��,ȷ��ÿ�������εĹ���
	//map<int, char>triangleMap;  // ��������� --> ���ͣ�0��ߣ�1�ұߣ�2���ߣ�
	// �����������ζ����Ϊ����
	//for (pE = edges.begin();pE != pEnd; ++pE)

	// ��������½�Ҷ�ڵ�
	// 1. �Ҳ����κηָ�
	// 2. ��ѵķָ������Ȳ��ָ��Ҫ��
	// 3. ������ȳ������ֵ
	// 4. ���������������Ƽ�ֵ
	// 5. ��ѷָ�����ʹ����һ��Ϊ���Ϊ0�Ŀսڵ�
	// 6. ��ѷָ�����ʹ��һ�߽ڵ����������������������
	// 7. �ڵ��СС��һ��ֵ
	if (bestSAH == FLT_MAX || bestSAH > KI * nTris || 
		depth > SKD_MAX_DEPTH || nTris <= SKD_RECOMM_LEAF_TRIS ||
		(bestP->t == nodeBox[0][bestAxis] && bestLTri == 0) ||
		(nodeBox[1][bestAxis] == bestP->t && bestRTri == 0) ||
		(bestP->t - nodeBox[0][bestAxis] == boxSize[bestAxis] && bestLTri == nTris) ||
		(nodeBox[1][bestAxis] - bestP->t == boxSize[bestAxis] && bestRTri == nTris) ||
		(boxSize[0] < minBoxSize[0] && boxSize[1] < minBoxSize[1] && boxSize[2] < minBoxSize[2]))
	{
		// ����Ҷ�ڵ��ƫ����,���Ż����ʵ�ʵ�ַ���θ�λ��1����ΪҶ�ڵ�
		// ���Ӹ��ڵ㵽��ǰ�ڵ��ָ��
		kdInteriors[parentIndex].child[branch] = (WSKDNode*)(numLeafs* sizeof(WSKDLeaf) | SKD_LEAF_MASK);

		// ����һ��Ҷ�ڵ㲢��ֵ������ռ䲻���������·���ռ�
		WSKDLeaf * leaf = requestLeaf();
		leaf->type = WSKDNode::KDN_LEAF;
		memcpy((void*)leaf->box, (void*)nodeBox, sizeof(float) * 6);

		// ͳ��Ҷ�ڵ�������
		set<int> triangleSet;
		for (vector<WBoundingEdge>::iterator pE = edges.begin(); pE != edges.end(); ++pE)
			triangleSet.insert(pE->triangleID);
		// Ҷ�ڵ�ָ�������������ƫ��������������Ҷ�ڵ������εĿռ�
		leaf->triangle = nTris != 0 ? (WTriangle**)(numLeafTriangles * sizeof(WTriangle*)) : NULL;
		WTriangle** pTri = requestLeafTriangles(triangleSet.size());
		int i = 0;
		for (set<int>::iterator pS = triangleSet.begin(); pS != triangleSet.end(); ++pS, ++i)
			pTri[i] = triangles + *pS;
		leaf->nTriangles = triangleSet.size();
		return;
	}

	//printf("begin to mark triangle\n");
	for (pE = edges.begin(); pE != pEnd; ++pE)
	{
		if (pE->axis == WBoundingEdge::EdgeAxis(bestAxis))
		{
			if (pE->type == WBoundingEdge::BE_END && pE->t <= bestT)
				triangleMap[pE->triangleID]  = 0;
			else if (pE->type == WBoundingEdge::BE_START && pE->t >= bestT)
				triangleMap[pE->triangleID]  = 1;
			else if (pE->type == WBoundingEdge::BE_PLANAR)
			{
				if (pE->t < bestT || (pE->t == bestT && bestSide == 0))
					triangleMap[pE->triangleID]  = 0;
				else if (pE->t > bestT || (pE->t == bestT && bestSide == 1))
					triangleMap[pE->triangleID]  = 1;
			}
		}
	}

	//printf("begin to generate edge\n");
	// ��ɨ��һ��event����ֻ����һ�߽ڵ�������ζ�Ӧ��event�����Ӧ�ӽڵ�event�б�
	// ����¼��������ڵ��������
	vector<WBoundingEdge> oriEdgesL, oriEdgesR;
	oriEdgesL.reserve(edges.size());
	oriEdgesR.reserve(edges.size());
	set<int> midTris;
	for (pE = edges.begin(); pE != pEnd;++pE)
	{
		if (triangleMap[pE->triangleID] == 0)
			oriEdgesL.push_back(*pE);
		else if (triangleMap[pE->triangleID] == 1)
			oriEdgesR.push_back(*pE);
		else
			midTris.insert(pE->triangleID);
	}

	// ���ں�������ڵ�������Σ������µ�BE
	// �õ����ҽڵ�İ�Χ��
	float LNodeBox[2][3];float RNodeBox[2][3];
	memcpy((void*)LNodeBox, (void*)nodeBox, sizeof(float) * 6);
	memcpy((void*)RNodeBox, (void*)nodeBox, sizeof(float) * 6);
	LNodeBox[1][bestAxis] = RNodeBox[0][bestAxis] = bestT;

	//printf("begin to clip triangle\n");
	// ʹ�ð�Χ�н��вü����ѽ�������ݴ��BE�б�
	WBoxClipper clipper0, clipper1;
	clipper0.setClipBox(LNodeBox);
	clipper1.setClipBox(RNodeBox);
	vector<WBoundingEdge> tempEdgesL, tempEdgesR;
	tempEdgesL.reserve(midTris.size() * 2 * 3);
	tempEdgesR.reserve(midTris.size() * 2 * 3);
	for (set<int>::iterator pT = midTris.begin();
		 pT != midTris.end(); ++pT)
	{
		WTriangle* pTri = triangles + *pT;
		float LRes[2][3], RRes[2][3];
		bool isLIn = clipper0.getClipBox(*pTri, LRes);
		bool isRIn = clipper1.getClipBox(*pTri, RRes);
		// ���������������ζ�Ӧ�����Χ���ཻ
		// ���ڸ���������Ӧ�ð���ĳ���ӽ������������εİ�Χ�п��ܺ��Ϊ0
		// ������������ȫ����Χ�вü�������ʱ��BoundingEdge����Ϊ��Χ�еĴ�С
		if (!isLIn)
			memcpy((void*)LRes, (void*)LNodeBox, sizeof(float) * 2 * 3);
		if (!isRIn)
			memcpy((void*)RRes, (void*)RNodeBox, sizeof(float) * 2 * 3);
		for(char i = 0; i < 3; i++)
		{
			WBoundingEdge::EdgeAxis a = WBoundingEdge::EdgeAxis(i);
			if (LRes[0][i] == LRes[1][i])
				tempEdgesL.push_back(WBoundingEdge(WBoundingEdge::BE_PLANAR, a, LRes[0][i], *pT));
			else
			{
				tempEdgesL.push_back(WBoundingEdge(WBoundingEdge::BE_START, a, LRes[0][i], *pT));
				tempEdgesL.push_back(WBoundingEdge(WBoundingEdge::BE_END, a, LRes[1][i], *pT));
			}
			if (RRes[0][i] == RRes[1][i])
				tempEdgesR.push_back(WBoundingEdge(WBoundingEdge::BE_PLANAR, a, RRes[0][i], *pT));
			else
			{
				tempEdgesR.push_back(WBoundingEdge(WBoundingEdge::BE_START, a, RRes[0][i], *pT));
				tempEdgesR.push_back(WBoundingEdge(WBoundingEdge::BE_END, a, RRes[1][i], *pT));
			}
		}
		if (!isLIn)
			printf("tri%d not in L\n",*pT);
		if (!isRIn)
			printf("tri%d not in R\n",*pT);
	}
	
	// ���µ�BE����ԭ���ļ����У�����ӽڵ��µ�BE����
	sort(tempEdgesL.begin(), tempEdgesL.end());
	sort(tempEdgesR.begin(), tempEdgesR.end());
	/*vector<WBoundingEdge> edgesL, edgesR;
	edgesL.resize(oriEdgesL.size() + tempEdgesL.size());
	edgesR.resize(oriEdgesR.size() + tempEdgesR.size());*/
	edges.clear();
	edges.resize(oriEdgesL.size() + tempEdgesL.size());
	merge(oriEdgesL.begin(), oriEdgesL.end(), tempEdgesL.begin(), tempEdgesL.end(), edges.begin());
	tempEdgesL.clear();		oriEdgesL.clear();

	// �洢��ǰ�ڵ�
	unsigned curIndex = numInteriors;
	WSKDInterior* pNode = requestInterior();
	pNode->type = WSKDNode::NodeType(bestAxis);
	pNode->splitPlane = bestT;

	kdInteriors[parentIndex].child[branch] = (WSKDNode*)(curIndex * sizeof(WSKDInterior) | SKD_INTERIOR_MASK);
	buildTreeKernel(edges, bestLTri, depth + 1, LNodeBox, triangleMap, curIndex, 0);

	edges.resize(oriEdgesR.size() + tempEdgesR.size());
	merge(oriEdgesR.begin(), oriEdgesR.end(), tempEdgesR.begin(), tempEdgesR.end(), edges.begin());
	tempEdgesR.clear();		oriEdgesR.clear();

	buildTreeKernel(edges, bestRTri, depth + 1, RNodeBox, triangleMap, curIndex, 1);
}

void WSimpleKD::clearTree()
{
	triangles = NULL;
	delete []kdInteriors;
	delete []kdLeafs;
	delete []leafTriangles;
	kdInteriors = NULL;
	kdLeafs = NULL;
	leafTriangles = NULL;
	numInteriors = numLeafs = treeDepth = totalTriangles = numLeafTriangles = 0;
	numInteriorsAllocated = numLeafsAllocated = numLeafTrianglesAllocated = 0;
}

void WSimpleKD::drawTree( unsigned int nthBox/*=0*/, float R /*= 0.7*/, float G /*= 0.7*/, float B /*= 0.7*/ )
{
//	cout<<"\nnode size:"<<nodes.size()<<endl;
	glColor3f(R, G, B);
//	drawTreeRecursive(kdInteriors, sceneBox);
	for (WSKDLeaf* pLeaf = kdLeafs ;pLeaf < kdLeafs + numLeafs; ++pLeaf)
	{
		WBoundingBox box(pLeaf->box);
		box.draw();
	}
}

void WSimpleKD::drawTreeRecursive( WSKDNode* node, const WBoundingBox& box )
{
	if (!numInteriors)
		return;
	box.draw();
	WBoundingBox leftBox, rightBox;
	leftBox = rightBox = box;
	switch(node->type)
	{
	case WSKDNode::KDN_LEAF:
		return;
	case WSKDNode::KDN_XSPLIT:
		leftBox.pMax.x = rightBox.pMin.x = ((WSKDInterior*)node)->splitPlane;
		break;
	case WSKDNode::KDN_YSPLIT:
		leftBox.pMax.y = rightBox.pMin.y = ((WSKDInterior*)node)->splitPlane;
		break;
	case WSKDNode::KDN_ZSPLIT:
		leftBox.pMax.z = rightBox.pMin.z = ((WSKDInterior*)node)->splitPlane;
		break;
	default:
		return;
	}
	drawTreeRecursive(((WSKDInterior*)node)->child[0], leftBox);
	drawTreeRecursive(((WSKDInterior*)node)->child[1], rightBox);
}


void WSimpleKD::buildBasicRopes(WSKDNode* pNode, 
							   WSKDNode* ropeX_P, WSKDNode* ropeX_N,
							   WSKDNode* ropeY_P, WSKDNode* ropeY_N,
							   WSKDNode* ropeZ_P, WSKDNode* ropeZ_N)
{
	if (pNode->type == WSKDNode::KDN_LEAF)
	{
		// �Ѵ�������rope����Ҷ�ڵ�
		WSKDLeaf* pLeaf = (WSKDLeaf*)pNode;
		pLeaf->ropes[1][0] = ropeX_P;
		pLeaf->ropes[0][0] = ropeX_N;
		pLeaf->ropes[1][1] = ropeY_P;
		pLeaf->ropes[0][1] = ropeY_N;
		pLeaf->ropes[1][2] = ropeZ_P;
		pLeaf->ropes[0][2] = ropeZ_N;
	} 
	else if(pNode->type == WSKDLeaf::KDN_XSPLIT)
	{
		// �޸�x�����rope��ʹ�������ӽڵ���x������ָ��Է�
		WSKDInterior* pInterior = (WSKDInterior*)pNode;
		buildBasicRopes(pInterior->child[0],
			pInterior->child[1], ropeX_N, ropeY_P, ropeY_N, ropeZ_P, ropeZ_N);
		buildBasicRopes(pInterior->child[1],
			ropeX_P, pInterior->child[0], ropeY_P, ropeY_N, ropeZ_P, ropeZ_N);
	}
	else if(pNode->type == WSKDLeaf::KDN_YSPLIT)
	{
		// ��������
		WSKDInterior* pInterior = (WSKDInterior*)pNode;
		buildBasicRopes(pInterior->child[0],
			ropeX_P, ropeX_N, pInterior->child[1], ropeY_N, ropeZ_P, ropeZ_N);
		buildBasicRopes(pInterior->child[1],
			ropeX_P, ropeX_N, ropeY_P, pInterior->child[0], ropeZ_P, ropeZ_N);
	}
	else if(pNode->type == WSKDLeaf::KDN_ZSPLIT)
	{
		// ��������
		WSKDInterior* pInterior = (WSKDInterior*)pNode;
		buildBasicRopes(pInterior->child[0],
			ropeX_P, ropeX_N, ropeY_P, ropeY_N, pInterior->child[1], ropeZ_N);
		buildBasicRopes(pInterior->child[1],
			ropeX_P, ropeX_N, ropeY_P, ropeY_N, ropeZ_P, pInterior->child[0]);
	}
}


bool WSimpleKD::WBoundingEdge::operator<( const WBoundingEdge&e ) const
{
	//��ͬλ��, tֵ��С�Ľ�ǰ
	//���� t ���ʱ��ȷ�� planar���͵���ǰ��start���͵Ľ�ǰ�� end ���͵ĽϺ�
	//������ȷ���ڱ�������ε�ʱ��������
	//һ�������ε�start�ߣ�������end��
	return (axis < e.axis) || (axis == e.axis) && ((t < e.t) || ((t == e.t) && (type < e.type)));
}

bool WSimpleKD::isIntersect( WRay& r, int beginNode)	
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

	__m128 mailBox[2];
	mailBox[0] = _mm_castsi128_ps(_mm_set1_epi32(0));
	mailBox[1] = _mm_castsi128_ps(_mm_set1_epi32(0));
	WTriangle** pMainBox = (WTriangle**)&mailBox[0];
	char curMailBox = 0;

	tIn = maxf(tIn, 0.0f);
	float pIn[3] = {r.point.x + r.direction.x * tIn,r.point.y + r.direction.y * tIn,r.point.z + r.direction.z * tIn};
	float tBest = M_INF_BIG;
	WSKDNode* pNode = kdInteriors;
	if (beginNode != -1)
		pNode = kdLeafs + beginNode;
	while(pNode)
	{
		while (pNode->type != WSKDNode::KDN_LEAF)
		{	// �ڲ��ڵ�
			++numTraverseInterior_IT;
			WSKDInterior* pInterior = (WSKDInterior*)pNode;
			char axis = pInterior->type;
			pNode = (pIn[axis] > pInterior->splitPlane) ? pInterior->child[1] : pInterior->child[0];
			if (pIn[axis] == pInterior->splitPlane)
				pNode = pInterior->child[nearFace[axis]];
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

		// ��ʱ��Ҷ�ڵ������������ζ����ཻ
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
		pNode = pLeaf->ropes[!nearFace[exitFace]][exitFace];
		pIn[exitFace] = pLeaf->box[!nearFace[exitFace]][exitFace];
		char other = otherAxis[exitFace][0];
		pIn[other] = r.point.v[other] + r.direction.v[other] * tOut;
		other = otherAxis[exitFace][1];
		pIn[other] = r.point.v[other] + r.direction.v[other] * tOut;
	}
	return false;
}
bool WSimpleKD::intersect( WRay& r,WDifferentialGeometry& DG, 
						 int* endNode, int beginNode)
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

	WSKDNode* pNode = kdInteriors;
	WTriangle* bestTriangle = NULL;
	if (beginNode != -1)
		pNode = kdLeafs + beginNode;
	while(pNode)
	{
		while (pNode->type - WSKDNode::KDN_LEAF)
		{	// �ڲ��ڵ�
			++numTraverseInterior_I;
			WSKDInterior* pInterior = (WSKDInterior*)pNode;
			const char axis = pInterior->type;
			const float t = pIn[axis];
			pNode = (t > pInterior->splitPlane) ? pInterior->child[1] : pInterior->child[0];
			if (t == pInterior->splitPlane)
				pNode = pInterior->child[nearFace[axis]];
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
			break;					// �ҵ�����Ľ���
		// û�з���Ҫ��Ľ��㣬����ropeת����Ӧ�Ľڵ���
		pNode = pLeaf->ropes[!nearFace[exitFace]][exitFace];
		pIn[exitFace] = pLeaf->box[!nearFace[exitFace]][exitFace];
		char other = otherAxis[exitFace][0];
		pIn[other] = r.point.v[other] + r.direction.v[other] * tOut;
		other = otherAxis[exitFace][1];
		pIn[other] = r.point.v[other] + r.direction.v[other] * tOut;
	}
	if (!bestTriangle)
		return false;
	if (endNode)
		*endNode = (pNode - kdLeafs) / sizeof(WSKDLeaf);
	bestTriangle->intersect(r,DG);
	return true;
}


bool WSimpleKD::intersect4(WRay r[4], WDifferentialGeometry DG[4], int endNode[4], int beginNode[4])
{
	return false;/*
	++numIntersect;
	// ��ʾ����߽Ͻ���������, 0 ��ʾ����ֵС�棬 1��ʾ����ֵ����
	__m128 nearFace4[3], farFace4[3];
	__m128 invD[3];
	__m128 zero4 = _mm_set_ps1(0.0f);
	__m128 one4  = _mm_castsi128_ps(_mm_set1_epi32(0x1));
	nearFace4[0] = _mm_set_ps(r[0].direction.x, r[1].direction.x, r[2].direction.x, r[3].direction.x);
	nearFace4[1] = _mm_set_ps(r[0].direction.y, r[1].direction.y, r[2].direction.y, r[3].direction.y);
	nearFace4[2] = _mm_set_ps(r[0].direction.z, r[1].direction.z, r[2].direction.z, r[3].direction.z);
	invD[0] = _mm_div_ps(one4, nearFace4[0]);
	invD[1] = _mm_div_ps(one4, nearFace4[1]);
	invD[2] = _mm_div_ps(one4, nearFace4[2]);
	nearFace4[0] = _mm_and_ps(_mm_cmplt_ps(nearFace4[0], zero4),one4);
	nearFace4[1] = _mm_and_ps(_mm_cmplt_ps(nearFace4[1], zero4),one4);
	nearFace4[2] = _mm_and_ps(_mm_cmplt_ps(nearFace4[2], zero4),one4);
	farFace4[0] = _mm_andnot_ps(nearFace4[0], one4);
	farFace4[1] = _mm_andnot_ps(nearFace4[1], one4);
	farFace4[2] = _mm_andnot_ps(nearFace4[2], one4);

	// ������ߵ�������Χ�еĽ����Լ�������exitFace
	int* pNearFace4 = (int*)& nearFace4;
	int* pFarFace4 = (int*)& farFace4;
	__m128 nearBox4[3], farBox4[3];
	nearBox4[0] = _mm_set_ps(sceneBox[pNearFace4[0]][0], sceneBox[pNearFace4[1]][0], 
								  sceneBox[pNearFace4[2]][0], sceneBox[pNearFace4[3]][0]);
	nearBox4[1] = _mm_set_ps(sceneBox[pNearFace4[4]][1], sceneBox[pNearFace4[5]][1], 
								  sceneBox[pNearFace4[6]][1], sceneBox[pNearFace4[7]][1]);
	nearBox4[2] = _mm_set_ps(sceneBox[pNearFace4[8]][2], sceneBox[pNearFace4[9]][2], 
								  sceneBox[pNearFace4[10]][2], sceneBox[pNearFace4[11]][2]);

	farBox4[0] = _mm_set_ps(sceneBox[pFarFace4[0]][0], sceneBox[pFarFace4[1]][0], 
		sceneBox[pFarFace4[2]][0], sceneBox[pFarFace4[3]][0]);
	farBox4[1] = _mm_set_ps(sceneBox[pFarFace4[4]][1], sceneBox[pFarFace4[5]][1], 
		sceneBox[pFarFace4[6]][1], sceneBox[pFarFace4[7]][1]);
	farBox4[2] = _mm_set_ps(sceneBox[pFarFace4[8]][2], sceneBox[pFarFace4[9]][2], 
		sceneBox[pFarFace4[10]][2], sceneBox[pFarFace4[11]][2]);

	__m128 ori[3];
	ori[0] = _mm_set_ps(r[0].point.x, r[1].point.x, r[2].point.x, r[3].point.x);
	ori[1] = _mm_set_ps(r[0].point.y, r[1].point.y, r[2].point.y, r[3].point.y);
	ori[2] = _mm_set_ps(r[0].point.z, r[1].point.z, r[2].point.z, r[3].point.z);

	__m128 tIn = _mm_mul_ps(_mm_sub_ps(nearBox4[0] - ori[0]), invD[0]);
	tIn = _mm_max_ps(tIn, _mm_mul_ps(_mm_sub_ps(nearBox4[1] - ori[1]), invD[1]));
	tIn = _mm_max_ps(tIn, _mm_mul_ps(_mm_sub_ps(nearBox4[2] - ori[2]), invD[2]));

	__m128 tOut = _mm_mul_ps(_mm_sub_ps(farBox4[0] - ori[0]), invD[0]);
	tOut = _mm_max_ps(tOut, _mm_mul_ps(_mm_sub_ps(farBox4[1] - ori[1]), invD[1]));
	tOut = _mm_max_ps(tOut, _mm_mul_ps(_mm_sub_ps(farBox4[2] - ori[2]), invD[2]));


	const float invD[3] = {1.0f / r.direction.x, 1.0f / r.direction.y, 1.0f / r.direction.z};
	float tIn  =            (sceneBox[nearFace4[0]][0] - r.point.x) * invD[0];
	tIn        = maxf(tIn,  (sceneBox[nearFace4[1]][1] - r.point.y) * invD[1]);
	tIn        = maxf(tIn,  (sceneBox[nearFace4[2]][2] - r.point.z) * invD[2]);
	float tOut  =             (sceneBox[!nearFace4[0]][0] - r.point.x) * invD[0];
	tOut        = maxf(tOut,  (sceneBox[!nearFace4[1]][1] - r.point.y) * invD[1]);
	tOut        = maxf(tOut,  (sceneBox[!nearFace4[2]][2] - r.point.z) * invD[2]);
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
	WSKDNode* pNode = kdInteriors;
	WTriangle* bestTriangle = NULL;
	if (beginNode != -1)
		pNode = kdLeafs + beginNode;
	while(pNode)
	{
		while (pNode->type != WSKDNode::KDN_LEAF)
		{	// �ڲ��ڵ�
			++numTraverseInterior_I;
			WSKDInterior* pInterior = (WSKDInterior*)pNode;
			char axis = pInterior->type;
			pNode = (pIn[axis] > pInterior->splitPlane) ? pInterior->child[1] : pInterior->child[0];
			if (pIn[axis] == pInterior->splitPlane)
				pNode = pInterior->child[nearFace4[axis]];
		}
		++numTraverseLeaf_I;
		WSKDLeaf* pLeaf = (WSKDLeaf*)pNode;
		WTriangle** tris = pLeaf->triangle;
		int nTris = pLeaf->nTriangles;
		while(nTris--)
		{
			WTriangle* pTri = *tris;
			__m128 pTri4 = _mm_set_ps1(*reinterpret_cast<float*>(&pTri));
			__m128 tmp14  = _mm_cmpeq_ps(pTri4, mailBox[0]);
			__m128 tmp24  = _mm_cmpeq_ps(pTri4, mailBox[1]);
			tmp14 = _mm_or_ps(tmp14, tmp24);
			if (!_mm_movemask_ps(tmp14))
			{
				++numTriangleIsect_I;
				float tIsect =  pTri->intersectTest(r);
				if (tIsect < tBest)
				{
					tBest = tIsect;
					bestTriangle = *tris;
				}
			}
			pMainBox[curMailBox] = pTri;
			curMailBox = (++curMailBox) & 0x7;
			tris++;
		}
		char exitFace = 0;
		tOut = (pLeaf->box[!nearFace4[0]][0] - r.point.x) * invD[0];
		float tTemp = (pLeaf->box[!nearFace4[1]][1] - r.point.y) * invD[1];
		if (tTemp < tOut){
			tOut = tTemp;	exitFace = 1;
		}
		tTemp = (pLeaf->box[!nearFace4[2]][2] - r.point.z) * invD[2];
		if (tTemp < tOut){
			tOut = tTemp;	exitFace = 2;
		}
		if (tBest <= tOut)
			break;					// �ҵ�����
		else
		{	// û�з���Ҫ��Ľ��㣬����ropeת����Ӧ�Ľڵ���
			pNode = pLeaf->ropes[!nearFace4[exitFace]][exitFace];
		}
		pIn[exitFace] = pLeaf->box[!nearFace4[exitFace]][exitFace];
		char other = otherAxis[exitFace][0];
		pIn[other] = r.point.v[other] + r.direction.v[other] * tOut;
		other = otherAxis[exitFace][1];
		pIn[other] = r.point.v[other] + r.direction.v[other] * tOut;
	}
	if (!bestTriangle)
		return false;
	if (endNode)
		*endNode = (pNode - kdLeafs) / sizeof(WSKDLeaf);
	bestTriangle->intersect(r,DG);
	return true;*/
}
void WSimpleKD::drawTriangles()
{
	WSKDLeaf* pNode;
	for (pNode = kdLeafs; pNode < kdLeafs + numLeafs; ++pNode)
	{
		WTriangle* pTri = *pNode->triangle;
		WTriangle* pEnd = pTri + pNode->nTriangles;
		for(; pTri < pEnd; ++pTri)
			pTri->draw();
	}
}

void WSimpleKD::buildExtendedRopes()
{
	WSKDLeaf* pEnd = kdLeafs + numLeafs;
	for (WSKDLeaf* pNode = kdLeafs; pNode < pEnd; ++pNode)
	{
		// ����ڵ�6�����Ӧ��rope
		for (char i = 0; i < 6; ++i)
		{
			char axis = i % 3;
			char isMax = i < 3;
			if (!pNode->ropes[isMax][axis])
				continue;
			if (pNode->ropes[isMax][axis]->type != WSKDNode::KDN_LEAF)
			{
				WSKDInterior* pI = (WSKDInterior*)pNode->ropes[isMax][axis];
				// һֱ���²���ֱ������Ҷ�ӽڵ�
				while(pI->type != WSKDNode::KDN_LEAF)
				{
					// �ж�Ҷ�ӽڵ��Ӧ����ֻ���Ǹ��ڵ������һ���ӽڵ��нӴ�
					if (pI->type == axis)
						pI = (WSKDInterior*)pI->child[!isMax];
					else if (pI->splitPlane < pNode->box[0][pI->type])
						pI = (WSKDInterior*)pI->child[1];
					else if (pI->splitPlane > pNode->box[1][pI->type])
						pI = (WSKDInterior*)pI->child[0];
					else
						break;
				}
				pNode->ropes[isMax][axis] = pI;
			}
		}
	}
}

void WSimpleKD::displayNodeInfo()
{
	int maxLeafTris = 0;
	for (WSKDLeaf* pLeaf = kdLeafs; pLeaf < kdLeafs + numLeafs; ++pLeaf)
		maxLeafTris = max(maxLeafTris, pLeaf->nTriangles);
	printf("number of interiors:%d\n"\
		   "number of leafs    :%d\n"\
		   "number of leaf tris:%d\n"\
		   "average leaf tris  :%f\n"\
		   "maximum leaf tris  :%d\n"\
		   "tree depth         :%d\n"\
		   , numInteriors, numLeafs, numLeafTriangles, (float)numLeafTriangles / numLeafs, maxLeafTris,treeDepth);
/*
	WSKDNode* stack[SKD_MAX_DEPTH];
	WSKDNode** top = stack;
	*top = kdInteriors;
	while(top >= stack)
	{
		WSKDNode* pNode = *(top--);
		if (pNode->type != WSKDNode::KDN_LEAF)
		{
			WSKDInterior* pInt = (WSKDInterior*)pNode;
			printf("type:%d split:%f\n", pInt->type, pInt->splitPlane);
			char child = 2;
			while (child--)
			{
				top++;
				*top = (WSKDInterior*)pInt->child[child];
			}
		}
		else
		{
			WSKDLeaf* pLeaf = (WSKDLeaf*)pNode;				
			printf("type:%d ", pLeaf->type);
			printf("box:(%f %f %f)(%f %f %f)\n", 
				pLeaf->box[0][0], pLeaf->box[0][1], pLeaf->box[0][2], 
				pLeaf->box[1][0], pLeaf->box[1][1], pLeaf->box[1][2]);
			WSKDNode** pN = pLeaf->ropes[0];
			printf("\tropes:");
			for (int i = 0; i < 6; i++)
			{
				if (pN[i])
				{
					int offset = pN[i]->type == WSKDNode::KDN_LEAF?
						(WSKDLeaf*)pN[i] - kdLeafs : (WSKDInterior*)pN[i] - kdInteriors;
					printf("%d ", offset);
				}
				else
					printf("null ");
			}
			printf("\n\ttriangles:");
			for (int i = 0; i < pLeaf->nTriangles; i++)
			{
				printf("%d ", pLeaf->triangle[i] - triangles);
			}
			printf("\n");
		}
	}*/
}

void WSimpleKD::buildCacheFriendlyNode()
{
	struct StackElement
	{
		WSKDInterior* pNode;
		int layer;
	};
	// ����ռ������кõĽڵ�
	WSKDInterior* tempInteriors = new WSKDInterior[numInteriors];
	deque<WSKDInterior*> stack0;
	stack0.push_back(kdInteriors);
	WSKDInterior* dstNode = tempInteriors;
	map<WSKDNode*, WSKDNode*> addressMap;			// ��ַ��Ӧ��

	while(stack0.size())
	{
		// ȡһ����������
		//StackElement stack1[MBVH_SUBTREE_DEPTH * 4];
		//StackElement* stack1Top = stack1;					// ջ��ָ��
		deque<StackElement>stack1;
		StackElement t;
		t.pNode = stack0.back();					// ���ڵ�����ջ
		t.layer = 0;								// ��¼�ڵ����
		stack1.push_back(t);
		stack0.pop_back();						

		while(stack1.size())
		{
			// pop and copy node
			StackElement top = stack1.front();
			WSKDInterior* pNode = top.pNode;
			int layer = top.layer;
			stack1.pop_front();
			memcpy((void*)dstNode, (void*)pNode, sizeof(WSKDInterior));
			// record address map
			addressMap[pNode] = dstNode;
			dstNode++;
			// process child node
			if (pNode->child[1]->type != WSKDNode::KDN_LEAF)
			{
				if (layer < SKD_SUBTREE_DEPTH)
				{	// �����ڲ��Ľڵ�
					top.pNode = (WSKDInterior*)pNode->child[1];
					top.layer = layer + 1;
					stack1.push_back(top);
				}
				else
				{	// �������²�Ľڵ�
					stack0.push_back((WSKDInterior*)pNode->child[1]);
				}
			}
			if (pNode->child[0]->type != WSKDNode::KDN_LEAF)
			{
				if (layer < SKD_SUBTREE_DEPTH)
				{	// �����ڲ��Ľڵ�
					top.pNode = (WSKDInterior*)pNode->child[0];
					top.layer = layer + 1;
					stack1.push_back(top);
				}
				else
				{	// �������²�Ľڵ�
					stack0.push_back((WSKDInterior*)pNode->child[0]);
				}
			}
		}

	}

	for(int i = 0; i < numInteriors; i++)
	{
		if (tempInteriors[i].child[0]->type != WSKDNode::KDN_LEAF)
			tempInteriors[i].child[0] = addressMap[tempInteriors[i].child[0]];
		if (tempInteriors[i].child[1]->type != WSKDNode::KDN_LEAF)
			tempInteriors[i].child[1] = addressMap[tempInteriors[i].child[1]];
	}
	delete[] kdInteriors;
	kdInteriors = tempInteriors;
}

void WSimpleKD::getGPUArray( WSKDNodeGPU*& kdNodesGPU, WSKDLeafBodyGPU*& kdLeafBodysGPU, int*& triArrayGPU, int& numNodesGPU, int& numLeafBodysGPU, int& numTriArrayGPU )
{
	if(numInteriors == 0 || numLeafs == 0)
		return;
	numNodesGPU     = numInteriors + numLeafs;
	numLeafBodysGPU = numLeafs;
	numTriArrayGPU  = numLeafTriangles + numLeafs;

	kdNodesGPU		= new WSKDNodeGPU[numNodesGPU];
	kdLeafBodysGPU  = new WSKDLeafBodyGPU[numLeafBodysGPU];
	triArrayGPU     = new int[numTriArrayGPU];

	int* interiorMap= new int[numInteriors];
	int* leafMap    = new int[numLeafs];    

	// ���Ȱ��������������˳������
	WSKDNode* nodeStack[SKD_MAX_DEPTH];
	WSKDNode**top = nodeStack;
	*top = kdInteriors;

	int dstNode = 0, dstTri = 0;
	while(top >= nodeStack)
	{
		WSKDNode* pNode = *(top--);
		// ��ջ��ȡ��һ���ڲ��ڵ㸴�Ƶ��ڲ��ڵ㣬�������ڲ��ڵ�ӳ���
		if (pNode->type != WSKDNode::KDN_LEAF)
		{
			WSKDInterior* pInt = (WSKDInterior*)pNode;
			kdNodesGPU[dstNode].splitPlane = pInt->splitPlane;		
			interiorMap[pInt - kdInteriors] = dstNode++;	// �����ڲ��ڵ�ӳ���
			top++; *top = pInt->child[1];	
			top++; *top = pInt->child[0];
		}
		else
		{
			WSKDLeaf* leaf = (WSKDLeaf*)pNode;
			// kdLeafBodysGPU�ϵ�λ��
			int offset = leaf - kdLeafs;
			kdNodesGPU[dstNode].nodeBodyID = offset;
			// �����������������ʼλ��
			kdNodesGPU[dstNode].triID      = dstTri | (WSKDNode::KDN_LEAF << 30);		
			leafMap[offset] = dstNode++;			// ����Ҷ�ڵ�ӳ���

			// ��������������
			triArrayGPU[dstTri++] = leaf->nTriangles;
			WTriangle** pTri = leaf->triangle;
			for (int ithTri = 0; ithTri < leaf->nTriangles; ++ithTri, ++dstTri,++pTri)
			{
				triArrayGPU[dstTri] = *pTri - triangles;
			}
		}
	}

	// ������ڲ��ڵ�� farNode ��Ա
	for (WSKDInterior* pNode = kdInteriors; pNode < kdInteriors + numInteriors; ++pNode)
	{
		// �����Զ�ڵ��ƫ����
		int nodeType = pNode->type;
		int farType  = pNode->child[1]->type;
		int farOffset = (farType != WSKDNode::KDN_LEAF) ? 
							interiorMap[(WSKDInterior*)(pNode->child[1]) - kdInteriors]:
							leafMap[(WSKDLeaf*)(pNode->child[1]) - kdLeafs];
		int dstID = interiorMap[pNode - kdInteriors];
		kdNodesGPU[dstID].farNode = (nodeType << 30) | farOffset;
	}

	// �����Ҷ�ڵ��壨WSKDLeafBodyGPU���� box �� rope
	for (WSKDLeaf* pNode = kdLeafs; pNode < kdLeafs + numLeafs; ++pNode)
	{
		int offset = pNode - kdLeafs;
		WSKDLeafBodyGPU* pDstLeafBody = kdLeafBodysGPU + offset;
		// ����Ҷ�ڵ��Χ��
		memcpy((void*)(pDstLeafBody), (void*)pNode->box[0], sizeof(float) * 6);

		// ���� ropes
		WSKDNode** pRope = pNode->ropes[0];
		for (char ithRope = 0; ithRope < 6; ++ithRope, ++pRope)
		{
			if (!(*pRope))
				pDstLeafBody->rope[ithRope] = 0xffffffff;		// ��ָ��
			else
			{
				int type = (*pRope)->type;
				int offset = type != WSKDNode::KDN_LEAF ?
					interiorMap[(WSKDInterior*)(*pRope) - kdInteriors] :
					leafMap[(WSKDLeaf*)(*pRope) - kdLeafs];
				pDstLeafBody->rope[ithRope] = offset;
			}
		}
	}
	delete[] interiorMap;
	delete[] leafMap;
}
