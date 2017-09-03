#include "StdAfx.h"
#include "Clipper.h"

WBoxClipper::WBoxClipper(void)
{
}

WBoxClipper::~WBoxClipper(void)
{
}

WBoxClipper::WBoxClipper( const WBoundingBox& box )
{
	clipBox = box;
	boxArray[0][0] = clipBox.pMin.x;
	boxArray[0][1] = clipBox.pMin.y;
	boxArray[0][2] = clipBox.pMin.z;
	boxArray[1][0] = clipBox.pMax.x;
	boxArray[1][1] = clipBox.pMax.y;
	boxArray[1][2] = clipBox.pMax.z;
}

void WBoxClipper::setClipBox( const WBoundingBox& box )
{
	clipBox = box;
	boxArray[0][0] = clipBox.pMin.x;
	boxArray[0][1] = clipBox.pMin.y;
	boxArray[0][2] = clipBox.pMin.z;
	boxArray[1][0] = clipBox.pMax.x;
	boxArray[1][1] = clipBox.pMax.y;
	boxArray[1][2] = clipBox.pMax.z;
}
void WBoxClipper::setClipBox(float box[2][3])
{
	memcpy((void*)boxArray, (void*)box, sizeof(float)* 6);
	memcpy((void*)clipBox.pMin.v, (void*)box, sizeof(float)*3);
	memcpy((void*)clipBox.pMax.v, (void*)box[1], sizeof(float)*3);
}

void WBoxClipper::displayPolygon( vector<WVector3>& vertices )
{
	glBegin(GL_LINE_LOOP);
	for(unsigned int i = 0; i< vertices.size(); i++)
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
	glEnd();
	glBegin(GL_POINTS);
	for(unsigned int i = 0; i< vertices.size(); i++)
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
	glEnd();
}


