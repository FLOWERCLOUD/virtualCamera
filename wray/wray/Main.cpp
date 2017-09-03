
#include "stdafx.h"

GLint winWidth=800,winHeight=600;
WObjReader reader;


float alpha=M_PI_2*3.0f,beta=0.0f;
float r=10;  //�������������alpha, betaЭͬȷ��Lx, Ly, Lz��Ҳ�������λ�ã���ֵ
float fovDeg = 80;  //ĳ�Ƕ��㶮��
float fov = fovDeg / 180.0f * M_PI;  //ĳ�Ƕȵ�rad�Ʊ�ʾ
float Lx,Ly,Lz;

/**********��������Ҫʹ�õı�����Ŀǰ�о�����û��ʵ����Ч*******************/
float lastMouseX,lastMouseY;  
bool isLeftButtonDown;
bool isMiddleButtonDown;
bool isRightButtonDown;
/**********�����Ʊ���End*********************/

/*************���̿��Ʊ���******************/
bool isFirstMove;
bool isAltKeyDown;

//���ڿ����Ƿ���ʾ���½�С����
bool isShowScene = true;
//���ڿ����Ƿ���ʾ���½Ƿ����е�������
bool isShowGrid = true;
/*************���̿��Ʊ���End***************/

WVector3 cameraTarget(0);
WRay myRay;

//WRay myRay2;
float rayalpha = 0.1f,raybeta = 0.1f;
//WDifferentialGeometry DG;
WScene myScene;
WAccelerator* myAccelerator;
//WOctTree*myOctTree;
WCamera*myCamera;
WLight *myLight;
WLight *myLight2;
//WSurfaceIntegrator*myIntegrator;
//Film myFilm(200,200,1,Filter::FILTER_BOX,Sampler::SAMPLER_RANDOM,65536);
bool isComputing=false;


bool isEyeChanged = false;
//#define PRODUCTION_RENDER
//#define PACKET_INTERSECT
int numCores = 4;//������������趨�߳���,�����ʵ������߳����������ȾЧ��
int threadIDArray[8] = {0,1,2,3,4,5,6,7};
//clock_t renderTime[8];
int isThreadComputing[8];

bool isInitiate = true;

//��Ҫ�ĵط���init()������compute������������Ƕѱ�������


