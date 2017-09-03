#include "StdAfx.h"
#include "Scene.h"
WScene::WScene(void)
{
	primitives=NULL;
	materials=NULL;
	nMaterials = nPrimitives = nSubPrimitives = nTriangles = 0;
	nTriangles = NULL;
}

WScene::~WScene(void)
{
	clearScene();
}
void WScene::clearScene()
{
	delete []primitives;
	primitives = NULL;
	for(unsigned int i=0;i<nMaterials;i++)
	{
		delete materials[i];
	}
	delete []materials;
	materials = NULL;
	for(unsigned int i=0;i<lights.size();i++)
	{
		delete lights[i];
	}
	lights.clear();
	nPrimitives=0;
	nSubPrimitives=0;
	nMaterials=0;
	clearTriangleArray();
}
void WScene::buildScene(WObjReader &reader)
{
	nPrimitives=reader.primitives.size();
	primitives=new WPrimitive[nPrimitives];
	unsigned int n=0;//ÿ��Primitive��SubPrimitive����
	for(unsigned int i=0;i<nPrimitives;i++)
	{
		reader.fillPrimitive(i,primitives[i]);
		primitives[i].getSubPrimNum(n);
		nSubPrimitives+=n;
	}
	buildSceneBBox();
	//�������������İ�Χ��
	//������������
	nMaterials=reader.Materials.size();
	materials=new WMaterial*[nMaterials];
	string mtlName;
	Wfloat3 diffuse;
	for(unsigned int i=0;i<nMaterials;i++)
	{
		mtlName=reader.Materials[i].name;
		diffuse=reader.Materials[i].diffuse;
//		cout<<mtlName<<diffuse.x<<diffuse.y<<diffuse.z<<endl;
		materials[i]=new WLambertMaterial(mtlName,i,WVector3(diffuse.x,diffuse.y,diffuse.z));
// 		((WLambertMaterial*)materials[i])->
// 			setColor(WVector3(diffuse.x,diffuse.y,diffuse.z));
	}
	buildTriangleArray();
}
#ifdef BLENDER_INCLUDE
void WScene::buildScene( Render* re )
{
	// ͳ�Ƴ������������Լ����������������ڷ���ռ�
	// ע�⣺ �����������Ԫ�ظ������ܶ�������������
	nPrimitives = 0;
	int approNumFace = 0;							// �������Ĺ���ֵ�����ڷ�������
	for ( ObjectInstanceRen* objInsRen = (ObjectInstanceRen*)re->instancetable.first; objInsRen; objInsRen = objInsRen->next)
	{
		nPrimitives++;
		approNumFace += objInsRen->obr->totvlak * 2; // ����ȫ�������ı���
	}

	// �����ݵ��뵽������������
	if (approNumFace == 0)
		return;
	triangles = new WTriangle[approNumFace];
	int actualNumFace = 0;							// ������ʵ������
	for (ObjectInstanceRen* objInsRen = (ObjectInstanceRen*)re->instancetable.first; objInsRen; objInsRen = objInsRen->next)
	{
		ObjectRen* objRen = objInsRen->obr;
		VlakTableNode* vlakNode;
		VlakRen* face;
		MTFace*  texFace;
		printf("\n object: %s\n", objInsRen->ob->id.name);
		for (int nthFace = 0; nthFace < objRen->totvlak ; nthFace++)
		{
			// ���һ�����ָ��
			if ((nthFace & 0xff) == 0)
			{
				vlakNode = &objRen->vlaknodes[nthFace >> 8];
				face = vlakNode->vlak;
				texFace = vlakNode->mtface;
			}
			else
			{
				face++;
				if (vlakNode->mtface)
					texFace++;
			}
			// vertex
			
			memcpy(&triangles[actualNumFace].point1, face->v1->co, sizeof(float) * 3);
			memcpy(&triangles[actualNumFace].point2, face->v2->co, sizeof(float) * 3);
			memcpy(&triangles[actualNumFace].point3, face->v3->co, sizeof(float) * 3);
			
			if (face->v4)					// �ı���
			{
				memcpy(&triangles[actualNumFace + 1].point1, face->v1->co, sizeof(float) * 3);
				memcpy(&triangles[actualNumFace + 1].point2, face->v3->co, sizeof(float) * 3);
				memcpy(&triangles[actualNumFace + 1].point3, face->v4->co, sizeof(float) * 3);
			}
			// normal
			if (face->flag & R_SMOOTH)		// �⻬ģʽ��ʹ����Ⱦ����
			{
				for (int i = 0; i < 3; i++)
				{
					triangles[actualNumFace].normal1.v[i] = -1.0f * face->v1->n[i];
					triangles[actualNumFace].normal2.v[i] = -1.0f * face->v2->n[i];
					triangles[actualNumFace].normal3.v[i] = -1.0f * face->v3->n[i];
				}
				if (face->v4)				// �ı���
				{
					for (int i = 0; i < 3; i++)
					{
						triangles[actualNumFace + 1].normal1.v[i] = -1.0f * face->v1->n[i];
						triangles[actualNumFace + 1].normal2.v[i] = -1.0f * face->v3->n[i];
						triangles[actualNumFace + 1].normal3.v[i] = -1.0f * face->v4->n[i];
					}
				}
			}
			else							// ����ʹ�ü��η���
			{
				memcpy(&triangles[actualNumFace].normal1, face->n, sizeof(float) * 3);
				triangles[actualNumFace].normal1 *= -1.0f;
				triangles[actualNumFace].normal2 = triangles[actualNumFace].normal1;
				triangles[actualNumFace].normal3 = triangles[actualNumFace].normal2;
				if (face->v4)				// �ı���
				{
					triangles[actualNumFace + 1].normal1 = 
						triangles[actualNumFace + 1].normal2 = 
						triangles[actualNumFace + 1].normal3 = 
						triangles[actualNumFace].normal1;
				}
			}
			// texcoord
			if (texFace)
			{
			
				memcpy(&triangles[actualNumFace].texCoord1, texFace->uv, sizeof(float) * 6);
				if (face->v4)				// �ı���
				{
					memcpy(&triangles[actualNumFace + 1].texCoord1, texFace->uv[0], sizeof(float) * 2);
					memcpy(&triangles[actualNumFace + 1].texCoord2, texFace->uv[2], sizeof(float) * 2);
					memcpy(&triangles[actualNumFace + 1].texCoord3, texFace->uv[3], sizeof(float) * 2);
				}
			}
			//triangles[actualNumFace].showCoords();
			triangles[actualNumFace].mtlId = 0;
			triangles[actualNumFace].buildAccelerateData(actualNumFace);
			if (face->v4)
			{
				triangles[actualNumFace + 1].mtlId = 0;
				triangles[actualNumFace + 1].buildAccelerateData(actualNumFace + 1);
				//triangles[actualNumFace + 1].showCoords();
				actualNumFace += 2;
			}
			else
				actualNumFace ++;
		}		
	}
	nTriangles = actualNumFace;
	// �趨��Χ��
	sceneBox = WBoundingBox(triangles[0]);
	for (unsigned int nthTri = 1; nthTri < nTriangles; nthTri++)
	{
		sceneBox.merge(triangles[nthTri]);
	}
	// ���ز���
	materials=new WMaterial*[1];
	string mtlName;
	mtlName= "default material";
	materials[0]=new WLambertMaterial(mtlName,0,WVector3(1,1,1));
}
#endif
void WScene::drawScene(bool showNormal,bool fillMode)
{
	WTriangle tri;
	WVector3 color;
//  	if(nMaterials!=0)
//  		MessageBox(0,L"0 Mtl",L"aa",0);
	for(unsigned int i=0;i<nPrimitives;i++)
	{
		primitives[i].getTriangle(0,tri);
		color=materials[tri.mtlId]->getColor();
		glColor3f(color.x,color.y,color.z);
		primitives[i].drawPrimitive(showNormal,fillMode);
	}
	for(unsigned int i=0;i<lights.size();i++)
		lights[i]->draw();

}
void WScene::getPrimitives(WPrimitive *&iprimitives, unsigned int &nPrims)
{
		nPrims=nPrimitives;
		iprimitives=this->primitives;
}
void WScene::getNthPrimitive(WPrimitive*&iprimitives,unsigned int nthPrim)
{
	iprimitives=&(primitives[nthPrim]);
}
void WScene::getMaterials(WMaterial**&imaterials,unsigned int&nMtl)
{
	nMtl=nMaterials;
	imaterials=this->materials;
}
void WScene::getNthMaterial(WMaterial*&imaterial,unsigned int nthMtl)
{
	imaterial=materials[nthMtl];
}
void WScene::setNthMaterial(WMaterial*imaterial,unsigned int nthMtl)
{
	if(nthMtl>=0&&nthMtl<nMaterials)
	{
		delete materials[nthMtl];
		materials[nthMtl]=imaterial;
	}
}
void WScene::rebuildAllSubPs(unsigned int inFacesPerSubP)
{
	if(!inFacesPerSubP)
		return;
	nSubPrimitives=0;
	unsigned int n;
	for(unsigned int i=0;i<nPrimitives;i++)
	{
		primitives[i].rebuildSubP(inFacesPerSubP);
		primitives[i].getSubPrimNum(n);
		nSubPrimitives+=n;
	}
	return;
}
void WScene::buildSceneBBox()
{
	if(nPrimitives==0)
		return;
	sceneBox=primitives[0].getBBox();
	for(unsigned int i=1;i<nPrimitives;i++)
	{
		//cout<<"mergebox"<<endl;
		sceneBox.merge(primitives[i].getBBox());
	}
	return;
}
void WScene::drawSceneBBox()
{
	glColor3f(0.1f,0.1f,0.5f);
	sceneBox.draw();
}
void WScene::addLight(WLight *light)
{
	lights.push_back(light);
}
WLight* WScene::getLightPointer(unsigned int nthLight)
{
	return lights[nthLight];
}
unsigned int WScene::getLightNum()
{
	return lights.size();
}
void WScene::clearSelect()
{
	for(unsigned int i=0;i<nPrimitives;i++)
		primitives[i].isSelected=false;
}