bool WBoxClipper::clipTriangle(const WTriangle& tri, vector<WVector3>& outVertices)
{
	// ȷ����Χ�к�����������
	// ���ڰ�Χ�У� 0��ʾx������Ϊ0�� 1��ʾy������Ϊ0��2��ʾz������Ϊ0, 3��ʾһ�����
	// ���������Σ� 0��ʾ��������x����1��ʾ��������y����2��ʾ��������z����3��ʾһ�����
	char boxType = 3, triType = 3;
	for (char i = 0; i < 3; ++i)
	{
		if (boxArray[0][i] == boxArray[1][i])
			boxType = i;
		if (tri.point1.v[i] == tri.point2.v[i] && 
			tri.point2.v[i] == tri.point3.v[i])
			triType = i;
	}

	// �����˳�
	// ע�⣺��������β��������ģ����ڸ��������Ȼ�������Χ���ཻ
	if (boxType < 3 && triType < 3 && 
		(triType != boxType || (triType == boxType && tri.point1.v[triType] != boxArray[0][boxType])))
		return false;

	char order[3] = {0, 1, 2};
	if (triType == 3)
	{
		// �������귶Χ��Сȷ���ü�˳�򣬼�����Ϊ�����������Ĵ���
		float triBox[2][3];
		triBox [0][0] = triBox[1][0] = tri.point1.x;
		triBox [0][1] = triBox[1][1] = tri.point1.y;
		triBox [0][2] = triBox[1][2] = tri.point1.z;

		triBox [0][0] = minf(triBox [0][0], tri.point2.x);
		triBox [0][1] = minf(triBox [0][1], tri.point2.y);
		triBox [0][2] = minf(triBox [0][2], tri.point2.z);
		triBox [1][0] = maxf(triBox [1][0], tri.point2.x);
		triBox [1][1] = maxf(triBox [1][1], tri.point2.y);
		triBox [1][2] = maxf(triBox [1][2], tri.point2.z);

		triBox [0][0] = minf(triBox [0][0], tri.point3.x);
		triBox [0][1] = minf(triBox [0][1], tri.point3.y);
		triBox [0][2] = minf(triBox [0][2], tri.point3.z);
		triBox [1][0] = maxf(triBox [1][0], tri.point3.x);
		triBox [1][1] = maxf(triBox [1][1], tri.point3.y);
		triBox [1][2] = maxf(triBox [1][2], tri.point3.z);

		triBox [0][0] = abs(triBox[0][0] - triBox[1][0]);
		triBox [0][1] = abs(triBox[0][1] - triBox[1][1]);
		triBox [0][2] = abs(triBox[0][2] - triBox[1][2]);
		for (int i = 0; i < 3; ++i)
			for (int j = i + 1; j < 3; ++j)
				if (triBox[0][i] < triBox[0][j])
					swap(order[i], order[j]);
	}
	// ׼������
	// ���������α�AABB�ü��õ��Ķ���α������Ϊ9����׼��һ����СΪ9�Ķ�ά����
	float polygon0[9][3], polygon1[9][3];

	polygon0[0][0] = tri.point1.x;
	polygon0[0][1] = tri.point1.y;
	polygon0[0][2] = tri.point1.z;

	polygon0[1][0] = tri.point2.x;
	polygon0[1][1] = tri.point2.y;
	polygon0[1][2] = tri.point2.z;

	polygon0[2][0] = tri.point3.x;
	polygon0[2][1] = tri.point3.y;
	polygon0[2][2] = tri.point3.z;

	float (*srcPoly)[3] = polygon0, (*dstPoly)[3] = polygon1;
	char numEdges = 3;
	char otherAxisTable[3][2] = {{1,2},{0,2},{0,1}};

	// ��ʼ�ü���ʹ��AABB��6����ü�
	for (char i = 0; i < 6; i++)
	{	// ѭ��˳��Ϊ minX-maxX-minY-maxY-minZ-maxZ
		// ѡ���ü������Լ��ü�λ��
		char axis = order[i >> 1];		// (0 1 2) --> (x y z)   
		char boxPoint = i & 0x1;		// (0 1)   --> (pMin pMax)
		// ����ǰ����Ϊ��Χ�к��Ϊ0�����򣬲����вü�
		if (axis == boxType)
			continue;
		// �ڵ�ǰѡ���������λ�ã��Զ���ν��вü�
		// newEdges��¼�ü���Ķ���α���
		char newNumEdges = 0;
		for (int curPoint = 0; curPoint < numEdges; curPoint++)
		{
			char nextPoint = (curPoint + 1) % numEdges;
			char isCurIn   = ((srcPoly[curPoint][axis] > boxArray[boxPoint][axis]) ^ boxPoint) |
				(srcPoly[curPoint][axis] == boxArray[boxPoint][axis]);
			char isNextIn  = ((srcPoly[nextPoint][axis] > boxArray[boxPoint][axis]) ^ boxPoint) |
				(srcPoly[nextPoint][axis] == boxArray[boxPoint][axis]);
			if (isCurIn & isNextIn)
			{	// ��ǰ�����һ���㶼�ڼ���ƽ���ڣ�Ӧ�ð���һ�������Ŀ������
				dstPoly[newNumEdges][0] = srcPoly[nextPoint][0];
				dstPoly[newNumEdges][1] = srcPoly[nextPoint][1];
				dstPoly[newNumEdges][2] = srcPoly[nextPoint][2];
				++newNumEdges;
			}
			else if (isCurIn & ~isNextIn & (srcPoly[curPoint][axis] != boxArray[boxPoint][axis]))
			{	// һ�������⴩Խ�˼���ƽ�棬���ҵ�ǰ�㲻��ƽ���ϣ�Ӧ�ðѽ������Ŀ������
				float t = (boxArray[boxPoint][axis] - srcPoly[curPoint][axis]) / 
					(srcPoly[nextPoint][axis] - srcPoly[curPoint][axis]);
				// ��ǰ����ֱ�Ӹ�ֵ��������ֹ�������ʹ��ʵ�ʽ��㲻�ڼ���ƽ���ϵ����
				dstPoly[newNumEdges][axis] = boxArray[boxPoint][axis];
				// ���㽻��������������������겢����Ŀ������
				char otherAxis = otherAxisTable[axis][0];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				otherAxis = otherAxisTable[axis][1];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				++newNumEdges;
			}
			else if (~isCurIn & isNextIn)
			{	// һ�������ڴ�Խ�˼���ƽ�棬Ӧ�ðѽ����Լ���һ�������Ŀ������
				float t = (boxArray[boxPoint][axis] - srcPoly[curPoint][axis]) / 
					(srcPoly[nextPoint][axis] - srcPoly[curPoint][axis]);
				// ��ǰ����ֱ�Ӹ�ֵ��������ֹ�������ʹ��ʵ�ʽ��㲻�ڼ���ƽ���ϵ����
				dstPoly[newNumEdges][axis] = boxArray[boxPoint][axis];
				// ���㽻��������������������겢����Ŀ������
				char otherAxis = otherAxisTable[axis][0];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				otherAxis = otherAxisTable[axis][1];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				++newNumEdges;
				// �����һ���㲻�ڼ���ƽ���ϣ�����һ����Ҳ����Ŀ������
				if (srcPoly[nextPoint][axis] != boxArray[boxPoint][axis])
				{
					dstPoly[newNumEdges][0] = srcPoly[nextPoint][0];
					dstPoly[newNumEdges][1] = srcPoly[nextPoint][1];
					dstPoly[newNumEdges][2] = srcPoly[nextPoint][2];
					++newNumEdges;
				}
			}
		}
		numEdges = newNumEdges;
		float (*temp)[3] = srcPoly;
		srcPoly = dstPoly;
		dstPoly = temp;
	}
	// ����Ϊ0˵�������θ�AABB���ཻ
	if (!numEdges)
		return false;
	// �½�����İ�Χ��
	for (char e = 0; e < numEdges; e++)
	{
		outVertices.push_back(WVector3(srcPoly[e][0],srcPoly[e][1],srcPoly[e][2]));
	}
	return true;
}