#define INFINITE_RENDER
DWORD WINAPI compute(LPVOID lpParam)  //ʹ�ù����е�ĳЩ���������й��߸��ټ��㲢���Ƽ�����
{
	int threadID = *((int*)lpParam);
	//cout << "threadID" << threadID << endl;
	isThreadComputing[threadID] = 1;
	if(/*isComputing==true*/true)
	{
//		if (isEyeChanged)
//		{
			myCamera->clearFilm();
//			isEyeChanged = false;
//		}

//		float progress=0,newprogress;
//		renderTime[threadID] = clock();
		WVector3 color;
		//���߸���
		WRay r;
		//��ͳ����ȫ������
		myAccelerator->resetStatistics();

		int resX, resY;
		//ȡ�ô��ڷֱ���
		//���Ҫ��ֲ�� �����myCamera�û���Ŀ��ƽ̨���趨��Camera���󣬻���������Ϊ����һ����ȡ�ֱ��ʵķ���
		//���߾����������Ŀ�����Camera�࣬��Ŀ��ƽ̨��ά���Ķ�������ʼ�����Camera�������Scene��ͬ�ˡ�
		myCamera->getFilmResolution(resX, resY);


		//�ǻԤ����飬��֪���ã�����û��
//#ifdef PACKET_INTERSECT
//		const int groupSize = 32767;
//		WRayGroup rayGroup;
//		rayGroup.allocSpace(groupSize);
//
//		int nGroups = int(float(resX * resY) / groupSize + 0.5);
//		int nthRay = 0;
//		for (int ithGroup = 0; ithGroup < nGroups; ++ithGroup)
//		{
//			int nRays = 0;
//			for (nthRay = ithGroup * groupSize; 
//				nthRay < (ithGroup + 1) * groupSize && nthRay < resX * resY; nthRay++, nRays++)
//			{
//				int x = nthRay % resX;
//				int y = nthRay / resX;
//				WRay r;
//				myCamera->getNextRay(r, x, y);
//				rayGroup.ori_min[nRays * 4] = r.point.x;
//				rayGroup.ori_min[nRays * 4 + 1] = r.point.y;
//				rayGroup.ori_min[nRays * 4 + 2] = r.point.z;
//				rayGroup.ori_min[nRays * 4 + 3] = r.tMin;
//				rayGroup.dir_max[nRays * 4] = r.direction.x;
//				rayGroup.dir_max[nRays * 4 + 1] = r.direction.y;
//				rayGroup.dir_max[nRays * 4 + 2] = r.direction.z;
//				rayGroup.dir_max[nRays * 4 + 3] = r.tMax;
//				rayGroup.ori[0][nRays] = r.point.x;
//				rayGroup.ori[1][nRays] = r.point.y;
//				rayGroup.ori[2][nRays] = r.point.z;
//				rayGroup.invDir[0][nRays] = 1.0f / r.direction.x;
//				rayGroup.invDir[1][nRays] = 1.0f / r.direction.y;
//				rayGroup.invDir[2][nRays] = 1.0f / r.direction.z;
//				rayGroup.isectTriangle[nRays] = NULL;
//			}
//			rayGroup.numRays = nRays;
//			WMultiRSBVH& rsBVH = *((WMultiRSBVH*)myAccelerator);
//			rsBVH.intersect(rayGroup);
//			for (nthRay = ithGroup * groupSize, nRays = 0;
//				nthRay < (ithGroup + 1) * groupSize && nthRay < resX * resY; 
//				nthRay++, nRays++)
//			{
//				int x = nthRay % resX;
//				int y = nthRay / resX;
//				WVector3 color;
//				if (rayGroup.isectTriangle[nRays])
//					color = rayGroup.DG[nRays].normal;
//				else
//					color = WVector3(0);
//				myCamera->setColor(fabs(color.x),fabs(color.y),fabs(color.z),x,y);
//			}
//			glutPostRedisplay();
//		}
//#else
		//���ﴫ��Ĳ�������һ����myScene�����е��鷳��������Ҫ����Ŀ��ƽ̨����ӦScene���󣬲�����Ҫ��һЩ�޸�
		WPathIntegrator * threadIntegrator=new WPathIntegrator(&myScene,myAccelerator,5,WSampler::SAMPLER_RANDOM,1.0f);
		//yInterval�����洢����ͼ��y��ָ�ɿ�,ÿ��ĺ��,��������numCores��ֵΪ4
		int yInterval = resY / numCores;
		//������ID(���������Զ�)����y����ʼֵ
		int yBegin    = yInterval * threadID;
		int ithIteration = 0;    //������������
		for (ithIteration = 0; ithIteration < 10000; ++ithIteration)
		{
			//ɨ��..���к���
			for (int y = yBegin;  y < yBegin + yInterval; ++y)
			{
				for (int x = 0; x < resX; ++x) //resX��x�᷽��ֱ���,���仰˵Ҳ���������x��������ȡ����ֵ 
				{
//#ifdef PRODUCTION_RENDER
					color = WVector3(0,0,0);
					const int dimSubSample = 1;  //�������ص㣬ÿ���߷ֳ�dimSubSample�ݣ�Ҳ���ǰ�һ�����ص�ֳ�dimSubSample^2�ݣ��п���ݵ�Ч��
					const float subSampleSize = 1.0f / float(dimSubSample);//�˴������ò�����Ĵ�С,�������С��ͼ��Ч����Ӱ���ݲ���ȷ
					for (int subY = 0; subY < dimSubSample; ++subY)
					{
						for (int subX = 0; subX < dimSubSample; ++subX)
						{
							float rayX = x + subX * subSampleSize;
							float rayY = y + subY * subSampleSize;
							//r���δ���ʱ���Է���Ϊ0,0,0�����
							myCamera->getNextRay(r, rayX, rayY);
							color+=threadIntegrator->integrate(r);
						}
					}
					const float numSubSample = dimSubSample * dimSubSample;
					//�����accumulateColor�����������ǽ�����õ�����ɫ�ۼӵ������е���ɫ���ݳ�Ա��.
					//���������û�������ƵĹ���,��ֻ�Ǽ���õ�������ɫ������洢֮,Ȼ����display�����л�����
					myCamera->accumulateColor(color.x / numSubSample,color.y / numSubSample,color.z / numSubSample, x, y);
//					printf("color.r=%f, color.g=%f, color.b=%f\n", color.x, color.y, color.z);
					//myCamera->accumulateColor(0.5,0.8,1.0, x, y);
//#else
//					WDifferentialGeometry DG;
//					myAccelerator->intersect(r, DG, NULL, -1);
//					myCamera->setColor(abs(DG.normal.x),abs(DG.normal.y),abs(DG.normal.z), x, y);
//#endif
				}
//				if (/*isComputing == false*/ false)
//					goto END_RENDERING;
/*
				newprogress= float(y) / resY;
				if(int(progress*20)!=int(newprogress*20))
				{
					progress=newprogress;
					cout<<"rendering progress"<<int(100*progress)<<'%'<<'\r';
				}
*/
			}
			//cout << threadID <<"th thread\t" << ithIteration << " th Iteration" << endl;
		}
//#endif


//�������ƣ�������Դ
//END_RENDERING:
		delete threadIntegrator;
		glutPostRedisplay();
		//myAccelerator->displayIsectStatistics();
		//myIntegrator->displayTime();
//		renderTime[threadID] =clock()-renderTime[threadID] ;
//		cout<<"thread " << threadID << " rendering time:"
//			<<double(renderTime[threadID])/CLOCKS_PER_SEC
//			<<'s'<<endl;
//		cout<<endl;
	}
//	isThreadComputing[threadID] = false;
//	for (;;)
//	{
//		if (isThreadComputing[0] + 
//			isThreadComputing[1] + 
//			isThreadComputing[2] + 
//			isThreadComputing[3] == 0)
//		{
//			isComputing = false;
//			break;
//		}
//		Sleep(250);
//	}
	return 0;
}