void WScene::buildTriangleArray()
{
	unsigned int totalTris=0;
	for(unsigned int nthPrimitive=0;
		nthPrimitive<nPrimitives;nthPrimitive++)
	{
		unsigned int nFaces;
		primitives[nthPrimitive].getFaceNum(nFaces);
		totalTris+=nFaces;
	}
	triangles=new WTriangle[totalTris];
//	nTriangles=totalTris;

	totalTris=0;
	for(unsigned int nthPrimitive=0;
		nthPrimitive<nPrimitives;nthPrimitive++)
	{
		unsigned int nFaces;
		primitives[nthPrimitive].getFaceNum(nFaces);
		for(unsigned int nthFace=0;
			nthFace<nFaces; nthFace++)
		{
			WTriangle face;
			primitives[nthPrimitive].getTriangle(nthFace,face);
			//�����˻�������
			if (face.point1 == face.point2 ||
				face.point2 == face.point3 ||
				face.point3 == face.point1)
			{
				continue;
			}
			triangles[totalTris]=face;
			triangles[totalTris].buildAccelerateData(totalTris);
			totalTris++;
		}
	}
	nTriangles = totalTris;
}

void WScene::drawByTriangleArray( bool showNormal/*=false*/,bool fillMode/*=false*/,WVector3 color/*=WVector3(0,0,0)*/ )
{
	glColor3f(color.x,color.y,color.z);
	for(unsigned int nthTriangle=0;
		nthTriangle<nTriangles;nthTriangle++)
	{
		triangles[nthTriangle].draw(showNormal,fillMode);
	}
}

