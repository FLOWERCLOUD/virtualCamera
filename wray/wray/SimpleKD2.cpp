#include "StdAfx.h"
#include "SimpleKD.h"
const char WSimpleKD2::otherAxis[3][2] = {{1,2},{0,2},{0,1}};
WSimpleKD2::WSimpleKD2(void)
{
	KT = 1;
	KI = 80;
	triangles = NULL;
	kdInteriors = NULL;
	kdLeafs = NULL;
	leafTriangles = NULL;
	numInteriors = numLeafBytes = treeDepth = totalTriangles = numLeafTriangles = 0;
	numInteriorsAllocated = numLeafBytesAllocated = numLeafTrianglesAllocated = 0;
	minBoxFactor = SKD_MIN_BOX_FACTOR;
}

WSimpleKD2::~WSimpleKD2(void)
{
	clearTree();
}

void WSimpleKD2::buildTree( WScene&scene )
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
	kdInteriors = new WSKDInterior2[approNodes];	// �ڲ��ڵ�����
	kdLeafs     = (WSKDLeaf2*)malloc(approNodes);		// Ҷ�ڵ�����
	leafTriangles = new WTriangle*[approNodes];	// Ҷ�ڵ�������ָ������
	numInteriorsAllocated = numLeafBytesAllocated = numLeafTrianglesAllocated = approNodes;

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
	WSKDInterior2* pInteriorEnd = kdInteriors + numInteriors;

	// �����ʵ�ʵ��ӽڵ�ָ���ַ
	for (WSKDInterior2* pInterior = kdInteriors; pInterior < pInteriorEnd; ++pInterior)
	{
		// ���ӽڵ�
		unsigned *child = (unsigned*)&pInterior->child[0];
		unsigned isInterior = *child & 0x80000000;
		*child &= 0x3fffffff;
		*child = isInterior ? (unsigned)(kdInteriors + *child) : ((unsigned)kdLeafs + *child);
		// ���ӽڵ�
		child++;
		isInterior = *child & 0x80000000;
		*child &= 0x3fffffff;
		*child = isInterior ? (unsigned)(kdInteriors + *child) : ((unsigned)kdLeafs + *child);
	}
	buildCacheFriendlyNode();
	buildBasicRopes(kdInteriors, NULL, NULL, NULL, NULL, NULL, NULL);
	buildExtendedRopes();
}
WSKDInterior2* WSimpleKD2::requestInterior()
{
	if (numInteriors < numInteriorsAllocated)
		return kdInteriors + (numInteriors++);
	// �ڲ��ڵ�ռ䲻�������·��䣬�����������ö�Ӧ��ָ��
	numInteriorsAllocated *= 2;
	WSKDInterior2* tempInterior = new WSKDInterior2[numInteriorsAllocated];
	memcpy((void*)tempInterior, (void*)kdInteriors, sizeof(WSKDInterior2) * numInteriors);
	delete[] kdInteriors;
	kdInteriors = tempInterior;
	return kdInteriors + (numInteriors++);
}
WSKDLeaf2* WSimpleKD2::requestLeaf(int nTri)
{
	//void* leaf = malloc(sizeof(WSKDLeaf2) + sizeof(TriAccel) * nTri);
	int leafSize = sizeof(WSKDLeaf2) + sizeof(TriAccel) * nTri;
	if (numLeafBytes + leafSize >= numLeafBytesAllocated)
	{
		numLeafBytesAllocated *= 2;
		WSKDLeaf2* tempLeaf = (WSKDLeaf2*)malloc(numLeafBytesAllocated);
		memcpy((void*)tempLeaf, (void*)kdLeafs, numLeafBytes);
		free(kdLeafs);
		kdLeafs = tempLeaf;
	}
	WSKDLeaf2* p = (WSKDLeaf2*)((char*)kdLeafs + numLeafBytes);
	numLeafBytes += leafSize;
	return p;
}
WTriangle** WSimpleKD2::requestLeafTriangles(int nTri)
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
void WSimpleKD2::buildTreeKernel( vector<WBoundingEdge>&edges, 
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
			{							// �������Ϊ�������ߵı������
				PL = (p->t - nodeBox[0][axis]) / boxSize[axis];
				PR = (nodeBox[1][axis] - p->t) / boxSize[axis];
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
		kdInteriors[parentIndex].child[branch] = (WSKDNode2*)(numLeafBytes | SKD_LEAF_MASK);

		// ͳ��Ҷ�ڵ�������
		set<int> triangleSet;
		for (vector<WBoundingEdge>::iterator pE = edges.begin(); pE != edges.end(); ++pE)
			triangleSet.insert(pE->triangleID);

		// ����һ��Ҷ�ڵ㲢��ֵ������ռ䲻���������·���ռ�
		WSKDLeaf2 * leaf = requestLeaf(triangleSet.size());
		leaf->type = WSKDNode2::KDN_LEAF;
		memcpy((void*)leaf->box, (void*)nodeBox, sizeof(float) * 6);
		leaf->nTriangles = triangleSet.size();

		// �����ٽṹ
		int i = 0;
		leaf->triInfo = (TriAccel*)((int*)(&leaf->triInfo) + 1);
		for (set<int>::iterator pS = triangleSet.begin(); pS != triangleSet.end(); ++pS, ++i)
			leaf->triInfo[i] = triangles[*pS].getAccelerateData();
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
	WSKDInterior2* pNode = requestInterior();
	pNode->type = WSKDNode2::NodeType(bestAxis);
	pNode->splitPlane = bestT;

	kdInteriors[parentIndex].child[branch] = (WSKDNode2*)(curIndex | SKD_INTERIOR_MASK);
	buildTreeKernel(edges, bestLTri, depth + 1, LNodeBox, triangleMap, curIndex, 0);

	edges.resize(oriEdgesR.size() + tempEdgesR.size());
	merge(oriEdgesR.begin(), oriEdgesR.end(), tempEdgesR.begin(), tempEdgesR.end(), edges.begin());
	tempEdgesR.clear();		oriEdgesR.clear();

	buildTreeKernel(edges, bestRTri, depth + 1, RNodeBox, triangleMap, curIndex, 1);
}

void WSimpleKD2::clearTree()
{
	triangles = NULL;
	delete []kdInteriors;
	delete []kdLeafs;
	delete []leafTriangles;
	kdInteriors = NULL;
	kdLeafs = NULL;
	leafTriangles = NULL;
	numInteriors = numLeafBytes = treeDepth = totalTriangles = numLeafTriangles = 0;
	numInteriorsAllocated = numLeafBytesAllocated = numLeafTrianglesAllocated = 0;
}

void WSimpleKD2::drawTree( unsigned int nthBox/*=0*/, float R /*= 0.7*/, float G /*= 0.7*/, float B /*= 0.7*/ )
{
//	cout<<"\nnode size:"<<nodes.size()<<endl;
	glColor3f(R, G, B);
//	drawTreeRecursive(kdInteriors, sceneBox);
	/*
	for (WSKDLeaf2** pLeaf = kdLeafs ;pLeaf < kdLeafs + numLeafBytes; ++pLeaf)
	{
		WBoundingBox box((*pLeaf)->box);
		box.draw();
	}*/
}

void WSimpleKD2::drawTreeRecursive( WSKDNode2* node, const WBoundingBox& box )
{
	if (!numInteriors)
		return;
	box.draw();
	WBoundingBox leftBox, rightBox;
	leftBox = rightBox = box;
	switch(node->type)
	{
	case WSKDNode2::KDN_LEAF:
		return;
	case WSKDNode2::KDN_XSPLIT:
		leftBox.pMax.x = rightBox.pMin.x = ((WSKDInterior2*)node)->splitPlane;
		break;
	case WSKDNode2::KDN_YSPLIT:
		leftBox.pMax.y = rightBox.pMin.y = ((WSKDInterior2*)node)->splitPlane;
		break;
	case WSKDNode2::KDN_ZSPLIT:
		leftBox.pMax.z = rightBox.pMin.z = ((WSKDInterior2*)node)->splitPlane;
		break;
	default:
		return;
	}
	drawTreeRecursive(((WSKDInterior2*)node)->child[0], leftBox);
	drawTreeRecursive(((WSKDInterior2*)node)->child[1], rightBox);
}


void WSimpleKD2::buildBasicRopes(WSKDNode2* pNode, 
							   WSKDNode2* ropeX_P, WSKDNode2* ropeX_N,
							   WSKDNode2* ropeY_P, WSKDNode2* ropeY_N,
							   WSKDNode2* ropeZ_P, WSKDNode2* ropeZ_N)
{
	if (pNode->type == WSKDNode2::KDN_LEAF)
	{
		WSKDLeaf2* pLeaf = (WSKDLeaf2*)pNode;
		pLeaf->ropes[1][0] = ropeX_P;
		pLeaf->ropes[0][0] = ropeX_N;
		pLeaf->ropes[1][1] = ropeY_P;
		pLeaf->ropes[0][1] = ropeY_N;
		pLeaf->ropes[1][2] = ropeZ_P;
		pLeaf->ropes[0][2] = ropeZ_N;
	} 
	else if(pNode->type == WSKDLeaf2::KDN_XSPLIT)
	{
		WSKDInterior2* pInterior = (WSKDInterior2*)pNode;
		buildBasicRopes(pInterior->child[0],
			pInterior->child[1], ropeX_N, ropeY_P, ropeY_N, ropeZ_P, ropeZ_N);
		buildBasicRopes(pInterior->child[1],
			ropeX_P, pInterior->child[0], ropeY_P, ropeY_N, ropeZ_P, ropeZ_N);
	}
	else if(pNode->type == WSKDLeaf2::KDN_YSPLIT)
	{
		WSKDInterior2* pInterior = (WSKDInterior2*)pNode;
		buildBasicRopes(pInterior->child[0],
			ropeX_P, ropeX_N, pInterior->child[1], ropeY_N, ropeZ_P, ropeZ_N);
		buildBasicRopes(pInterior->child[1],
			ropeX_P, ropeX_N, ropeY_P, pInterior->child[0], ropeZ_P, ropeZ_N);
	}
	else if(pNode->type == WSKDLeaf2::KDN_ZSPLIT)
	{
		WSKDInterior2* pInterior = (WSKDInterior2*)pNode;
		buildBasicRopes(pInterior->child[0],
			ropeX_P, ropeX_N, ropeY_P, ropeY_N, pInterior->child[1], ropeZ_N);
		buildBasicRopes(pInterior->child[1],
			ropeX_P, ropeX_N, ropeY_P, ropeY_N, ropeZ_P, pInterior->child[0]);
	}
}


bool WSimpleKD2::WBoundingEdge::operator<( const WBoundingEdge&e ) const
{
	//��ͬλ��, tֵ��С�Ľ�ǰ
	//���� t ���ʱ��ȷ�� planar���͵���ǰ��start���͵Ľ�ǰ�� end ���͵ĽϺ�
	//������ȷ���ڱ�������ε�ʱ��������
	//һ�������ε�start�ߣ�������end��
	return (axis < e.axis) || (axis == e.axis) && ((t < e.t) || ((t == e.t) && (type < e.type)));
}

bool WSimpleKD2::isIntersect( WRay& r, int beginNode)	
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
	mailBox[0] = _mm_castsi128_ps(_mm_set1_epi32(-1));
	mailBox[1] = _mm_castsi128_ps(_mm_set1_epi32(-1));
	int* pMainBox = (int*)&mailBox[0];
	char curMailBox = 0;

	tIn = maxf(tIn, 0.0f);
	float pIn[3] = {r.point.x + r.direction.x * tIn,r.point.y + r.direction.y * tIn,r.point.z + r.direction.z * tIn};
	float tBest = M_INF_BIG;
	WSKDNode2* pNode = kdInteriors;
	if (beginNode != -1)
		pNode = (WSKDLeaf2*)beginNode;
	while(pNode)
	{
		while (pNode->type != WSKDNode::KDN_LEAF)
		{	// �ڲ��ڵ�
			++numTraverseInterior_IT;
			WSKDInterior2* pInterior = (WSKDInterior2*)pNode;
			char axis = pInterior->type;
			pNode = (pIn[axis] > pInterior->splitPlane) ? pInterior->child[1] : pInterior->child[0];
			if (pIn[axis] == pInterior->splitPlane)
				pNode = pInterior->child[nearFace[axis]];
		}
		++numTraverseLeaf_IT;
		WSKDLeaf2* pLeaf = (WSKDLeaf2*)pNode;
		TriAccel* pTri = (TriAccel*)(pLeaf + 1);
		int nTris = pLeaf->nTriangles;
		while(nTris--)
		{
			int triID = pTri->ci & TRIACCEL_TRIID_MASK;
#ifdef SKD_MAILBOX
			__m128 pTri4 = _mm_set_ps1(*reinterpret_cast<float*>(&triID));
			__m128 tmp14  = _mm_cmpeq_ps(pTri4, mailBox[0]);
			__m128 tmp24  = _mm_cmpeq_ps(pTri4, mailBox[1]);
			tmp14 = _mm_or_ps(tmp14, tmp24);
			if (!_mm_movemask_ps(tmp14))
			{
#endif
				++numTriangleIsect_I;
				float tIsect =  pTri->intersectTest(r);
				if (tIsect < tBest)
					return true;
#ifdef SKD_MAILBOX
			}
			pMainBox[curMailBox] = triID;
			curMailBox = (++curMailBox) & 0x7;
#endif
			pTri++;
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
bool WSimpleKD2::intersect( WRay& r,WDifferentialGeometry& DG, 
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
	mailBox[0] = _mm_castsi128_ps(_mm_set1_epi32(-1));
	mailBox[1] = _mm_castsi128_ps(_mm_set1_epi32(-1));
	int* pMainBox = (int*)&mailBox[0];
	char curMailBox = 0;

	tIn = maxf(tIn, 0.0f);
	float pIn[3] = {r.point.x + r.direction.x * tIn,
		            r.point.y + r.direction.y * tIn,
					r.point.z + r.direction.z * tIn};
	float tBest = FLT_MAX;
	WSKDNode2* pNode = kdInteriors;
	WTriangle* bestTriangle = NULL;
	if (beginNode != -1)
		pNode = (WSKDLeaf2*)beginNode;
	while(pNode)
	{
		while (pNode->type - WSKDNode2::KDN_LEAF)
		{	// �ڲ��ڵ�
			++numTraverseInterior_I;
			WSKDInterior2* pInterior = (WSKDInterior2*)pNode;
			const char axis = pInterior->type;
			const float t = pIn[axis];
			pNode = (t > pInterior->splitPlane) ? pInterior->child[1] : pInterior->child[0];
			if (t == pInterior->splitPlane)
				pNode = pInterior->child[nearFace[axis]];
		}
		++numTraverseLeaf_I;
		WSKDLeaf2* pLeaf = (WSKDLeaf2*)pNode;
		TriAccel* pTri = (TriAccel*)(pLeaf + 1);
		int nTris = pLeaf->nTriangles;
		while(nTris--)
		{
			// ��õ�ǰ���������
			int triID = pTri->ci & TRIACCEL_TRIID_MASK;
			// ���mailbox���Ƿ�����ͬ�����
#ifdef SKD_MAILBOX
			__m128 pTri4 = _mm_set_ps1(*reinterpret_cast<float*>(&triID));
			__m128 tmp14  = _mm_cmpeq_ps(pTri4, mailBox[0]);
			__m128 tmp24  = _mm_cmpeq_ps(pTri4, mailBox[1]);
			tmp14 = _mm_or_ps(tmp14, tmp24);
			// ���û�У��������ν�����
			if (!_mm_movemask_ps(tmp14))
			{
#endif
				++numTriangleIsect_I;
				float tIsect =  pTri->intersectTest(r);
				if (tIsect < tBest)
				{
					tBest = tIsect;
					bestTriangle = triangles + triID;
				}
#ifdef SKD_MAILBOX
			}
			pMainBox[curMailBox] = triID;
			curMailBox = (++curMailBox) & 0x7;
#endif
			++pTri;
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
		*endNode = (int)pNode;
	bestTriangle->intersect(r,DG);
	return true;
}


bool WSimpleKD2::intersect4(WRay r[4], WDifferentialGeometry DG[4], int endNode[4], int beginNode[4])
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
void WSimpleKD2::drawTriangles()
{/*
	WSKDLeaf** pNode;
	for (pNode = kdLeafs; pNode < kdLeafs + numLeafs; ++pNode)
	{
		WTriangle* pTri = *pNode->triangle;
		WTriangle* pEnd = pTri + pNode->nTriangles;
		for(; pTri < pEnd; ++pTri)
			pTri->draw();
	}*/
}

void WSimpleKD2::buildExtendedRopes()
{
	for (char* pRaw = (char*)kdLeafs; pRaw < (char*)kdLeafs + numLeafBytes;)
	{
		WSKDLeaf2* pNode = (WSKDLeaf2*)pRaw;
		for (char i = 0; i < 6; ++i)
		{
			char axis = i % 3;
			char isMax = i < 3;
			if (!pNode->ropes[isMax][axis])
				continue;
			if (pNode->ropes[isMax][axis]->type != WSKDNode2::KDN_LEAF)
			{
				WSKDInterior2* pI = (WSKDInterior2*)pNode->ropes[isMax][axis];
				while(pI->type != WSKDNode2::KDN_LEAF)
				{
					if (pI->type == axis)
						pI = (WSKDInterior2*)pI->child[!isMax];
					else if (pI->splitPlane < pNode->box[0][pI->type])
						pI = (WSKDInterior2*)pI->child[1];
					else if (pI->splitPlane > pNode->box[1][pI->type])
						pI = (WSKDInterior2*)pI->child[0];
					else
						break;
				}
				pNode->ropes[isMax][axis] = pI;
			}
		}
		pRaw += sizeof(WSKDLeaf2) + sizeof(TriAccel) * pNode->nTriangles;
	}
}

void WSimpleKD2::displayNodeInfo()
{
	int maxLeafTris = 0;/*
	for (WSKDLeaf2** pLeaf = kdLeafs; pLeaf < kdLeafs + numLeafBytes; ++pLeaf)
		maxLeafTris = max(maxLeafTris, (*pLeaf)->nTriangles);
	printf("number of interiors:%d\n"\
		   "number of leafs    :%d\n"\
		   "number of leaf tris:%d\n"\
		   "average leaf tris  :%f\n"\
		   "maximum leaf tris  :%d\n"\
		   "tree depth         :%d\n"\
		   , numInteriors, numLeafBytes, numLeafTriangles, (float)numLeafTriangles / numLeafBytes, maxLeafTris,treeDepth);*/
}

void WSimpleKD2::buildCacheFriendlyNode()
{
	struct StackElement
	{
		WSKDInterior2* pNode;
		int layer;
	};
	// ����ռ������кõĽڵ�
	WSKDInterior2* tempInteriors = new WSKDInterior2[numInteriors];
	deque<WSKDInterior2*> stack0;
	stack0.push_back(kdInteriors);
	WSKDInterior2* dstNode = tempInteriors;
	map<WSKDNode2*, WSKDNode2*> addressMap;			// ��ַ��Ӧ��

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
			WSKDInterior2* pNode = top.pNode;
			int layer = top.layer;
			stack1.pop_front();
			memcpy((void*)dstNode, (void*)pNode, sizeof(WSKDInterior2));
			// record address map
			addressMap[pNode] = dstNode;
			dstNode++;
			// process child node
			if (pNode->child[0]->type != WSKDNode2::KDN_LEAF)
			{
				if (layer < SKD_SUBTREE_DEPTH)
				{	// �����ڲ��Ľڵ�
					top.pNode = (WSKDInterior2*)pNode->child[0];
					top.layer = layer + 1;
					stack1.push_back(top);
				}
				else
				{	// �������²�Ľڵ�
					stack0.push_back((WSKDInterior2*)pNode->child[0]);
				}
			}
			if (pNode->child[1]->type != WSKDNode2::KDN_LEAF)
			{
				if (layer < SKD_SUBTREE_DEPTH)
				{	// �����ڲ��Ľڵ�
					top.pNode = (WSKDInterior2*)pNode->child[1];
					top.layer = layer + 1;
					stack1.push_back(top);
				}
				else
				{	// �������²�Ľڵ�
					stack0.push_back((WSKDInterior2*)pNode->child[1]);
				}
			}
		}

	}

	for(int i = 0; i < numInteriors; i++)
	{
		if (tempInteriors[i].child[0]->type != WSKDNode2::KDN_LEAF)
			tempInteriors[i].child[0] = addressMap[tempInteriors[i].child[0]];
		if (tempInteriors[i].child[1]->type != WSKDNode2::KDN_LEAF)
			tempInteriors[i].child[1] = addressMap[tempInteriors[i].child[1]];
	}
	delete[] kdInteriors;
	kdInteriors = tempInteriors;
}