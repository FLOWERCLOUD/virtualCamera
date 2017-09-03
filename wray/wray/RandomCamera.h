#pragma once
//������������趨�������ص����ɫ
class WRandomCamera:public WCamera
{
public:
	WRandomCamera(void);
	virtual ~WRandomCamera(void);
	
	//���غ���
/*
	void drawFilmInScreen(
		float offsetX =0.0f,
		float offsetY =0.0f);
	void drawFilmInWorld(
		float offsetX =0.0f,
		float offsetY =0.0f,
		float offsetZ =0.0f);	*/

	//�������Ϊmetropolis�㷨ר�ã�
	//��������ָ�������Ӧ�����ص����ɫ
	//��������ɹ����볡����һ�����������
	//��ȥ������۲�������õ�	
	void setColor(
		float R,float G,float B,
		WVector3 dir);
	//����3�������Ĺ�����Camera����ͬ��
	//����ͬʱҲ�����õļ�¼��������������������
	//��������
	void setFilmResolutionX(unsigned int resX);
	void setFilmResolutionXY(
		unsigned int resX,unsigned int resY);
	void clearFilm(
		float R =0.0f,float G =0.0f,float B =0.0f);
	
	//������Щ����
	void getNextRay(WRay&ray){}
	void setColor(float R,float G,float B){}
	void changeSampleSize(unsigned int size);	//ÿ����������ʼ��Ϊ1
	void changeSampler(WSampler::WSamplerType type){}
	float currProgress(){return 0.0f;}
	bool isFilmFull(){return false;}
	int* getFilmBitPointer(){return NULL;}
	unsigned int getSampleSize(){return 1;}
	
private:
	unsigned int* nSamples;
	unsigned int resolutionX;
	unsigned int resolutionY;
	
};
