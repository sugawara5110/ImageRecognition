//*****************************************************************************************//
//**                                                                                     **//
//**                                    CNN                                              **//
//**                                                                                     **//
//*****************************************************************************************//
#ifndef Class_CNN_Header
#define Class_CNN_Header

#include "../../..//Common/Direct3DWrapper/Dx_NN.h"

class CNN;
class Convolution;
class Pooling;

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
	friend CNN;
	Convolution* errCn = nullptr;
	Pooling* errPo = nullptr;
	PolygonData2D dnn;
	UINT NumFilter;

public:
	Affine(UINT inW, UINT inH, UINT* numNode, int depth, UINT split, UINT inputsetnum);
	void Draw(float x, float y);
	void ErrConnection();
};

class Pooling :public DxPooling {

protected:
	friend CNN;
	Affine* inAf = nullptr;
	Convolution* inCn = nullptr;
	Convolution* errCn = nullptr;
	PolygonData2D dpo;
	UINT NumFilter;

public:
	Pooling(UINT width, UINT height, UINT poolNum, UINT inputsetnum);
	void Draw(float x, float y);
	void InConnection();
	void ErrConnection();
	void TestConnection();
	void DetectionConnection(UINT SearchNum);
};

class Convolution :public DxConvolution {

protected:
	friend CNN;
	Affine* inAf = nullptr;
	Pooling* inPo = nullptr;
	Convolution* inCn = nullptr;
	Convolution* errCn = nullptr;
	Pooling* errPo = nullptr;
	PolygonData2D dcn;
	UINT NumFilter;

public:
	Convolution(UINT width, UINT height, UINT filNum, UINT inputsetnum, UINT elnumwid, UINT filstep);
	void Draw(float x, float y);
	void InConnection();
	void ErrConnection();
	void TestConnection();
	void DetectionConnection(UINT SearchNum);
};

enum LayerName {
	CONV,
	POOL,
	AFFINE
};

struct Layer {
	LayerName layerName;
	UINT mapWid;//検出範囲wid
	UINT mapHei;//検出範囲hei
	UINT NumFilter;
	UINT maxThread;
	UINT NumConvFilterWid;//畳み込みフィルターサイズ
	UINT NumConvFilterSlide;//畳み込みフィルタースライド量
	UINT numNode[MAX_DEPTH_NUM - 1];//Affineのノード数(入力層除き)
	UINT NumDepthNotInput;//Affineの深さ(入力除く)
};

class CNN {

protected:
	UINT NumConv = 0;
	UINT NumPool = 0;
	Convolution** cn = nullptr;
	Pooling** po = nullptr;
	Affine* nn = nullptr;
	GradCAM* gc = nullptr;
	UINT layerSize = 0;

	CNN() {}

public:
	CNN(UINT srcW, UINT srcH, Layer* layer, UINT layerSize);
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