void init()
{
	glClearColor(0.5,0.5,0.5,0.5);
	glEnableClientState(GL_VERTEX_ARRAY);

	//initialize camera
	//�����λ�ã���r, beta, alpha����ֵ��ͬȷ����
	Lx=r*cos(beta)*cos(alpha);
	Ly=r*cos(beta)*sin(alpha);
	Lz=r*sin(beta);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Lx,Ly,Lz,20,20,20,0,0,1);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovDeg,float(winWidth)/float(winHeight), 0.1f,1000.0f);

	reader.readFile("014.obj");//("library.obj");
	//reader.readFile("tiaowu.obj");
	//reader.readFile("mesh_0000.obj");

	//���ù��ߣ������÷�����ȷ�����ƺ�û������
	//myRay.point=WVector3(0,0,0);
	//myRay.direction=WVector3(1.01,0.01,0.01);
	//myRay.tMin=1e-7f;
	//rayalpha=raybeta=0.0f;


	//myScene����reader�����ݹ�����Ҫ��Ⱦ�ĳ���
	//��ֲ��Ŀ��ƽ̨ʱ���Կ�������Ŀ��ƽ̨�Ѿ����������������һ��������mySceneʹ�õ�reader����

	//�����buildScene(reader), ���ĳ�buildScene(Mesh);Ӧ�þ�OK��ǰ���reader.readFile()����ȥ��
	myScene.buildScene(reader);
//	myScene.rebuildAllSubPs(1);
	//myScene.showMaterials();
	//myScene.showTriangles();
	//�������ƽ���myScene֮��ͽ�reader�е���������ˡ�
	//���Ҳ���Բ�����
	reader.clear();

