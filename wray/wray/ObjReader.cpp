#include "stdafx.h"
#include "ObjReader.h"
WObjReader::WObjReader()
{
//	readingThread=NULL;
}
WObjReader::~WObjReader()
{
	this->clear();
}
void WObjReader::readObjFile(const char*fileName)
{
	char*flag=new char[50];		//array to receive input		
	bool isPrimitiveBegin=true;	//the first vertex of each primitive
	WObjPrimitive*currPrimitive=NULL;//pointer to current primitive
	WObjTriangle*currTriangle=NULL;//pointer to current triangle
	Wfloat3 threeDCoor;			//3D coordinate
	Wfloat2 twoDCoor;			//2D coordinate
	int currentMtlIndex=-1;		//the index of current Material
	char* pointIndex=new char[20];
	int pointIndexInt;
	string point;
	stringstream pointBuffer;
	bool isGeometryBegin=false;
	unsigned int totalLength;	//�ļ��ܳ���
	//����maya��max������obj��ʽ��һ��
	//��ͬ��maya��ʽ��ÿ��primitive�ĵ�һ��v֮ǰ������һ��g���
	//��maxû�У������������vt���֮��isGeometryBegin�ſ�����
	//ȷ������������ʾ�������Ƶ�g��ǲŰ��������Ƽ�¼����
	int totalVertices=0,totalTexcoords=0,totalNormals=0;
	file.clear();
	file.open((fileName),ios::in);
	if(!file)
	{
		MessageBox(NULL,_T("�Ҳ����ļ�"),_T("����"),0);
		return;
	}
	file.seekg(0,ios::end);
	totalLength=file.tellg();
	file.seekg(0,ios::beg);
	int checkCounter = 1;
	while(!file.eof())
	{
		checkCounter--;
		if(!checkCounter)
		{
			// ÿ��OBJ_READ_CHECK_COUNTER�б���һ�ν���
			checkCounter = OBJ_READ_CHECK_COUNTER;
			readingProcess=float(file.tellg()* 100.0f)/totalLength ;
			cout<<"file reading progress:"<<readingProcess<<"%     \r";
		}
		file>>flag;

		//2014-12-13-ignore the "#"information
		if(!strcmp(flag,"#"))
		{
			//file.ignore(std::numeric_limits<std::streamsize>::max(), '\n' );
			file.ignore(1e2,'\n');
			continue;
		}

	//	cout<<"flag"<<flag<<endl;
		//cout<<flag<<strcmp(flag,"v")<<endl;
		if(!strcmp(flag,"v"))//vertex flag
		{
			//a new primitive must begin with "v"
			//when encounter the first "v" of a primitive 
			//isPrimitiveBegin is true
			if(isPrimitiveBegin)
			{
				if(currPrimitive)
				{
					primitives.push_back(*currPrimitive);
					totalVertices+=currPrimitive->vertices.size();
					totalTexcoords+=currPrimitive->texcoords.size();
					totalNormals+=currPrimitive->normals.size();
					delete currPrimitive;
				}
				currPrimitive=new WObjPrimitive;
				isPrimitiveBegin=false;
			}
			file>>threeDCoor.x
				>>threeDCoor.y
				>>threeDCoor.z;//vertex coordinate
			currPrimitive->vertices.push_back(threeDCoor);
			//cout<<threeDCoor.x<<threeDCoor.y<<threeDCoor.z<<endl;
		}
		else
		{
			isPrimitiveBegin=true;
			if(!strcmp(flag,"vt"))//vertex texcoord flag
			{
				isGeometryBegin=true;
				file>>twoDCoor.u
					>>twoDCoor.v;
				currPrimitive->texcoords.push_back(twoDCoor);
				//cout<<twoDCoor.u<<twoDCoor.v<<endl;
			}
			else if(!strcmp(flag,"vn"))//vertex normal flag
			{
				file>>threeDCoor.x
					>>threeDCoor.y
					>>threeDCoor.z;
				currPrimitive->normals.push_back(threeDCoor);
				//cout<<threeDCoor.x<<threeDCoor.y<<threeDCoor.z<<endl;
			}
			else if(!strcmp(flag,"g"))//primitive name flag
			{
				file>>flag;string a;
				if(isGeometryBegin)
				{
					currPrimitive->name=flag;
					isGeometryBegin=false;
				}
				//cout<<currPrimitive->name<<endl;
			}
			else if(!strcmp(flag,"usemtl"))//material flag
			{				
				string mtlName;
				vector<WObjMaterial>::iterator mtlIterator;
				WObjMaterial*newMaterial;
				currentMtlIndex=-1;
				//update current material
				file>>mtlName;
				for(mtlIterator=Materials.begin();
					mtlIterator!=Materials.end();
					mtlIterator++)
					//find if there is an existing material with the 
					//same name
				{
					currentMtlIndex++;
					if(mtlIterator->name==mtlName)
						break;//find
				}
				if(mtlIterator==Materials.end())
					//can't find,so create a new material
				{
					newMaterial=new WObjMaterial;
					newMaterial->name=mtlName;
					Materials.push_back(*newMaterial);
					delete newMaterial;
					currentMtlIndex=Materials.size()-1;
				}
				//cout<<Materials[0].name<<endl;
			}
			else if (!strcmp(flag,"f"))//face flag
			{
				currTriangle=new WObjTriangle;
				char line[100];
				file.getline(line,100);
				int idxV[4],idxVt[4],idxVn[4];

				char part[4][30];
				int nPart = sscanf(line, "%s%s%s%s", part[0], part[1], part[2], part[3]);
				int nTri = nPart == 3 ? 1 : 2;
				for (int i = 0; i < nPart; ++i)
				{
					int nNum = sscanf(part[i], "%d/%d/%d", &idxV[i], &idxVt[i], &idxVn[i]);
					if (nNum != 3)
					{
						nNum = sscanf(part[i], "%d//%d", &idxV[i], &idxVn[i]);
						idxVt[i] = 0;
						if (nNum != 2)
						{
							nNum = sscanf(part[i], "%d/%d", &idxV[i], &idxVt[i]);
							idxVn[i] = 0;
							if (nNum != 2)
							{
								nNum = sscanf(part[i], "%d", &idxV[i]);
								idxVt[i] = idxVn[i] = 0;
							}
						}
					}
					idxV[i]  = idxV[i]  >0 ? idxV[i]  - 1 - totalVertices  : currPrimitive->vertices.size()  + idxV[i]; 
					idxVt[i] = idxVt[i] >0 ? idxVt[i] - 1 - totalTexcoords : currPrimitive->texcoords.size() + idxVt[i]; 
					idxVn[i] = idxVn[i] >0 ? idxVn[i] - 1 - totalNormals   : currPrimitive->normals.size()   + idxVn[i]; 
				}
				
				// һ��������
				WObjTriangle tri = {
					{idxV[0],idxV[1],idxV[2]},
					{idxVt[0],idxVt[1],idxVt[2]},
					{idxVn[0],idxVn[1],idxVn[2]},
					currentMtlIndex
				};
				currPrimitive->faces.push_back(tri);
				if (nTri == 2)
				{
					WObjTriangle tri = {
						{idxV[2],idxV[3],idxV[0]},
						{idxVt[2],idxVt[3],idxVt[0]},
						{idxVn[2],idxVn[3],idxVn[0]},
						currentMtlIndex
					};
					currPrimitive->faces.push_back(tri);
				}
			}
		}
	}
	primitives.push_back(*currPrimitive);
	file.close();
	file.clear();
	//������ļ�״̬�ֻ����ó�endofbit��clear�����ָ�״̬��Ϊgoodbit
	delete[] flag;
	delete[] pointIndex;
	cout<<"complete to read obj file            \n"<<endl;
	cout<<endl;
}
bool WObjReader::getWireArray(int nthPrimitive,WGLWireArray*vB)
{
	if(nthPrimitive<0||nthPrimitive>=(int)this->primitives.size())
		return false;

	//vertices
	int nVertices=(int)this->primitives[nthPrimitive].vertices.size();
	(*vB).vertices=new float[3*nVertices];
	for(int i=0;i<nVertices;i++)
	{
		(*vB).vertices[3*i]=this->primitives[nthPrimitive].vertices[i].x;
		(*vB).vertices[3*i+1]=this->primitives[nthPrimitive].vertices[i].y;
		(*vB).vertices[3*i+2]=this->primitives[nthPrimitive].vertices[i].z;
	}

	//indices
	vB->nFaces=(int)this->primitives[nthPrimitive].faces.size();
	int nVertexIndices=3*vB->nFaces;
	vB->indices=new unsigned int[nVertexIndices];
	for(int i=0;i<nVertexIndices/3;i++)
	{
		vB->indices[i*3]=this->primitives[nthPrimitive].faces[i].vertIndex[0];
		vB->indices[i*3+1]=this->primitives[nthPrimitive].faces[i].vertIndex[1];
		vB->indices[i*3+2]=this->primitives[nthPrimitive].faces[i].vertIndex[2];
	}
	return true;
}
void WObjReader::showObj()
{
	vector<WObjPrimitive>::iterator itPrimitive;
	vector<Wfloat3>::iterator itFloat3;
	vector<Wfloat2>::iterator itFloat2;
	vector<WObjTriangle>::iterator itTriangle;

	for(itPrimitive=primitives.begin();
		itPrimitive!=primitives.end();
		itPrimitive++)
	{
		cout<<"\n##################"<<endl;
		cout<<"Object Name:"+itPrimitive->name<<endl;
		cout<<"##################"<<endl;
		cout<<"\nvertices:"<<endl;
		for(itFloat3=itPrimitive->vertices.begin();
			itFloat3!=itPrimitive->vertices.end();
			itFloat3++)
		{
			cout<<itFloat3->x<<'\t'
				<<itFloat3->y<<'\t'
				<<itFloat3->z<<'\t'<<endl;
		}
		cout<<"\nnormals:"<<endl;
		for(itFloat3=itPrimitive->normals.begin();
			itFloat3!=itPrimitive->normals.end();
			itFloat3++)
		{
			cout<<itFloat3->x<<'\t'
				<<itFloat3->y<<'\t'
				<<itFloat3->z<<'\t'<<endl;
		}
		cout<<"\ntexcoords:"<<endl;
		for(itFloat2=itPrimitive->texcoords.begin();
			itFloat2!=itPrimitive->texcoords.end();
			itFloat2++)
		{
			cout<<itFloat2->u<<'\t'
				<<itFloat2->v<<'\t'<<endl;
		}
		cout<<"\nfaces:"<<endl;
		for(itTriangle=itPrimitive->faces.begin();
			itTriangle!=itPrimitive->faces.end();
			itTriangle++)
		{
			for(int i=0;i<3;i++)
			{
				cout<<itTriangle->vertIndex[i]<<'/'
					<<itTriangle->texcoordIndex[i]<<'/'
					<<itTriangle->norIndex[i]<<'\t';
			}
			cout<<"materialID:"<<itTriangle->mtlIndex<<"  material:"
				<<Materials[itTriangle->mtlIndex].name<<endl;
		}

	}
}
void WObjReader::clear()
{
	vector<WObjPrimitive>::iterator pP;
	for(pP=primitives.begin();pP<primitives.end();pP++)
	{
		pP->faces.clear();
		pP->normals.clear();
		pP->texcoords.clear();
		pP->vertices.clear();
	}
	primitives.clear();
	Materials.clear();
}

