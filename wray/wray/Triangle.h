#pragma once
#include "Vector3.h"
#include "Ray.h"
#include "DifferentialGeometry.h"

//ѡ�����ְ취�������εĽ���
//�������������B�ȽϿ�
#define ISECTMETHOD_A 1
#define ISECTMETHOD_B 2
#define INTERSECTION_TEST_METHOD ISECTMETHOD_A
#define INTERSECTION_METHOD      ISECTMETHOD_A

#define TRIACCEL_TRIID_MASK 0x1ffffff
struct TriAccel
{
	float nu, nv, np, pu, pv, e0u, e0v, e1u, e1v;
	/* ci���������ε�������Ϣ����λ�ֲ�����
	 * λ				����
	 * 31				�Ƿ������������
	 * 30 29			u ʵ������
	 * 28 27			v ʵ������
	 * 26 25			w ʵ������
	 * 24- 0			���������
	 */
	unsigned int ci;
	float intersectTest(const WRay&r);
};

struct WRayPacket
{
	__m128 ori[3], dir[3], tMin, tMax;
};

class WTriangle
{
private:
public:
	TriAccel tA;
	WVector3 point1,point2,point3;
	WVector2 texCoord1,texCoord2,texCoord3;
	WVector3 normal1,normal2,normal3;
	unsigned int mtlId;

	WTriangle(
		const WVector3&ipoint1,
		const WVector3&ipoint2,
		const WVector3&ipoint3,
		const WVector2&itexCoord1,
		const WVector2&itexCoord2,
		const WVector2&itexCoord3,
		const WVector3&inormal1,
		const WVector3&inormal2,
		const WVector3&inormal3,
		unsigned int mtlId
		);

	WTriangle(void);
	virtual ~WTriangle(void);

	float area();

	//����������������󽻵ĺ���
	//-----------------------
	//intersectTest�����ཻ��ʱ��Ĺ���tֵ,������ཻ��
	//���ع��ߵ�M_INF_BIG����ʾ������Զ���ཻ
	//���Ե�ʱ��ֻ���ⷵ��ֵ�Ƿ�С��tMax����
	//-----------------------
	//intersect�ٶ��Ѿ�����������߱��н��㣬
	//��������DifferentialGeometry
	//-----------------------
	//����������󽻵�ʱ����ѭ������intersectTest����
	//�õ��������������ཻ�Ķ���Σ��ٵ���intersect����
	//��������DifferentialGeometry,�����¹��ߵ�tMaxֵ
	//DifferentialGeometry�ڴ�����ʱ������ʼ��
	//Ȼ������intersect������ʼ����������
	float intersectTest(const WRay&r);
	__m128 intersectTest(const WRayPacket& rp);
	void intersect(WRay&r,WDifferentialGeometry&DG);
	//��uv������ñ���һ�㼰�䷨����
	//���ڵƹ�Ĳ���
	void getPoint(float u,float v,
		WVector3&position,WVector3&normal,WVector2&texCoord);

	//��openGL�л��������Σ�����ʱ��
	void draw(bool showNormal=false,bool fillMode=false);
	//������������ݣ�����ʱ��
	void showCoords();
	//��������ζ������ݣ�����ʱ��
	void showVertexCoords();
	//��������
	WVector3 getCentroid();
//#if INTERSECTION_METHOD==ISECTMETHOD_B
	void buildAccelerateData(int triID = 0);			// �����������ݽṹ
	TriAccel& getAccelerateData(){return tA;}
//#endif
	//__device__ TriAccel getAccelerateDataGPU(){return tA;}
};