//���ּ�����,������������ѡһ��,����Ч��һ��,���ڸ������Ч��������ͬ
	myAccelerator = new WSimpleBVH();
//	myAccelerator = new WMultiBVH();
	//myAccelerator = new WSimpleKD();
//	myAccelerator = new WSimpleKD2();
//	myAccelerator = new WEnhancedKD();
//	myAccelerator = new WDualKD();


	//clock �ƺ�ֻ�������㽨�����������õ�ʱ��ġ�
//	clock_t c = clock();
	myAccelerator->buildTree(myScene);
	//2014-12-8


//	printf("\nbuild time:%fs\n", (clock() - c) / (float)CLOCKS_PER_SEC);
//	myAccelerator->displayNodeInfo();


	//ͬ�������õġ�
	/*WSimpleKD* skd = (WSimpleKD*)myAccelerator;
	WSKDNodeGPU* iGPU;
	WSKDLeafBodyGPU* lGPU;
	int* tGPU;
	int numNode, numLeafBody, numTriArray;*/
//	skd->getGPUArray(iGPU, lGPU, tGPU, numNode, numLeafBody, numTriArray);



	//��Ҫ���е�Ҫ�к��ã�
	//�����Ǽ�����ɫֵ��ʱ��Ҫ����Щ��ʼֵ
	myCamera = new WCamera;
	//parameter�Ĳ����ֱ��� ���λ��, �۲�Ŀ���, �Ϸ�, .......
	myCamera->setParameter(WVector3(Lx,Ly,Lz),/*cameraTarget*/WVector3(-1.0, -1.0, -1.0),WVector3(0,1,0),fov,winWidth*0.5/winHeight);
	myCamera->setFilmResolutionX(winWidth);
	myCamera->changeSampleSize(1);

	//��Ӳ���

	//...û�õĶ���..
	/*WMaterial*perfectRefl=new WPerfectReflectionMaterial("PRL",3);
	((WPerfectReflectionMaterial*)perfectRefl)->setColor(WVector3(1));
	WMaterial*perfectRefr=new WPerfectRefractionMaterial("PRR",3,WVector3(1,1,0.8),1.5);
	WMaterial*metal=new WMetalMaterial("Metal",4,WVector3(0.8,0.8,0.8),3);
	WMaterial*phong=new WPhongMaterial("phong",3,WVector3(1,0.5,0),1,100);
	WMaterial*dielectric=new WDielectricMaterial("ddd",3,WVector3(1,0.5,0),100000,2.0);*/
//	myScene.setNthMaterial(metal,0);
//	myScene.setNthMaterial(dielectric,0);
//	myScene.setNthMaterial(perfectRefr,0);

	//��ӵƹ�
	//ֻ��һ�����Դ
	myLight=new WPointLight(WVector3(50), WVector3(1,-5,1));
	myScene.addLight(myLight);
	myLight=new WPointLight(WVector3(50), WVector3(0,-2,3));
	myScene.addLight(myLight);

	//��������Ļ���������������ĵƣ���ĳ�־��ι�Դ
	myLight=new WRectangleLight(WVector3(0,-5,8),WVector3(0,0,-1),WVector3(1,0,0),10,5,WVector3(25),true);
//	myScene.addLight(myLight);
	//��������Ļ���������̨�ĵƣ�Ҳ����ĳ�־��ι�Դ
//	myLight=new WRectangleLight(WVector3(2,24,1),WVector3(0,-1,0),WVector3(0,0,1),4.5,6.5,WVector3(50),true);
//	myScene.addLight(myLight);

	//myIntegrator=new WPathIntegrator(&myScene,myAccelerator,3,WSampler::SAMPLER_RANDOM,1.0f);

//	myIntegrator = new WRecursivePathIntegrator(&myScene,myAccelerator,2,10);

