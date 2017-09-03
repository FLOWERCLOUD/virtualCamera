#include "StdAfx.h"
#include "KDTree.h"

WKDTree::WKDTree(void)
{
	nMailBoxes=0;
	nAllocatedNodes=128;
	nUsedNodes=0;
	currRayID=0;
	maxDepth=15;
	maxSubPsPerNode=3;
	traversalCost=10.0f;
	isectCost=1.0f;
	nodes=NULL;
	nodeBoxes=NULL;
	mailBoxes=NULL;
}

WKDTree::~WKDTree(void)
{
	clearTree();
}
void WKDTree::setNodeAttr(
				 unsigned int imaxDepth,
				 unsigned int imaxSubPsPerNode,
				 float iisectCost,
				 float itraversalCost
				 )
{
	if(imaxDepth>1)maxDepth=imaxDepth;
	if(imaxSubPsPerNode>1)maxSubPsPerNode=imaxSubPsPerNode;
	if(iisectCost>0)isectCost=iisectCost;
	if(itraversalCost>0)traversalCost=itraversalCost;
	return;
}

void WKDTree::clearTree()
{
	for(unsigned int nthNode=0;nthNode<nUsedNodes;nthNode++)
	{
		if(nodes[nthNode].type==WKDNode::KDN_LEAF)
		{
			//ɾ��ÿ���ڵ��mailbox��������
			delete[]nodes[nthNode].mailBoxIndices;
		}
	}
	delete []nodes;
	delete []mailBoxes;
	delete []nodeBoxes;
	nodes=NULL;
	mailBoxes=NULL;
	nodeBoxes=NULL;
	nUsedNodes=0;
	currRayID=0;
}
void WKDTree::cleanMailBoxes()
{
	WMailBox*p=mailBoxes;
	for(unsigned int i=0;i<nMailBoxes;i++)
	{
		p->rayID=0;
		p++;
	}
}
void WKDTree::buildMailBoxes(WScene&scene)
{
	nMailBoxes=scene.getSubPrimNum();
	mailBoxes=new WMailBox[nMailBoxes];
	WPrimitive*prims;
	unsigned int nPrims;
	WSubPrimitive*subPs;
	unsigned int nSubPs;
	scene.getPrimitives(prims,nPrims);
	unsigned int currBoxID=0;//��ǰmailbox���±�
	for(unsigned int i=0;i<nPrims;i++)
	{
		prims[i].getSubPrimitives(subPs,nSubPs);
		for(unsigned int j=0;j<nSubPs;j++)
		{
			mailBoxes[currBoxID].pSubP=subPs;
			subPs++;
			mailBoxes[currBoxID].rayID=0;
			currBoxID++;
		}
	}
}
void WKDTree::drawMailBoxes(	
						   float R,
						   float G,
						   float B)
{
	glColor3f(R,G,B);
	for(unsigned int i=0;i<nMailBoxes;i++)
	{
		mailBoxes[i].pSubP->box.draw();//������Χ��
	}
	//cout<<nMailBoxes;
}
void WKDTree::makeLeaf(unsigned int nMBoxes,
					  const vector<int>&mBoxIndices,
					  vector<WKDNode>&vNodes)
{
	WKDNode node;
	node.nMailBoxes=nMBoxes;
	node.mailBoxIndices=new int[nMBoxes];
	for(unsigned int i=0;i<nMBoxes;i++)
		node.mailBoxIndices[i]=mBoxIndices[i];
	node.type=WKDNode::KDN_LEAF;
	vNodes.push_back(node);
	/*		cout<<"return in depth"<<depth<<endl;*/
	return;
}
void WKDTree::updateProcess(const float &depth)
{
	float newProcess=process+pow(2.0f,-1.0f*depth);
	if(int(process*20)!=int(newProcess*20))
		cout<<"tree building process:"<<int(newProcess*100)<<'%'<<endl;
	process=newProcess;
	return ;
}
void WKDTree::buildTreeCore(
						   unsigned int depth,
						   const vector<int>&mBoxIndices,
						   const WBoundingBox&bBox,
						   vector<WKDNode>&vNodes,
						   vector<WBoundingBox>&vBoxes)
{
	unsigned int nMBoxes;
	WKDNode*currNode;
	WBoundingBox*currBBox;
	nMBoxes=mBoxIndices.size();

	//Ԥ���ռ�
	if(nUsedNodes==nAllocatedNodes)
	{	
		nAllocatedNodes*=2;
		vNodes.reserve(nAllocatedNodes);
		vBoxes.reserve(nAllocatedNodes);

	}
	nUsedNodes++;
	//��õ�ǰ�Ľڵ�Ͷ�Ӧ��BoundingBox
	currNode=new WKDNode;
	currBBox=new WBoundingBox;
	*currBBox=bBox;
	vBoxes.push_back(*currBBox);

// 	cout<<"\n##############depth begin##############"<<depth<<endl;
//  	cout<<"nUsedNodes"<<nUsedNodes<<endl;
//  	cout<<"\nmBoxIndices"<<endl;
// 	for(unsigned int i=0;i<nMBoxes;i++)
// 		cout<<mBoxIndices[i]<<ends;
// 	cout<<endl;

	//�����������½�Ҷ�ڵ�
	if(depth==maxDepth||nMBoxes<=maxSubPsPerNode||!nMBoxes)
	{
		//cout<<"depth"<<depth<<endl;
		makeLeaf(nMBoxes,mBoxIndices,vNodes);
		updateProcess(depth);
/*		cout<<"return in depth"<<depth<<endl;*/
		return;
	}
	//�ҳ����귶Χ��������ָ�
	int splitAxis=currBBox->maxAxis();
// 	cout<<"\nsplit Axis"<<splitAxis<<endl;
// 
// 	cout<<"build BoundingEdge begin"<<endl;
	vector<BoundingEdge>edges;	
	vector<BoundingEdge>::iterator pEdge;
	//Ԥ���ռ䣬boundingEdge��������BoundingBox������
//	cout<<"nMboxes"<<nMBoxes<<endl;
// 	edges.reserve(nMBoxes*2);
	//edges.clear();
 	edges.resize(nMBoxes*2);
	//��ʼ��boundingEdge
/*	cout<<"resized"<<endl;*/
	pEdge=edges.begin();
/*	cout<<"beginloop"<<endl;*/
	for(unsigned int i=0;
		i<nMBoxes;i++,pEdge++)
	{
		int ithMailBox=mBoxIndices[i];
/*		cout<<ithMailBox<<ends;*/
//		BoundingEdge tempEdge;
		switch (splitAxis)
		{
		case 0:
			pEdge->t=mailBoxes[ithMailBox].pSubP->box.pMin.x;
			pEdge->type=BoundingEdge::BE_START;
			pEdge->mailBoxNum=ithMailBox;

			pEdge++;
			pEdge->t=mailBoxes[ithMailBox].pSubP->box.pMax.x;
			pEdge->type=BoundingEdge::BE_END;
			pEdge->mailBoxNum=ithMailBox;

// 			tempEdge.t=mailBoxes[ithMailBox].pSubP->box.pMin.x;
// 			tempEdge.type=BoundingEdge::BE_START;
// 			tempEdge.mailBoxNum=i;
// 			
// 			edges.push_back(tempEdge);
// 			tempEdge.t=mailBoxes[ithMailBox].pSubP->box.pMax.x;
// 			tempEdge.type=BoundingEdge::BE_END;
// 			tempEdge.mailBoxNum=i;
// 			edges.push_back(tempEdge);
			break;
		case 1:
			pEdge->t=mailBoxes[ithMailBox].pSubP->box.pMin.y;
			pEdge->type=BoundingEdge::BE_START;
			pEdge->mailBoxNum=ithMailBox;

			pEdge++;
			pEdge->t=mailBoxes[ithMailBox].pSubP->box.pMax.y;
			pEdge->type=BoundingEdge::BE_END;
			pEdge->mailBoxNum=ithMailBox;

// 			tempEdge.t=mailBoxes[ithMailBox].pSubP->box.pMin.y;
// 			tempEdge.type=BoundingEdge::BE_START;
// 			tempEdge.mailBoxNum=i;
// 			edges.push_back(tempEdge);
// 			tempEdge.t=mailBoxes[ithMailBox].pSubP->box.pMax.y;
// 			tempEdge.type=BoundingEdge::BE_END;
// 			tempEdge.mailBoxNum=i;
// 			edges.push_back(tempEdge);
			break;
		case 2:
			pEdge->t=mailBoxes[ithMailBox].pSubP->box.pMin.z;
			pEdge->type=BoundingEdge::BE_START;
			pEdge->mailBoxNum=ithMailBox;

			pEdge++;
			pEdge->t=mailBoxes[ithMailBox].pSubP->box.pMax.z;
			pEdge->type=BoundingEdge::BE_END;
			pEdge->mailBoxNum=ithMailBox;

// 			tempEdge.t=mailBoxes[ithMailBox].pSubP->box.pMin.z;
// 			tempEdge.type=BoundingEdge::BE_START;
// 			tempEdge.mailBoxNum=i;
// 			edges.push_back(tempEdge);
// 			tempEdge.t=mailBoxes[ithMailBox].pSubP->box.pMax.z;
// 			tempEdge.type=BoundingEdge::BE_END;
// 			tempEdge.mailBoxNum=i;
// 			edges.push_back(tempEdge);
		}

	}
/*	cout<<"build BoundingEdge end"<<endl;*/
	//���������С��BoundingEdge��������
	stable_sort(edges.begin(),edges.end());

//   	cout<<"sorted"<<endl;
//  	for(pEdge=edges.begin();pEdge!=edges.end();pEdge++)
//  		cout<<pEdge->mailBoxNum<<ends;
//  	cout<<endl;

	//�ҳ�bBox��Ӧ��������귶Χ
	float bBoxMin,bBoxMax,invBBoxDelta;
	switch (splitAxis)
	{
	case 0:
		bBoxMin=bBox.pMin.x;
		bBoxMax=bBox.pMax.x;
		break;
	case 1:
		bBoxMin=bBox.pMin.y;
		bBoxMax=bBox.pMax.y;
		break;
	case 2:
		bBoxMin=bBox.pMin.z;
		bBoxMax=bBox.pMax.z;
	}
	invBBoxDelta=1.0f/(bBoxMax-bBoxMin);

/*	cout<<"compute best position begin"<<endl;*/

	//ѡ����ѻ���
	//���������������������ӵĲ���
	float oldCost=isectCost*nMBoxes;
	float bestCost=M_INF_BIG;
	float cost;
	float pisect;//�ཻ����
	float bestT=bBoxMin;
	unsigned int nBelow=0;
	unsigned int nAbove=nMBoxes;
	unsigned int bestBelow=0,bestAbove=nAbove;
//	unsigned int bestOffset=0;
	float isEmpty;
//	int nthEdge;//��¼��ѻ��ֶ�Ӧ��BoundingEdge
/*	cout<<"edges"<<endl;*/
	for(pEdge=edges.begin()/*,nthEdge=0*/;
		pEdge!=edges.end();pEdge++/*,nthEdge++*/)
	{
		//cout<<nthEdge<<endl;
		float edgeT=pEdge->t;
/*		cout<<pEdge->mailBoxNum<<ends;*/
		if(edgeT>bBoxMin+1e-5f
			&& edgeT<bBoxMax-1e-5f)//ȷ�����������Ϊ0�Ľڵ�
		{ 

			pisect=(edgeT-bBoxMin)*invBBoxDelta;
			if(nBelow==0)//��һ����
				isEmpty=pisect;
			else if(nAbove==1)//���һ����
				isEmpty=1-pisect;
			else
				isEmpty=0;
			//cout<<isEmpty<<ends;			
			cost=traversalCost+
				isectCost*(1-isEmpty)*
				(pisect*nBelow+(1-pisect)*nAbove);
// 			cout<<" pisect "<<pisect
// 				<<" cost "<<cost
// 				<<" bestCost "<<bestCost
// 				<<" bestBelow "<<bestBelow
// 				<<" bestAbove "<<bestAbove<<endl;
			if(cost<bestCost)
			{
				bestCost=cost;
//				bestOffset=nthEdge;
				bestBelow=nBelow;
				bestAbove=nAbove;
				bestT=edgeT;
			}
		}
		if(pEdge->type==BoundingEdge::BE_START)
		{
			nBelow++;
		}
		else //Ҳ����BoundingEdge::BE_END
		{
			nAbove--;
		}
	}
// 	cout<<"bestT"<<bestT<<endl;
// 	cout<<"best below"<<bestBelow<<endl;
// 	cout<<"best above"<<bestAbove<<endl;
// 	cout<<"bestCost"<<bestCost<<"oldCost"<<oldCost<<endl;
	//���û�к��ʵķָ���ڵ�ǰλ���½�Ҷ�ڵ�
	if(bestCost>oldCost*2)
	{
		makeLeaf(nMBoxes,mBoxIndices,vNodes);
		updateProcess(depth);
		return;
	}


// 	cout<<endl;
// 	cout<<"split mailboxes begin"<<endl;
// 	for(pEdge=edges.begin();pEdge!=edges.end();pEdge++)
// 		cout<<pEdge->mailBoxNum<<ends;

	//�������mailBox�ֱ�����Ӧ��mailBox����
/*
	int*belowMBoxIndices,*aboveMBoxIndices;
	if(bestBelow!=0)
		belowMBoxIndices=new int[bestBelow];
	else
		belowMBoxIndices=NULL;
	if(bestAbove!=0)
		aboveMBoxIndices=new int[bestAbove];
	else
		aboveMBoxIndices=NULL;
	unsigned int belowIndex=0,aboveIndex=0;
	for(pEdge=edges.begin();
		pEdge!=edges.end();pEdge++)
	{
		if(pEdge->type==BoundingEdge::BE_START
			&&pEdge->t<bestT)
		{
			belowMBoxIndices[belowIndex]=pEdge->mailBoxNum;
			belowIndex++;
		}
		if(pEdge->type==BoundingEdge::BE_END
			&&pEdge->t>=bestT)
		{
			aboveMBoxIndices[aboveIndex]=pEdge->mailBoxNum;
			aboveIndex++;
		}

	}*/

	vector<int>belowMBIndices;
	vector<int>aboveMBIndices;
	vector<int>::iterator pi;
	for(pEdge=edges.begin();
		pEdge!=edges.end();pEdge++)
	{
/*		cout<<pEdge->mailBoxNum<<endl;*/
		if(pEdge->type==BoundingEdge::BE_START
			&&pEdge->t<bestT)
		{
// 			cout<<"below"<<ends;
// 			cout<<pEdge->mailBoxNum<<endl;
			belowMBIndices.push_back(pEdge->mailBoxNum);
		}
		if(pEdge->type==BoundingEdge::BE_END
			&&pEdge->t>=bestT)
		{
// 			cout<<"above"<<ends;
// 			cout<<pEdge->mailBoxNum<<endl;
			aboveMBIndices.push_back(pEdge->mailBoxNum);
		}

	}
// 	cout<<"below"<<endl;
// 	for(pi=belowMBIndices.begin();
// 		pi!=belowMBIndices.end();pi++)
// 		cout<<*pi<<ends;
// 	cout<<endl;
// 	cout<<"above"<<endl;
// 	for(pi=aboveMBIndices.begin();
// 		pi!=aboveMBIndices.end();pi++)
// 		cout<<*pi<<ends;
// 	cout<<endl;




	/*
	switch (splitAxis)
	{
	case 0:	
		for(unsigned int i=0;i<nMBoxes;i++)
		{
			if(mailBoxes[mBoxIndices[i]].pSubP->box.pMin.x<bestT)
			{
				belowMBoxIndices[belowIndex]=mBoxIndices[i];
				belowIndex++;
			}
			if(mailBoxes[mBoxIndices[i]].pSubP->box.pMax.x>=bestT)
			{
				//cout<<"above"<<endl;
				aboveMBoxIndices[aboveIndex]=mBoxIndices[i];
				aboveIndex++;
			}
		}
			break;
	case 1:	
		for(unsigned int i=0;i<nMBoxes;i++)
		{
			if(mailBoxes[mBoxIndices[i]].pSubP->box.pMin.y<bestT)
			{
				belowMBoxIndices[belowIndex]=mBoxIndices[i];
				belowIndex++;
			}
			if(mailBoxes[mBoxIndices[i]].pSubP->box.pMax.y>=bestT)
			{
				aboveMBoxIndices[aboveIndex]=mBoxIndices[i];
				aboveIndex++;
			}
		}
		break;
	case 2:	
		for(unsigned int i=0;i<nMBoxes;i++)
		{
			if(mailBoxes[mBoxIndices[i]].pSubP->box.pMin.z<bestT)
			{
				belowMBoxIndices[belowIndex]=mBoxIndices[i];
				belowIndex++;
			}
			if(mailBoxes[mBoxIndices[i]].pSubP->box.pMax.z>=bestT)
			{
				aboveMBoxIndices[aboveIndex]=mBoxIndices[i];
				aboveIndex++;
			}
		}
	}*/
// 	cout<<"\nbelowIndices"<<endl;
// 	for(int i=0;i<bestBelow;i++)
// 		cout<<belowMBoxIndices[i]<<ends;
// 	cout<<"\naboveIndices"<<endl;
// 	for(int i=0;i<bestAbove;i++)
// 		cout<<aboveMBoxIndices[i]<<ends;


/*	cout<<"compute two boundingboxes begin"<<endl;*/
	//����ָ�ƽ�������BoundingBox
	WBoundingBox belowBox,aboveBox;
	belowBox=aboveBox=bBox;
/*	cout<<"bestT"<<bestT<<endl;*/
	switch(splitAxis)
	{
	case 0:
		belowBox.pMax.x=aboveBox.pMin.x=bestT;
		currNode->type=WKDNode::KDN_XSPLIT;
		break;
	case 1:
		belowBox.pMax.y=aboveBox.pMin.y=bestT;					currNode->type=WKDNode::KDN_YSPLIT;
		break;
	case 2:
		belowBox.pMax.z=aboveBox.pMin.z=bestT;
		currNode->type=WKDNode::KDN_ZSPLIT;
	}
	unsigned int nextDepth=depth+1;

// 	bBox.displayCoords();
//	belowBox.displayCoords();
// 	aboveBox.displayCoords();
	//����below�ӽڵ�
//	cout<<"build subTree 1 begin"<<endl;
	currNode->splitPos=bestT;
	vNodes.push_back(*currNode);
	unsigned int currIndices=vNodes.size()-1;
	buildTreeCore(nextDepth,belowMBIndices,belowBox,vNodes,vBoxes);

	//currNode->type=(WKDNode::NodeType)splitAxis;
	vNodes[currIndices].aboveChild=nUsedNodes;



/*	cout<<"build subTree 2 begin"<<endl;*/
	//����above�ӽڵ�
	buildTreeCore(nextDepth,aboveMBIndices,aboveBox,vNodes,vBoxes);
	delete currNode;
	delete currBBox;
// 	cout<<"delete memory begin"<<endl;
// 
// 	if(belowMBoxIndices)
// 		delete []belowMBoxIndices;
// 	if(aboveMBoxIndices)
// 		delete []aboveMBoxIndices;
// 	belowMBoxIndices=aboveMBoxIndices=NULL;
// 	cout<<"delete memory end"<<endl;
}
void WKDTree::buildTree(WScene&scene)
{
	delete[]mailBoxes;
	delete[]nodeBoxes;
	delete[]nodes;
	WClock timer;
	timer.begin();
	buildMailBoxes(scene);
	sceneBox=scene.getBBox();
 	cout<<"nMailBoxes"<<nMailBoxes<<endl;
// 		int*mBoxIndices=new int[nMailBoxes];
	vector<int>mBIndices;
	vector<WKDNode>vNodes;
	vector<WBoundingBox>vBoxes;
	mBIndices.resize(nMailBoxes);
	vNodes.clear();
	vNodes.reserve(nAllocatedNodes);
	vBoxes.reserve(nAllocatedNodes);
// 	for(unsigned int i=0;i<nMailBoxes;i++)
// 		mBoxIndices[i]=i;
	for(unsigned int i=0;i<nMailBoxes;i++)
		mBIndices[i]=i;
	WBoundingBox bBox=scene.getBBox();
	process=0.0f;
	buildTreeCore(0,mBIndices,bBox,vNodes,vBoxes);
	/*	delete[]mBoxIndices;*/
	this->nodes=new WKDNode[nUsedNodes];
	this->nodeBoxes=new WBoundingBox[nUsedNodes];
	for(unsigned int i=0;i<nUsedNodes;i++)
	{
		nodes[i]=vNodes[i];
		nodeBoxes[i]=vBoxes[i];
	}
	cout<<"nNodes: "<<nUsedNodes<<endl;
/*	delete[]nodeBoxes;
	nodeBoxes=NULL;*/
	vNodes.clear();
	vBoxes.clear();
	timer.end();
	cout<<"build complete.Total time: "<<endl;
	timer.display();
	cout<<"nNodes: "<<nUsedNodes<<endl;
}
void WKDTree::drawTree(unsigned int nthBox,
					  float R,float G,float B)
{
	if(nodeBoxes == NULL)
		return;
	glColor3f(R,G,B);
	//cout<<nUsedNodes<<endl;
	if(nUsedNodes==0)
		return;
/* 	for(unsigned int i=0;i<nUsedNodes;i++)
 		nodeBoxes[i].draw();*/
	if(nodes[nthBox].type==WKDNode::KDN_LEAF)
	{
//		cout<<"leaf"<<endl;
		nodeBoxes[nthBox].draw();
		return;
	}
	else
	{
//		cout<<"above"<<endl;
		nodeBoxes[nthBox].draw();
		unsigned int aboveIndices=
			nodes[nthBox].aboveChild;
		drawTree(nthBox+1,R,G,B);
		drawTree(aboveIndices,R,G,B);
		return;
	}
}
void WKDTree::drawScene(unsigned int nthBox,bool isFill)
{
	if(nUsedNodes==0)
		return;
	if(nodes[nthBox].type==WKDNode::KDN_LEAF)
	{
		int *mBIndices=nodes[nthBox].mailBoxIndices;
		unsigned int nMBs=nodes[nthBox].nMailBoxes;
		for(unsigned int i=0;i<nMBs;i++)
		{
			this->mailBoxes[mBIndices[i]].pSubP->draw(true);
		}
		return;
	}
	else
	{
		unsigned int aboveIndices=nodes[nthBox].aboveChild;
		drawScene(nthBox+1,isFill);
		drawScene(aboveIndices,isFill);
		return;
	}
}
void WKDTree::displayNodeInfo()
{
	for(unsigned int i=0;i<nUsedNodes;i++)
	{
		cout<<i<<' '<<nodes[i].type<<ends;
		if(nodes[i].type!=WKDNode::KDN_LEAF)
			cout<<nodes[i].aboveChild;
		else
			cout<<"nMailBoxes"<<nodes[i].nMailBoxes;
		cout<<endl;
	}
	return;
}

