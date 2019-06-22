//*****************************************************************************************//
//**                                                                                     **//
//**                                    CNN                                              **//
//**                                                                                     **//
//*****************************************************************************************//
#ifndef Class_CNN_Header
#define Class_CNN_Header

#include "../../..//Common/Direct3DWrapper/Dx_NN.h"

class GradCAM :public DxGradCAM {

protected:
	PolygonData2D dgc;

public:
	GradCAM(UINT srcWid, UINT srcHei, UINT SizeFeatureMapW, UINT SizeFeatureMapH, UINT NumGradientEl,
		UINT NumFil, UINT inputsetnum);
	void Draw(float x, float y);
};

class Affine :public DxNeuralNetwork {

protected:
	PolygonData2D dnn;
	UINT NumFilter;

public:
	Affine(UINT inW, UINT inH, UINT* numNode, int depth, UINT split, UINT inputsetnum);
	void Draw(float x, float y);
};

class Pooling :public DxPooling {

protected:
	PolygonData2D dpo;
	UINT NumFilter;

public:
	Pooling(UINT width, UINT height, UINT poolNum, UINT inputsetnum);
	void Draw(float x, float y);
};

class Convolution :public DxConvolution {

protected:
	PolygonData2D dcn;
	UINT NumFilter;

public:
	Convolution(UINT width, UINT height, UINT filNum, UINT inputsetnum, UINT elnumwid, UINT filstep);
	void Draw(float x, float y);
};

class CNN {

protected:
	UINT NumConv = 0;
	UINT NumPool = 0;
	UINT NumDepth = 0;
	Convolution** cn = nullptr;
	Pooling** po = nullptr;
	Affine* nn = nullptr;
	GradCAM* gc = nullptr;

	void RunConvolutionToPooling(UINT ind);
	void RunPoolingToConvolution(UINT ind);
	void RunPoolingToNN(UINT ind);

	void RunConvolutionToPoolingDetec(UINT ind, UINT SearchNum);
	void RunPoolingToConvolutionDetec(UINT ind, UINT SearchNum);
	void RunPoolingToNNDetec(UINT ind, UINT SearchNum);

	void RunConvolutionToPoolingTest(UINT ind);
	void RunPoolingToConvolutionTest(UINT ind);
	void RunPoolingToNNTest(UINT ind);

	void NNToPoolingBackPropagation(UINT ind);
	void ConvolutionToPoolingBackPropagation(UINT ind);
	void PoolingToConvolutionBackPropagation(UINT ind);

	void query();
	void queryDetec(UINT SearchNum);
	void queryTest();

	CNN() {}

public:
	CNN(UINT srcW, UINT srcH, UINT width, UINT height, UINT* numNode, int depth, UINT NumFilter, UINT SearchMaxNum);
	~CNN();
	void Detection(UINT SearchNum);
	void DetectionGradCAM(UINT SearchNum, UINT srcMapWid, UINT mapslide);
	void SetLearningLate(float nn, float cn);
	void Training();
	void Test();
	void TrainingDraw();
	void GradCAMDraw();
	float GetOutputEl(UINT ElNum, UINT inputsetInd = 0);
	void SetTargetEl(float el, UINT ElNum);
	void FirstInput(float el, UINT ElNum, UINT inputsetInd = 0);
	void SetPixel3ch(ID3D12Resource* pi);
	void SetPixel3ch(BYTE* pi);
	void SaveData();
	void LoadData();
};

#endif