/*	myIntegrator=new WIrradianceCacheIntegrator(
 		&myScene,myAccelerator,
  		20,50,20,10,3,5,
 			WSampler::SAMPLER_RANDOM,0.3,0.9,0.5,1.5);*/

}

void displayFcn()
{   std::cout<<"displayFcn"<<std::endl;
	DWORD dwThreadId,dwThrdParam=1;
	HANDLE hThread;
	//��ʾģʽ����
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glViewport(0,0,winWidth,winHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,winWidth,0,winHeight,-1,1);


	//��֮ǰ����õ�����ɫֵ���Ƶ���Ļ��ȥ
	myCamera->drawFilmInWorld(0,0,0);


	//���ڻ�������С����
	/***********************���½��Ӵ����ƴ���*******************************/
	
	
	if (isShowScene)
	{
		glEnable(GL_SCISSOR_TEST);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glScissor(0,0,winWidth/4,winHeight/4);
		glViewport(0,0,winWidth/4,winHeight/4);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(Lx,Ly,Lz,cameraTarget.x,cameraTarget.y,cameraTarget.z,0,0,1);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fovDeg,float(winWidth)/float(winHeight),
			0.1f,1000.0f);

		glColor3f(0.3,0.3,0.3);
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	//	glEnable(GL_CULL_FACE);
	//	glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);

		glBegin(GL_POINTS);
		glVertex3f(cameraTarget.x, cameraTarget.y, cameraTarget.z);
		glEnd();


		myCamera->drawCamera();
	//	myCamera->drawFilmInScreen();

	//	myFilm.draw();

	    //��������С�����������
		myScene.drawScene(false,false);
		/*
	//	myAccelerator->drawTree(0, 0,1,0);
	//	mySKD.drawTriangles();
	//	myTree.drawScene();
		myRay.tMin=1e-7f;
		myRay.tMax=M_INF_BIG;
		myRay.direction = WVector3(cos(raybeta)*cos(rayalpha),
								  cos(raybeta)*sin(rayalpha),sin(raybeta));
		myRay.drawSegment();
		glColor3f(0.7f,0.7f,0.7f);
		myRay2.drawSegment();

	#ifndef PACKET_INTERSECT
   		if(myAccelerator->intersect(myRay,DG))
  			DG.draw();
	#endif

		WBoundingBox box;
		box.pMin.x = -8.7500362;
		box.pMin.y = -1.3373880;
		box.pMin.z = 8.0343828;
		box.pMax.x = -8.7500353;
		box.pMax.y = -1.3051301;
		box.pMax.z = 8.0511990;
		glLineWidth(3.0f);
		glColor3f(1.0,0.0,0.0);
		box.draw();*/
		glColor3f(0.7f,0.7f,0.7f);
		glLineWidth(0.1f);

		const int n = 10;
		//���µ�if�������ڻ�������С���ڵ�������
		if (isShowGrid)
		{
			//����������
			glLineWidth(3.0f);
			glBegin(GL_LINES);
			glColor3f(1,0,0);
			glVertex3i(0,0,0);
			glVertex3i(n,0,0);
			glColor3f(0,1,0);
			glVertex3i(0,0,0);
			glVertex3i(0,n,0);
			glEnd();

			glLineWidth(0.5f);
			glBegin(GL_LINES);
			glColor3f(1,0,0);
			glVertex3i(0,0,0);
			glVertex3i(-n,0,0);
			glColor3f(0,1,0);
			glVertex3i(0,0,0);
			glVertex3i(0,-n,0);
			glEnd();

			glLineWidth(1.0f);
			glColor3f(0.7,0.7,0.7);
			glBegin(GL_LINES);
			for ( int i = -n; i <= n; ++i)
			{
				glVertex3i(i, -n, 0);
				glVertex3i(i, n, 0);
				glVertex3i(-n, i, 0);
				glVertex3i(n,i,  0);
			}
			glEnd();
		}



		if(!isComputing&&0)
		{
	//		((IrradianceCacheIntegrator*)myIntegrator)->displaySamplePoints();
	//		((IrradianceCacheIntegrator*)myIntegrator)->displayTreeNodes();
		}

		glDisable(GL_SCISSOR_TEST);
	}

	
	/***************************���½��Ӵ����ƴ���End*********************************/
	

	/*******************************�������߸�����Ⱦ�Ĵ���***********************************/
	//if��������Ķ���Ҫ��glutMainLoop����֮������һ�Σ�����Ҳֻ������һ��
	if(!isInitiate){
		for (int ithThread = 0; ithThread < numCores; ++ithThread)
		{
			hThread=CreateThread(
				NULL,
				0,
				compute,
				(threadIDArray + ithThread),
				0,
				&dwThreadId);
			if(hThread==NULL)
			{
				//cout<<"�����߳�ʧ��"<<endl;
			}
		}
		glutPostRedisplay();
		isInitiate = !isInitiate;
	}else{
	}

	/***************************�������߸�����Ⱦ�������End************************************/

	glutSwapBuffers();
	glFlush();


}
void mouseFcn(GLint button,GLint action,GLint xMouse,GLint yMouse)
{
	int mod=glutGetModifiers();
	if(mod==GLUT_ACTIVE_ALT)
		isAltKeyDown=true;
	else
		isAltKeyDown=false;
	if(button==GLUT_LEFT_BUTTON)
	{		
		if(action==GLUT_DOWN)
		{
			isLeftButtonDown=true;
			isFirstMove=true;
		}
		else if(action==GLUT_UP)
		{
			isLeftButtonDown=false;
			isFirstMove=true;
		}		
	}
	if(button==GLUT_MIDDLE_BUTTON)
	{
		if(action==GLUT_DOWN)
		{
			isMiddleButtonDown = true;
			isFirstMove = true;
		}
		else if(action==GLUT_UP)
		{
			isMiddleButtonDown = false;
			isFirstMove=true;
		}
	}
	if(button==GLUT_RIGHT_BUTTON)
	{
		if(action==GLUT_DOWN)
		{
			isRightButtonDown=true;
			isFirstMove=true;
		}
		else if(action==GLUT_UP)
		{
			isRightButtonDown=false;
			isFirstMove=true;
		}
	}
}