void WObjReader::getCoordArray(float*&pArray,unsigned int&nFloats,unsigned nthPrimitive,coordType type)
{
	unsigned int num;
	switch(type)
	{
	case VERTCOORDS:
		num=primitives[nthPrimitive].vertices.size();//����
		nFloats=num*3;//������
		pArray=new float[nFloats];
		for(unsigned int i=0;i<num;i++)
		{
			pArray[3*i]=primitives[nthPrimitive].vertices[i].x;
			pArray[3*i+1]=primitives[nthPrimitive].vertices[i].y;
			pArray[3*i+2]=primitives[nthPrimitive].vertices[i].z;
		}
		break;
	case TEXCOORDS:
		num=primitives[nthPrimitive].texcoords.size();//��ͼ������
		nFloats=num*2;//������
		pArray=new float[nFloats];
		for(unsigned int i=0;i<num;i++)
		{
			pArray[2*i]=primitives[nthPrimitive].texcoords[i].u;
			pArray[2*i+1]=primitives[nthPrimitive].texcoords[i].v;
		}
		break;
	case NORCOORDS:
		num=primitives[nthPrimitive].normals.size();//��������
		nFloats=num*3;//������
		pArray=new float[nFloats];
		for(unsigned int i=0;i<num;i++)
		{
			pArray[3*i]=primitives[nthPrimitive].normals[i].x;
			pArray[3*i+1]=primitives[nthPrimitive].normals[i].y;
			pArray[3*i+2]=primitives[nthPrimitive].normals[i].z;
		}
		break;
	default:
		nFloats=0;
		pArray=NULL;
	}
	return;
}
void WObjReader::getIndexArray(unsigned int*&pArray,unsigned int &nInts,
				   unsigned nthPrimitive,indexType type)
{
	unsigned int num;
	switch(type)
	{
	case VERTINDICES:
		num=primitives[nthPrimitive].faces.size();//����
		nInts=num*3;//������
		pArray=new unsigned int[nInts];
		for(unsigned int i=0;i<num;i++)
		{
			pArray[i*3]=primitives[nthPrimitive].faces[i].vertIndex[0];//��һ�����������
			pArray[i*3+1]=primitives[nthPrimitive].faces[i].vertIndex[1];//�ڶ������������
			pArray[i*3+2]=primitives[nthPrimitive].faces[i].vertIndex[2];//���������������
		}
		break;
	case TEXINDICES:
		num=primitives[nthPrimitive].faces.size();//����
		nInts=num*3;//������
		pArray=new unsigned int[nInts];
		for(unsigned int i=0;i<num;i++)
		{
			pArray[i*3]=primitives[nthPrimitive].faces[i].texcoordIndex[0];//��һ�����������
			pArray[i*3+1]=primitives[nthPrimitive].faces[i].texcoordIndex[1];//�ڶ������������
			pArray[i*3+2]=primitives[nthPrimitive].faces[i].texcoordIndex[2];//���������������
		}
		break;
	case NORINDICES:
		num=primitives[nthPrimitive].faces.size();//����
		nInts=num*3;//������
		pArray=new unsigned int[nInts];
		for(unsigned int i=0;i<num;i++)
		{
			pArray[i*3]=primitives[nthPrimitive].faces[i].norIndex[0];//��һ�����������
			pArray[i*3+1]=primitives[nthPrimitive].faces[i].norIndex[1];//�ڶ������������
			pArray[i*3+2]=primitives[nthPrimitive].faces[i].norIndex[2];//���������������
		}
		break;
	}
	return;
}
void WObjReader::fillPrimitive(unsigned int nthPrimitive,WPrimitive&pri)
{
	float*vertices,*texcoords,*normals;
	unsigned int*vertIndices,*texcoordIndices,*normalIndices;
	unsigned int vSize,tSize,nSize;//��Ӧ����Ĵ�С
	unsigned int nIndices;
	getCoordArray(vertices,vSize,nthPrimitive,WObjReader::VERTCOORDS);
	getCoordArray(texcoords,tSize,nthPrimitive,WObjReader::TEXCOORDS);
	getCoordArray(normals,nSize,nthPrimitive,WObjReader::NORCOORDS);
	getIndexArray(vertIndices,nIndices,nthPrimitive,WObjReader::VERTINDICES);
	getIndexArray(texcoordIndices,nIndices,nthPrimitive,WObjReader::TEXINDICES);
	getIndexArray(normalIndices,nIndices,nthPrimitive,WObjReader::NORINDICES);
	pri.vertices=vertices;
	pri.texcoords=texcoords;
	pri.normals=normals;
	pri.vertexIndices=vertIndices;
	pri.normalIndices=normalIndices;
	pri.texCoordIndices=texcoordIndices;
	pri.nVertices=vSize/3;
	pri.nTexcoords=tSize/2;
	pri.nNormals=nSize/3;
	pri.nFaces=nIndices/3;

	pri.mtlIndices=new unsigned int[pri.nFaces];
	for(unsigned int i=0;i<pri.nFaces;i++)
	{
		pri.mtlIndices[i]=primitives[nthPrimitive].faces[i].mtlIndex;
	}
	pri.buildSubP();
	pri.buildBBox();

}