void WKDTree::displayTime()
{
	clock.display();
}
bool WKDTree::intersect(WRay &r, WDifferentialGeometry &DG,
					   int& endNode, int beginNode)
{
//	cout<<"\n\n#################intersection test begin#########"<<endl;

	//��¼�����ڽڵ��Χ���ڲ��Ĳ�����Ч��Χ�ı�����
	//������������Ӱ���������Ĳ�����Χr.tMin,r.tMax
	//�����ý�����ͬtSplit���ȷ��Զ���ڵ㣬
	//�Լ��Ƿ�Ҫ�������ӽڵ㶼���в���
	float tMin,tMax;
//	r.normalizeDir();//����������λ��

	//sceneBox.displayCoords();

	//������������ཻ��ֱ�ӷ���
	if(!sceneBox.isIntersect(r,tMin,tMax))
		return false;
	if(nUsedNodes==0)//KD��Ϊ��
		return false;
	currRayID++;

	WKDNode*pCurrNode=&nodes[0];
	//"��"�ڵ�͡�Զ���ڵ��ָ��
	WKDNode*pNearNode,*pFarNode;
	WKDNode*pNextNode;//��һ��ѭ��Ҫ����Ľڵ�
	float rayDComponent;//���������ڷָ�����ķ���
	float rayPComponent;//����ڷָ�����ķ���
	float tSplit;		//�ָ�ƽ��Ĺ���tֵ
	vector<WKDToDo>toDo;

//	clock.begin();
	while(1)
	{
// 		r.tMin=tMin;
// 		r.tMax=tMax;//���߲�����Χ
//		cout<<"\nminmax"<<tMin<<' '<<tMax<<endl;
		if(pCurrNode->type==WKDNode::KDN_LEAF)
			goto LEAF;
		switch(pCurrNode->type)
		{
		case WKDNode::KDN_XSPLIT:
			rayDComponent=r.direction.x;
			rayPComponent=r.point.x;break;
		case WKDNode::KDN_YSPLIT:
			rayDComponent=r.direction.y;
			rayPComponent=r.point.y;break;
		case WKDNode::KDN_ZSPLIT:
			rayDComponent=r.direction.z;
			rayPComponent=r.point.z;
		}
//		cout<<"pCurrNodeType"<<pCurrNode->type<<endl;

		//ǰ���ǵ�ǰ�ڵ����ڲ��ڵ�
		if(rayDComponent>1e-9f)
		{
			pNearNode=pCurrNode+1;
			pFarNode=&nodes[pCurrNode->aboveChild];
		}
		else if(rayDComponent<-1e-9f)
		{
			pNearNode=&nodes[pCurrNode->aboveChild];
			pFarNode=pCurrNode+1;
		}
		else//�����ڷָ�����ķ��������ķ���Ϊ0
		{
			if(rayPComponent>pCurrNode->splitPos)
			{
				pNextNode=&nodes[pCurrNode->aboveChild];
			}
			else
			{
				pNextNode=pCurrNode+1;
			}
		}
		//����������������ж��Ƿ�Ҫ���������ӽڵ�
		if(rayDComponent>1e-9f||rayDComponent<-1e-9f)
		{
			//����tSplit
			tSplit=(pCurrNode->splitPos-rayPComponent)/rayDComponent;
			//�������������
			//�����ж����߸�һ���ӽڵ㲻���ཻ��
			//����һ��ֻ��Ҫ����һ���ӽڵ�
			if(tSplit<tMin)
			{
				pNextNode=pFarNode;
			}
			else if(tSplit>tMax)
			{
				pNextNode=pNearNode;
			}
			//����������ȼ���Ͻ����ӽڵ㣬Ȼ������Զ���ӽڵ�
			else
			{
				pNextNode=pNearNode;
				WKDToDo nextToDo;
				nextToDo.pNode=pFarNode;
				nextToDo.tMin=tSplit;
				nextToDo.tMax=tMax;
				tMax=tSplit;
				toDo.push_back(nextToDo);
	//			cout<<"pushed tSplit="<<tSplit<<endl;
			}
		}
		//cout<<"splitPos"<<pNextNode->splitPos<<endl;
		if(pNextNode->type!=WKDNode::KDN_LEAF)//��ѡ�ڵ�Ϊ�ڽڵ�
		{
			pCurrNode=pNextNode;
			continue;
		}
		else//ΪҶ�ڵ�
		{
			pCurrNode=pNextNode;
LEAF:
			WTriangle bestTriangle;//��¼������ཻ������
			float bestT=r.tMax;//����Ĺ���tֵ
			float oldTMax=r.tMax;//���߱�����tMax
			int*mBIndices=pCurrNode->mailBoxIndices;//mailbox�±�����
			unsigned int nMBs=
				(unsigned int)pCurrNode->nMailBoxes;//mailBox�±���
//			cout<<"nMailBoxes"<<nMBs<<endl;
			WMailBox*pMB;//��ǰmailboxָ��
			//������mailbox��
			for(unsigned int nthMB=0;nthMB<nMBs;nthMB++)
			{
				pMB=&mailBoxes[mBIndices[nthMB]];
//				glColor3f(0,0.7,0);
				//����Ҫ�����ÿ��mailbox�İ�Χ��
//				pMB->pSubP->box.draw();
				float t;//�ཻ��tֵ
				WTriangle tri;//��Ӧ��������
				
				//����������ʾ���mailBox��Χ������߲��ཻ
				//���ڲ���������߲��ཻ
				if(pMB->rayID==currRayID)					
				{
					continue;
				}
				else
				{
					bool isIntersect=false;//��¼�����Ƿ������ཻ�ı���
					float mBTMin,mBTMax;//�����������ƺ�ûʲô��
					//����ÿ��mailbox��
					//�����subP�İ�Χ���ཻ����һ�����㽻��
					if(pMB->pSubP->box.isIntersect(r,mBTMin,mBTMax))
					{
						//����ཻ����mailbox�ڵ���������
						unsigned int beginIndex=pMB->pSubP->beginIndex;
						unsigned int nFaces=pMB->pSubP->nFaces+beginIndex;
//						cout<<beginIndex<<"*****"<<nFaces<<endl;
//						clock.begin();
						for(unsigned int nthFace=beginIndex;
							nthFace<nFaces;
							nthFace++)
						{
							//��ÿ��mailbox�ڲ���ÿ����������
							pMB->pSubP->pPrimitive->getTriangle(nthFace,tri);
							//cout<<"isSucceed"<<is<<endl;
							t=tri.intersectTest(r);

							//����ÿ�������ཻ���Ե�������
						//	tri.draw(false,true);
							if(t<bestT)//�ҵ������Ľ���
							{
								//isIntersect��ʾ
								//����ͬmailbox������������н���
								isIntersect=true;
								bestTriangle=tri;
								bestT=t;
//								cout<<"update"<<endl;
								//r.tMax=bestT;
								//��һ����Ҫ����
								//����tri.intersectTest(r)����������
								//���ڲ���
							}
						} 

										
//						clock.end();
					}
					if(!isIntersect)
						pMB->rayID=currRayID;
					//˵����mailbox�ڵ�������߲��ཻ	
					//��mailbox�İ�Χ������߲��ཻ
				}
			}
//			cout<<setprecision(10)<<"bestT"<<bestT<<endl;
//			cout<<setprecision(10)<<"oldTMax"<<oldTMax<<endl;
			//�󽻳ɹ�,�ҽ����ڽڵ㣨������mailbox����Χ����
			if(bestT<r.tMax&&bestT<=tMax&&bestT>=tMin)
			{
//				cout<<"**************find intersection************"<<endl;
//				clock.begin();
				bestTriangle.intersect(r,DG);
//				clock.end();
//				DG.position.showCoords();
//				clock.end();
				return true;
			}
			else//��ʧ��
			{
//				cout<<"**************not find intersection************"<<endl;
				if(toDo.size()==0)//���ʾ����������KD����û�н���
				{
//					clock.end();
					return false;
				}
				WKDToDo nextToDo=toDo.back();
				toDo.pop_back();
//				cout<<"pop"<<endl;
				pCurrNode=nextToDo.pNode;
				tMin=nextToDo.tMin;
				tMax=nextToDo.tMax;
//				cout<<"min"<<tMin<<"max"<<tMax<<endl;
			}
		}
	}
}
bool WKDTree::isIntersect(WRay &r, int beginNode)
{
	//	cout<<"\n\n#################intersection test begin#########"<<endl;

	//��¼�����ڽڵ��Χ���ڲ��Ĳ�����Ч��Χ�ı�����
	//������������Ӱ���������Ĳ�����Χr.tMin,r.tMax
	//�����ý�����ͬtSplit���ȷ��Զ���ڵ㣬
	//�Լ��Ƿ�Ҫ�������ӽڵ㶼���в���
	float tMin,tMax;
	//r.normalizeDir();//����������λ��

	//sceneBox.displayCoords();

	//������������ཻ��ֱ�ӷ���
	if(!sceneBox.isIntersect(r,tMin,tMax))
		return false;
	if(nUsedNodes==0)//KD��Ϊ��
		return false;
	currRayID++;

	WKDNode*pCurrNode=&nodes[0];
	//"��"�ڵ�͡�Զ���ڵ��ָ��
	WKDNode*pNearNode,*pFarNode;
	WKDNode*pNextNode;//��һ��ѭ��Ҫ����Ľڵ�
	float rayDComponent;//���������ڷָ�����ķ���
	float rayPComponent;//����ڷָ�����ķ���
	float tSplit;		//�ָ�ƽ��Ĺ���tֵ
	vector<WKDToDo>toDo;

	while(1)
	{
		// 		r.tMin=tMin;
		// 		r.tMax=tMax;//���߲�����Χ
		//		cout<<"\nminmax"<<tMin<<' '<<tMax<<endl;

		if(pCurrNode->type==WKDNode::KDN_LEAF)
			goto LEAF;
		switch(pCurrNode->type)
		{
		case WKDNode::KDN_XSPLIT:
			rayDComponent=r.direction.x;
			rayPComponent=r.point.x;break;
		case WKDNode::KDN_YSPLIT:
			rayDComponent=r.direction.y;
			rayPComponent=r.point.y;break;
		case WKDNode::KDN_ZSPLIT:
			rayDComponent=r.direction.z;
			rayPComponent=r.point.z;
		}
		//		cout<<"pCurrNodeType"<<pCurrNode->type<<endl;

		//ǰ���ǵ�ǰ�ڵ����ڲ��ڵ�
		if(rayDComponent>1e-9f)
		{
			pNearNode=pCurrNode+1;
			pFarNode=&nodes[pCurrNode->aboveChild];
		}
		else if(rayDComponent<-1e-9f)
		{
			pNearNode=&nodes[pCurrNode->aboveChild];
			pFarNode=pCurrNode+1;
		}
		else//�����ڷָ�����ķ��������ķ���Ϊ0
		{
			if(rayPComponent>pCurrNode->splitPos)
			{
				pNextNode=&nodes[pCurrNode->aboveChild];
			}
			else
			{
				pNextNode=pCurrNode+1;
			}
		}
		//����������������ж��Ƿ�Ҫ���������ӽڵ�
		if(rayDComponent>1e-9f||rayDComponent<-1e-9f)
		{
			//����tSplit
			tSplit=(pCurrNode->splitPos-rayPComponent)/rayDComponent;
			//�������������
			//�����ж����߸�һ���ӽڵ㲻���ཻ��
			//����һ��ֻ��Ҫ����һ���ӽڵ�
			if(tSplit<tMin)
			{
				pNextNode=pFarNode;
			}
			else if(tSplit>tMax)
			{
				pNextNode=pNearNode;
			}
			//����������ȼ���Ͻ����ӽڵ㣬Ȼ������Զ���ӽڵ�
			else
			{
				pNextNode=pNearNode;
				WKDToDo nextToDo;
				nextToDo.pNode=pFarNode;
				nextToDo.tMin=tSplit;
				nextToDo.tMax=tMax;
				tMax=tSplit;
				toDo.push_back(nextToDo);
				//			cout<<"pushed tSplit="<<tSplit<<endl;
			}
		}
		//cout<<"splitPos"<<pNextNode->splitPos<<endl;
		if(pNextNode->type!=WKDNode::KDN_LEAF)//��ѡ�ڵ�Ϊ�ڽڵ�
		{
			pCurrNode=pNextNode;
			continue;
		}
		else//ΪҶ�ڵ�
		{
			pCurrNode=pNextNode;
LEAF:
			WTriangle bestTriangle;//��¼������ཻ������
			float bestT=r.tMax;//����Ĺ���tֵ
			float oldTMax=r.tMax;//���߱�����tMax
			int*mBIndices=pCurrNode->mailBoxIndices;//mailbox�±�����
			unsigned int nMBs=
				(unsigned int)pCurrNode->nMailBoxes;//mailBox�±���
			//			cout<<"nMailBoxes"<<nMBs<<endl;
			WMailBox*pMB;//��ǰmailboxָ��
			//������mailbox��
			for(unsigned int nthMB=0;nthMB<nMBs;nthMB++)
			{
				pMB=&mailBoxes[mBIndices[nthMB]];
				//				glColor3f(0,0.7,0);
				//����Ҫ�����ÿ��mailbox�İ�Χ��
				//				pMB->pSubP->box.draw();
				float t;//�ཻ��tֵ
				WTriangle tri;//��Ӧ��������

				//����������ʾ���mailBox��Χ������߲��ཻ
				//���ڲ���������߲��ཻ
				if(pMB->rayID==currRayID)					
				{
					continue;
				}
				else
				{
					bool isIntersect=false;//��¼�����Ƿ������ཻ�ı���
					float mBTMin,mBTMax;//�����������ƺ�ûʲô��
					//����ÿ��mailbox��
					//�����subP�İ�Χ���ཻ����һ�����㽻��
					if(pMB->pSubP->box.isIntersect(r,mBTMin,mBTMax))
					{
						//����ཻ����mailbox�ڵ���������
						unsigned int beginIndex=pMB->pSubP->beginIndex;
						unsigned int nFaces=pMB->pSubP->nFaces+beginIndex;
						//cout<<beginIndex<<"*****"<<nFaces<<endl;

						for(unsigned int nthFace=beginIndex;
							nthFace<nFaces;
							nthFace++)
						{
							//��ÿ��mailbox�ڲ���ÿ����������
							pMB->pSubP->pPrimitive->getTriangle(nthFace,tri);
							//cout<<"isSucceed"<<is<<endl;
							t=tri.intersectTest(r);

							//����ÿ�������ཻ���Ե�������
							//	tri.draw(false,true);
							if(t<r.tMax)//�ҵ������Ľ���
							{
								//isIntersect��ʾ
								//����ͬmailbox������������н���
								return true;
								//								cout<<"update"<<endl;
								//r.tMax=bestT;
								//��һ����Ҫ����
								//����tri.intersectTest(r)����������
								//���ڲ���
							}
						} 


					}
					if(!isIntersect)
						pMB->rayID=currRayID;
					//˵����mailbox�ڵ�������߲��ཻ	
					//��mailbox�İ�Χ������߲��ཻ
				}
			}
			//			cout<<setprecision(10)<<"bestT"<<bestT<<endl;
			//			cout<<setprecision(10)<<"oldTMax"<<oldTMax<<endl;
			//�󽻳ɹ�,�ҽ����ڽڵ㣨������mailbox����Χ����
// 			if(bestT<r.tMax&&bestT<=tMax&&bestT>=tMin)
// 			{
// 				//				cout<<"**************find intersection************"<<endl;
// 				bestTriangle.intersect(r,DG);
// 				//				DG.position.showCoords();
// 				return true;
// 			}
// 			else//��ʧ��
// 			{
				//				cout<<"**************not find intersection************"<<endl;
			if(toDo.size()==0)//���ʾ����������KD����û�н���
				return false;
			WKDToDo nextToDo=toDo.back();
			toDo.pop_back();
			//				cout<<"pop"<<endl;
			pCurrNode=nextToDo.pNode;
			tMin=nextToDo.tMin;
			tMax=nextToDo.tMax;
				//				cout<<"min"<<tMin<<"max"<<tMax<<endl;
//			}
		}
	}
}