void mouseMoveFcn(GLint xMouse,GLint yMouse)
{
	float deltaX;
	float deltaY;
	if(isLeftButtonDown&&isAltKeyDown)
	{
		if(isFirstMove)
		{
			lastMouseX=xMouse;
			lastMouseY=yMouse;
			isFirstMove=false;
		}
		else
		{		
			deltaX=xMouse-lastMouseX;
			deltaY=yMouse-lastMouseY;
			lastMouseX=xMouse;
			lastMouseY=yMouse;
			alpha-=deltaX*0.004f;//deltaX;
			beta+=deltaY*0.004f;//deltaY;
			Lx=r*cos(beta)*cos(alpha) + cameraTarget.x;
			Ly=r*cos(beta)*sin(alpha) + cameraTarget.y;
			Lz=r*sin(beta) + cameraTarget.z;
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gluLookAt(Lx,Ly,Lz,cameraTarget.x,cameraTarget.y,cameraTarget.z,0,0,1);
			myCamera->setParameter(WVector3(Lx,Ly,Lz),cameraTarget,WVector3(0,0,1),fov,(float)winWidth/(float)winHeight);
			glutPostRedisplay();
		}
		isEyeChanged = true;
	}
	if(isMiddleButtonDown && isAltKeyDown)
	{
		if(isFirstMove)
		{
			lastMouseX=xMouse;
			lastMouseY=yMouse;
			isFirstMove=false;
		}
		else
		{
			deltaX=xMouse-lastMouseX;
			deltaY=yMouse-lastMouseY;
			lastMouseX=xMouse;
			lastMouseY=yMouse;
			WVector3 x,y;
			myCamera->getXY(x, y);
			cameraTarget += -0.01f * x * deltaX + 0.01f * y * deltaY;
			Lx=r*cos(beta)*cos(alpha) + cameraTarget.x;
			Ly=r*cos(beta)*sin(alpha) + cameraTarget.y;
			Lz=r*sin(beta) + cameraTarget.z;
			myCamera->setParameter(WVector3(Lx,Ly,Lz),cameraTarget,WVector3(0,0,1),fov,(float)winWidth/(float)winHeight);
			glutPostRedisplay();
		}
		isEyeChanged = true;
	}
	if(isRightButtonDown&&isAltKeyDown)
	{
		if(isFirstMove)
		{
			lastMouseX=xMouse;
			lastMouseY=yMouse;
			isFirstMove=false;
		}
		else
		{
			deltaX=xMouse-lastMouseX;
			lastMouseX=xMouse;
			r-=deltaX*0.05f;
			Lx=r*cos(beta)*cos(alpha) + cameraTarget.x;
			Ly=r*cos(beta)*sin(alpha) + cameraTarget.y;
			Lz=r*sin(beta) + cameraTarget.z;
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gluLookAt(Lx,Ly,Lz,cameraTarget.x,cameraTarget.y,cameraTarget.z,0,0,1);
			myCamera->setParameter(WVector3(Lx,Ly,Lz),cameraTarget,WVector3(0,0,1),fov,(float)winWidth/(float)winHeight);
			glutPostRedisplay();
		}
		isEyeChanged = true;

	}

}