void WBoxClipper::drawPolygon( vector<WVector3>& vertices )
{
	glBegin(GL_POINTS);
	for (unsigned i = 0; i < vertices.size(); i++)
	{
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
	}
	glEnd();
	glBegin(GL_LINE_LOOP);
	for (unsigned i = 0; i < vertices.size(); i++)
	{
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
//		cout << "vertices " << i << endl;
//		vertices[i].showCoords();
	}
	glEnd();
}

bool WBoxClipper::getClipBox(const WTriangle& tri, float resultBox[2][3])
{
	// ȷ����Χ�к�����������
	// ���ڰ�Χ�У� 0��ʾx������Ϊ0�� 1��ʾy������Ϊ0��2��ʾz������Ϊ0, 3��ʾһ�����
	// ���������Σ� 0��ʾ��������x����1��ʾ��������y����2��ʾ��������z����3��ʾһ�����
	char boxType = 3, triType = 3;
	for (char i = 0; i < 3; ++i)
	{
		if (boxArray[0][i] == boxArray[1][i])
			boxType = i;
		if (tri.point1.v[i] == tri.point2.v[i] && 
			tri.point2.v[i] == tri.point3.v[i])
			triType = i;
	}

	// �����˳�
	// ע�⣺��������β��������ģ����ڸ��������Ȼ�������Χ���ཻ
	if (boxType < 3 && triType < 3 && 
		(triType != boxType || (triType == boxType && tri.point1.v[triType] != boxArray[0][boxType])))
		return false;

	char order[3] = {0, 1, 2};
	if (triType == 3)
	{
		// �������귶Χ��Сȷ���ü�˳�򣬼�����Ϊ�����������Ĵ���
		float triBox[2][3];
		triBox [0][0] = triBox[1][0] = tri.point1.x;
		triBox [0][1] = triBox[1][1] = tri.point1.y;
		triBox [0][2] = triBox[1][2] = tri.point1.z;

		triBox [0][0] = minf(triBox [0][0], tri.point2.x);
		triBox [0][1] = minf(triBox [0][1], tri.point2.y);
		triBox [0][2] = minf(triBox [0][2], tri.point2.z);
		triBox [1][0] = maxf(triBox [1][0], tri.point2.x);
		triBox [1][1] = maxf(triBox [1][1], tri.point2.y);
		triBox [1][2] = maxf(triBox [1][2], tri.point2.z);

		triBox [0][0] = minf(triBox [0][0], tri.point3.x);
		triBox [0][1] = minf(triBox [0][1], tri.point3.y);
		triBox [0][2] = minf(triBox [0][2], tri.point3.z);
		triBox [1][0] = maxf(triBox [1][0], tri.point3.x);
		triBox [1][1] = maxf(triBox [1][1], tri.point3.y);
		triBox [1][2] = maxf(triBox [1][2], tri.point3.z);

		triBox [0][0] = abs(triBox[0][0] - triBox[1][0]);
		triBox [0][1] = abs(triBox[0][1] - triBox[1][1]);
		triBox [0][2] = abs(triBox[0][2] - triBox[1][2]);
		for (int i = 0; i < 3; ++i)
			for (int j = i + 1; j < 3; ++j)
				if (triBox[0][i] < triBox[0][j])
					swap(order[i], order[j]);
	}
	// ׼������
	// ���������α�AABB�ü��õ��Ķ���α������Ϊ9����׼��һ����СΪ9�Ķ�ά����
	float polygon0[9][3], polygon1[9][3];

	polygon0[0][0] = tri.point1.x;
	polygon0[0][1] = tri.point1.y;
	polygon0[0][2] = tri.point1.z;

	polygon0[1][0] = tri.point2.x;
	polygon0[1][1] = tri.point2.y;
	polygon0[1][2] = tri.point2.z;

	polygon0[2][0] = tri.point3.x;
	polygon0[2][1] = tri.point3.y;
	polygon0[2][2] = tri.point3.z;

	float (*srcPoly)[3] = polygon0, (*dstPoly)[3] = polygon1;
	char numEdges = 3;
	char otherAxisTable[3][2] = {{1,2},{0,2},{0,1}};

	// ��ʼ�ü���ʹ��AABB��6����ü�
	for (char i = 0; i < 6; i++)
	{	// ѭ��˳��Ϊ minX-maxX-minY-maxY-minZ-maxZ
		// ѡ���ü������Լ��ü�λ��
		char axis = order[i >> 1];		// (0 1 2) --> (x y z)   
		char boxPoint = i & 0x1;		// (0 1)   --> (pMin pMax)
		// ����ǰ����Ϊ��Χ�к��Ϊ0�����򣬲����вü�
		if (axis == boxType)
			continue;
		// �ڵ�ǰѡ���������λ�ã��Զ���ν��вü�
		// newEdges��¼�ü���Ķ���α���
		char newNumEdges = 0;
		for (int curPoint = 0; curPoint < numEdges; curPoint++)
		{
			char nextPoint = (curPoint + 1) % numEdges;
			char isCurIn   = ((srcPoly[curPoint][axis] > boxArray[boxPoint][axis]) ^ boxPoint) |
				             (srcPoly[curPoint][axis] == boxArray[boxPoint][axis]);
			char isNextIn  = ((srcPoly[nextPoint][axis] > boxArray[boxPoint][axis]) ^ boxPoint) |
				             (srcPoly[nextPoint][axis] == boxArray[boxPoint][axis]);
			if (isCurIn & isNextIn)
			{	// ��ǰ�����һ���㶼�ڼ���ƽ���ڣ�Ӧ�ð���һ�������Ŀ������
				dstPoly[newNumEdges][0] = srcPoly[nextPoint][0];
				dstPoly[newNumEdges][1] = srcPoly[nextPoint][1];
				dstPoly[newNumEdges][2] = srcPoly[nextPoint][2];
				++newNumEdges;
			}
			else if (isCurIn & ~isNextIn & (srcPoly[curPoint][axis] != boxArray[boxPoint][axis]))
			{	// һ�������⴩Խ�˼���ƽ�棬���ҵ�ǰ�㲻��ƽ���ϣ�Ӧ�ðѽ������Ŀ������
				float t = (boxArray[boxPoint][axis] - srcPoly[curPoint][axis]) / 
					(srcPoly[nextPoint][axis] - srcPoly[curPoint][axis]);
				// ��ǰ����ֱ�Ӹ�ֵ��������ֹ�������ʹ��ʵ�ʽ��㲻�ڼ���ƽ���ϵ����
				dstPoly[newNumEdges][axis] = boxArray[boxPoint][axis];
				// ���㽻��������������������겢����Ŀ������
				char otherAxis = otherAxisTable[axis][0];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				otherAxis = otherAxisTable[axis][1];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				++newNumEdges;
			}
			else if (~isCurIn & isNextIn)
			{	// һ�������ڴ�Խ�˼���ƽ�棬Ӧ�ðѽ����Լ���һ�������Ŀ������
				float t = (boxArray[boxPoint][axis] - srcPoly[curPoint][axis]) / 
					(srcPoly[nextPoint][axis] - srcPoly[curPoint][axis]);
				// ��ǰ����ֱ�Ӹ�ֵ��������ֹ�������ʹ��ʵ�ʽ��㲻�ڼ���ƽ���ϵ����
				dstPoly[newNumEdges][axis] = boxArray[boxPoint][axis];
				// ���㽻��������������������겢����Ŀ������
				char otherAxis = otherAxisTable[axis][0];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				otherAxis = otherAxisTable[axis][1];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				++newNumEdges;
				// �����һ���㲻�ڼ���ƽ���ϣ�����һ����Ҳ����Ŀ������
				if (srcPoly[nextPoint][axis] != boxArray[boxPoint][axis])
				{
					dstPoly[newNumEdges][0] = srcPoly[nextPoint][0];
					dstPoly[newNumEdges][1] = srcPoly[nextPoint][1];
					dstPoly[newNumEdges][2] = srcPoly[nextPoint][2];
					++newNumEdges;
				}
			}
		}
		numEdges = newNumEdges;
		float (*temp)[3] = srcPoly;
		srcPoly = dstPoly;
		dstPoly = temp;
	}
	// ����Ϊ0˵�������θ�AABB���ཻ
	if (!numEdges)
		return false;

	// �½�����İ�Χ��
	resultBox[0][0] = resultBox[1][0] = srcPoly[0][0];
	resultBox[0][1] = resultBox[1][1] = srcPoly[0][1];
	resultBox[0][2] = resultBox[1][2] = srcPoly[0][2];
	for (char e = 1; e < numEdges; e++)
	{
		resultBox[0][0] = minf(resultBox[0][0], srcPoly[e][0]);
		resultBox[0][1] = minf(resultBox[0][1], srcPoly[e][1]);
		resultBox[0][2] = minf(resultBox[0][2], srcPoly[e][2]);
		resultBox[1][0] = maxf(resultBox[1][0], srcPoly[e][0]);
		resultBox[1][1] = maxf(resultBox[1][1], srcPoly[e][1]);
		resultBox[1][2] = maxf(resultBox[1][2], srcPoly[e][2]);
	}
	// ��ֹ�������½����Χ�г����ü���Χ��
	resultBox[0][0] = maxf(resultBox[0][0], boxArray[0][0]);
	resultBox[0][1] = maxf(resultBox[0][1], boxArray[0][1]);
	resultBox[0][2] = maxf(resultBox[0][2], boxArray[0][2]);
	resultBox[1][0] = minf(resultBox[1][0], boxArray[1][0]);
	resultBox[1][1] = minf(resultBox[1][1], boxArray[1][1]);
	resultBox[1][2] = minf(resultBox[1][2], boxArray[1][2]);
	return true;
}