void WScene::clearTriangleArray()
{
	if (!nTriangles)
	{
		return;
	}
	delete[] triangles;
	triangles=NULL;
	nTriangles = 0;
}

void WScene::getTriangleArray( WTriangle*&itriangles,unsigned int&nTris )
{
	itriangles=triangles;
	nTris=nTriangles;
}

void WScene::getTriAccelArray(float*& array, int& nFloat4s)
{
	nFloat4s = nTriangles * 3;
	array = new float[nFloat4s * 4];
	for (int ithTri = 0; ithTri < nTriangles; ++ithTri)
	{
		TriAccel& tA = triangles[ithTri].tA;
		float* begin = array + ithTri * 12;
		begin[0] = tA.nu;
		begin[1] = tA.nv;
		begin[2] = tA.np;
		begin[4] = tA.pu;
		begin[5] = tA.pv;
		begin[8] = tA.e0u;
		begin[9] = tA.e0v;
		begin[10] = tA.e1u;
		begin[11] = tA.e1v;
		int& u = *((int*)(begin + 3));
		int& v = *((int*)(begin + 6));
		int& w = *((int*)(begin + 7));
		char ci = tA.ci >> 25;
		w = ci & 0x3;			ci >>= 2;
		v = ci & 0x3;			ci >>= 2;
		u = ci & 0x3;
	}
}