void WObjReader::readMtlFile(const char*fileName)
{
	char flag[50];

	file.open(fileName,ios::in);
	if(!file)
	{
		cout<<"error occurs when open mtl file "<<endl;
		return;
	}
	while(!file.eof())
	{
		char subFlag[20];
		file>>flag;

		if(!strcmp(flag,"newmtl"))
		{
			string mtlName;	
			file>>mtlName;
			WObjMaterial newMtl;
			newMtl.name=mtlName;
			newMtl.emission.x = newMtl.emission.y = newMtl.emission.z = 0;
			newMtl.specular = 0;
			Materials.push_back(newMtl);
		}
		//��������ɫ��Ӧ��flag
		if(!strcmp(flag,"Kd"))
		{
			WObjMaterial& newMtl = Materials.back();
			file>>newMtl.diffuse.x
				>>newMtl.diffuse.y
				>>newMtl.diffuse.z;
		}
		//�Է�����ɫ��Ӧ��flag(�����˻�����ͨ��)
		if(!strcmp(flag,"Ke"))
		{
			WObjMaterial& newMtl = Materials.back();
			float x,y,z;
			file >> x >> y >> z;
			newMtl.emission.x = x * 5;
			newMtl.emission.y = y * 5;
			newMtl.emission.z = z * 5;
		}
		else if(!strcmp(flag,"Tf"))
		{
			WObjMaterial& newMtl = Materials.back();
			float x,y,z;
			file >> x >> y >> z;
			newMtl.transparency.x = x;
			newMtl.transparency.y = y;
			newMtl.transparency.z = z;
		}
		else if(!strcmp(flag,"Ks"))
		{
			WObjMaterial& newMtl = Materials.back();
			float x,y,z;
			file >> x >> y >> z;
			newMtl.specular = (x+y+z)/3;
		}
	}
	file.close();	file.clear();	
	cout<<"complete to read mtl file "<<endl;
	//������ļ�״̬�ֻ����ó�endofbit��clear�����ָ�״̬��Ϊgoodbit
}

void WObjReader::showMtl()
{
	vector<WObjMaterial>::iterator p;
	for(p=Materials.begin();p!=Materials.end();p++)
	{
		cout<<"\n##################"<<endl;
		cout<<"Material Name:"+p->name<<endl;
		cout<<"##################"<<endl;
		cout<<"diffuse color: "
			<<p->diffuse.x<<'\t'
			<<p->diffuse.y<<'\t'
			<<p->diffuse.z<<endl;
	}
}

void WObjReader::readFile(CString fileName)
{
// 	string name;
// 	name=fileName.substr(0,fileName.length()-4);
// 	string objName,mtlName;
// 	objName=name+".obj";
// 	mtlName=name+".mtl";

// 	strncpy(objName2,fileName,30);
	fileName=fileName.Mid(0,fileName.GetLength()-3);
	CString objName=fileName+CString("obj");
	CString mtlName=fileName+CString("mtl");


	char*file=
		(char*)mtlName.GetBuffer(mtlName.GetLength());
 	readMtlFile(file);
//	MessageBox(0,LPCWSTR(file),0,0);

	file=
		(char*)objName.GetBuffer(objName.GetLength());
//	MessageBox(0,LPCWSTR(file),0,0);
 	readObjFile(file);
//	clear();
}