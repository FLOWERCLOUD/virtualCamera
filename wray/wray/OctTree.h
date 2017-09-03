#pragma once
//���ڰ˲�������Ĳ�����
struct WIrradianceSample
{
	WVector3 E;
	WVector3 normal;
	WVector3 point;
	float maxDist;
	//��ʾ������,����ʱ��
	void display(bool isDisplayBox=false);
};
//��ֵ��
//����Ҫ���в�ֵ����ĵ�ͷ�����Ϣ
//����˲������˲����������в�ֵ����
//�����¼��������
class WIrradianceInterpolator
{
public:
	//���캯�������ø�����ֵ������������λ�ã�������Ϣ
	//����������Ժ���ͨ��refresh��������λ�úͷ�����Ϣ
	WIrradianceInterpolator(
		float imaxNormalError=0.1,
		float imaxPlanarError=0.1,
		float ismoothFactor=2,
		unsigned int iminSamples=10)
	{
		nSamplesAccepted=nSamplesChecked=0;
		totalWeight=0.0f;
		totalE=WVector3(0);
		maxNormalError=imaxNormalError;
//		maxDistanceError=imaxDistanceError;
		maxPlanarError=imaxPlanarError;
		minSamples=iminSamples;
		smoothFactor=ismoothFactor;
	}
	void setInterpolateParams(
		float imaxNormalError=0.1,
		float imaxPlanarError=0.1,
		float ismoothFactor=2.0,
		unsigned int iminSamples=10)
	{
		nSamplesAccepted=nSamplesChecked=0;
		totalWeight=0.0f;
		totalE=WVector3(0);
		maxNormalError=imaxNormalError;
//		maxDistanceError=imaxDistanceError;
		maxPlanarError=imaxPlanarError;
		minSamples=iminSamples;
		smoothFactor=ismoothFactor;
	}

	//��һ���µĵ���в�ֵ
	void refresh(WVector3&iposition,WVector3&inormal)
	{
		position=iposition;normal=inormal;
		nSamplesAccepted=nSamplesChecked=0;
		totalWeight=0;
		totalE=WVector3(0);
	}
	WVector3 getPosition(){return position;}
	//���㵱ǰ���Ƿ��ʺ�ʹ��sample�Ĳ�������в�ֵ
	//����ʺϣ��޸���ز�����������true
	//���򷵻�false
	//�����в����������Ϻ�
	//ͨ��nSamplesAccepted�ж��Ƿ��ҵ����ʵĲ�ֵ�㣬���ҵ���
	//totalE/totalWeight�õ����յ�E
	void operator()(WIrradianceSample&sample);
	//�ж��Ƿ��ֵ�ɹ������������ղ�ֵ���õ�E
	bool finalInterpolate(WVector3&E);
private:
	WVector3 normal;
	WVector3 position;

	float totalWeight;
	WVector3 totalE;

	//�����Ǹ�����ֵ
	//nSamplesAccepted,nSamplesChecked�ֱ���ͨ������������
	//�ͼ�����������
	unsigned int nSamplesAccepted,nSamplesChecked;
	//maxNormalError�Ƿ������н����ҵ���ֵ��ԽС���Խ��
	//maxPlanarError�ǲ����㹲��̶ȵ���ֵ��ԽС����̶�Խ��
	//smoothFactor�ǹ���ƽ�����ɵ���ֵ��Խ��Խƽ��
	float maxNormalError,maxPlanarError,smoothFactor;
	//minSamples��ʾ������Ҫ�Ĳ�ֵ��
	unsigned int minSamples;
};

//�˲����ڵ�
struct WOctNode
{
public:
	//���캯�������ӽڵ�ָ�붼��ΪNULL
	WOctNode();
public:
	vector<WIrradianceSample>samples;
	WOctNode*children[8];
};


//�˲���������irradianceCache����,�԰�������������
class WOctTree
{
public:
	WOctTree(WScene*iscene,unsigned int imaxDepth=10);
	virtual ~WOctTree(void);
	void addSample(const WIrradianceSample&sample);
	//��ʾ�ڵ�İ�Χ�У�����ʱ��
	void displayNodes();
	void displaySamples(bool isDisplayBox=false);
	void initialize(WScene*iscene);

	//������нڵ�
	void clear();
	void setMaxDepth(unsigned int imaxDepth)
	{maxDepth=imaxDepth;}

	//�ڰ˲����ҵ���Ҫ����ĵ���Χ�Ĳ�����
	//�����в�ֵ���㣬�������IrradianceInterpolate��
	//Ȼ��ͨ��IrradianceInterpolate�������Ϣ��
	//�����жϲ�ֵ�Ƿ�ɹ�
	void process(WIrradianceInterpolator&interpolate);
private:
	void displaySample(WOctNode*node,bool isDisplayBox=false);
	//ɾ���ڵ㣬�˺���Ϊ�ݹ����
	void deleteNodes(WOctNode*node);
	//�Ѳ�������ӵ��˲������ʵ�λ��
	void add(
		WOctNode*node,
		const WIrradianceSample&sample,
		WBoundingBox nodeBox, 
		const WBoundingBox&sampleBox, 
		float diagSquared,
		unsigned int depth);
	//�ڰ˲����ҵ���Ҫ����ĵ���Χ�Ĳ�����
	//�����в�ֵ���㣬�������IrradianceInterpolate��
	//Ȼ��ͨ��IrradianceInterpolate�������Ϣ��
	//�����жϲ�ֵ�Ƿ�ɹ�
	void lookUP(
		WOctNode*node,
		WBoundingBox&nodeBox,
		WIrradianceInterpolator&interpolate
		);

	vector<WBoundingBox>boxes;			//�ڵ��Χ�У�����ʱ��
	WOctNode*root;						//���ڵ�
	unsigned int maxDepth;
	WScene*scene;
	WBoundingBox sceneBox;
};