void WScene::getMaterialArrayFloat4Uint2( 
										unsigned int*& mtlIDs, unsigned int& nIDPixels, 
										float*& mtlData, unsigned int& nDataPixels )
{
	//�����������صĿռ�
	//������������Ϊuint2����
	nIDPixels = nMaterials;
	mtlIDs = new unsigned int[nIDPixels * 2];
	vector<float> dataVector, temp;
	unsigned int mtlAddress = 0;
	for (unsigned int ithMtl = 0; ithMtl < nMaterials; ithMtl++)
	{
		//mtlIDs��һ����¼��
		//��һ���ǲ��ʵ�����
		//�ڶ����ǲ��ʵ���ʼ��ַ,
		//���ڲ�����������Ϊfloat4���͵�
		//������Ҫ����4��Ҳ����������2λ
		mtlIDs[(ithMtl<<1)] = materials[ithMtl]->type;
		mtlIDs[(ithMtl<<1) + 1] = dataVector.size()>>2;
		//�Ѳ������ݵ���һ���ݴ�������
		materials[ithMtl]->getProperties(temp);
		dataVector.insert(dataVector.end(), temp.begin(), temp.end());
	}
	//���ݴ��������������ʽ���
	nDataPixels = dataVector.size()>>2;
	mtlData = new float[dataVector.size()];
	for (unsigned int ithElement = 0; ithElement < dataVector.size(); 
		ithElement++)
	{
		mtlData[ithElement] = dataVector[ithElement];
	}
}

void WScene::getLightArrayFloat4Uint2( 
									 unsigned int*& lightIDs, unsigned int& nIDPixels, 
									 float*& lightData, unsigned int& nDataPixels )
{
	//�����������صĿռ�
	//������������Ϊuint2����
	nIDPixels = lights.size();
	lightIDs = new unsigned int[nIDPixels * 2];
	vector<float> dataVector, temp;
	unsigned int lightAddress = 0;
	for (unsigned int ithLight = 0; ithLight < lights.size(); ithLight++)
	{
		//lightIDs��һ����¼��
		//��һ���ǲ��ʵ�����
		//�ڶ����ǲ��ʵ���ʼ��ַ,
		//���ڲ�����������Ϊfloat4���͵�
		//������Ҫ����4��Ҳ����������2λ
		lightIDs[ithLight<<1] = lights[ithLight]->type;
		lightIDs[(ithLight<<1) + 1] = dataVector.size()>>2;
		//�Ѳ������ݵ���һ���ݴ�������
		lights[ithLight]->getProperties(temp);
		dataVector.insert(dataVector.end(), temp.begin(), temp.end());
	}
	//���ݴ��������������ʽ���
	nDataPixels = dataVector.size()>>2;
	lightData = new float[dataVector.size()];
	for (unsigned int ithElement = 0; ithElement < dataVector.size(); 
		ithElement++)
	{
		lightData[ithElement] = dataVector[ithElement];
	}
}