void keyFcn(GLubyte key,GLint xMouse,GLint yMouse)
{
	DWORD dwThreadId,dwThrdParam=1;
	HANDLE hThread;
	switch (key)
	{
	case 'd':
		rayalpha+=0.015f;
		break;
	case 'a':
		rayalpha-=0.015f;
		break;
	case 's':
		raybeta+=0.015f;
		break;
	case 'w':
		raybeta-=0.015f;
		break;
	case 'r':
		if(!isComputing)
		{
			isComputing=true;

			for (int ithThread = 0; ithThread < numCores; ++ithThread)
			{
				hThread=CreateThread(
					NULL,
					0,
					compute,
					(threadIDArray + ithThread),
					0,
					&dwThreadId);
				if(hThread==NULL)
				{
					cout<<"�����߳�ʧ��"<<endl;
				}
				//Sleep(100);
			}
		}
		else
			isComputing = !isComputing;
		glutPostRedisplay();
		break;
	case 'g':
		isShowGrid = !isShowGrid;
		break;
	case 'e':
		isComputing = false;
		break;
	case 'v':
		isShowScene = !isShowScene;
	}

	glutPostRedisplay();

}
void reshapeFcn(GLint Width,GLint Height)
{
	//	cout<<"reshape"<<endl;
	winWidth=Width;
	winHeight=Height;

	glViewport(0,0,winWidth/4,winHeight/4);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovDeg,float(winWidth)/float(winHeight),
		0.1f,10000.0f);

	glViewport(0,0,winWidth,winHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,winWidth,0,winHeight,-1,1);

	if(!isComputing)
	{
		//��Ⱦʱ��ֹ��������-->����ֻ���޶��˴����л����Ĵ�С,���ڿ������ŵ�������С����
		myCamera->setParameter(WVector3(Lx,Ly,Lz),cameraTarget,WVector3(0,0,1),fov,(float)winWidth/(float)winHeight);
		myCamera->setFilmResolutionX(winWidth);
	}
}


void timerFcn(int v)
{
	glutTimerFunc(500, timerFcn, -1);
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	

	glutInit(&argc,(char**)argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(winWidth,winHeight);
	glutCreateWindow("WRay");


	init();

//	glutIdleFunc(displayFcn);
	glutDisplayFunc(displayFcn);
	glutReshapeFunc(reshapeFcn);
	glutKeyboardFunc(keyFcn);
	glutMouseFunc(mouseFcn);
	glutMotionFunc(mouseMoveFcn);
	//glutTimerFunc(500, timerFcn, -1);

	glutMainLoop();



	return 0;
}
