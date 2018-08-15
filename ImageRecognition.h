//*****************************************************************************************//
//**                                                                                     **//
//**                              ImageRecognition                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_ImageRecognition_Header
#define Class_ImageRecognition_Header

#include "../../../Common/Direct3DWrapper/Dx_NN.h"
#include "../../../Common/Direct3DWrapper/DxText.h"
#define LEARTEXWID 64
#define BADGENUM 32

class SP {

public:
	SearchPixel * Sp = nullptr;
	float *spPix = nullptr;
	PolygonData2D dsp[2];
	UINT SearchMaxNum;
	UINT  SearchNum;
	bool *Searchflg = nullptr;
	int *SearchOutInd = nullptr;
	UINT Search10cnt = 0;
	float *out = nullptr;

	SP(UINT srcwid, UINT srchei, UINT seawid, UINT seahei, float outscale, UINT step, UINT outNum, float Threshold, bool searchOn);
	~SP();
};

class ImageRecognition : public PolygonData2D {

private:
	DxNeuralNetwork * nn = nullptr;
	DxPooling *po[3] = { nullptr };
	DxConvolution *cn[3] = { nullptr };
	SP *sp[3] = { nullptr };
	UINT spInd = 0;
	UINT InW;
	UINT InH;

	UINT *numN = nullptr;
	UINT Width; //入力画像サイズ
	UINT Height;//入力画像サイズ
	UINT nnWidth;
	UINT nnHeight;
	int Depth;
	UINT filNum = 1;
	UCHAR Type;//C:畳込みプーリングNN, P:プーリングNN, N:NNのみ, D:ディープ, S
	float Threshold;//閾値

	PolygonData2D dnn, dpo[3], dcn[3];
	PolygonData2D *din;
	UINT ***pixIn = nullptr;
	int TexNo = -1;
	UINT SearchMaxNum;
	bool searchon;

	UINT learTexNum = 0;
	UINT positivef = 0;
	UINT texPosNum = 0;
	UINT ppmPosNum = 0;
	UINT posNum = 0;
	UINT negaNum = 0;
	UINT poscnt = 0;
	UINT negacnt = 0;
	float *target = nullptr;
	float currentTarget = 0.0f;
	int errer = 0;
	int currout = 0;
	int testOut = 0;
	int testCountp = 0;
	int testOutpArr[BADGENUM] = { 0 };
	int testCountn = 0;
	int testOutnArr[BADGENUM] = { 0 };

	BYTE *posImageTrain = nullptr;
	BYTE *negaImageTrain = nullptr;
	BYTE *posImageTest = nullptr;
	BYTE *negaImageTest = nullptr;
	UINT posTraNum, posTestNum, negaTraNum, negaTestNum;

	void RunConvolutionToPooling(UINT ind);
	void RunPoolingToConvolution(UINT ind);
	void RunPoolingToNN(UINT ind);
	void RunConvolutionToPoolingDetec(UINT ind);
	void RunPoolingToConvolutionDetec(UINT ind);
	void RunPoolingToNNDetec(UINT ind);
	void RunConvolutionToPoolingTest(UINT ind);
	void RunPoolingToConvolutionTest(UINT ind);
	void RunPoolingToNNTest(UINT ind);
	void NNToPoolingBackPropagation(UINT ind);
	void ConvolutionToPoolingBackPropagation(UINT ind);
	void PoolingToConvolutionBackPropagation(UINT ind);
	void query();
	void queryDetec();
	void queryTest();
	void searchPixel();

public:
	ImageRecognition(UINT srcWid, UINT srcHei, UINT width, UINT height, UINT *numNode, int depth, UINT filNum, UCHAR type, bool searchOn, float Threshold);
	~ImageRecognition();
	void SetTarget(float *tar);
	void SetLearningNum(UINT texNum, UINT ppmNum);
	void CreateLearningImagebyte(float RateOftrainImage, BYTE *ppm);
	void LearningByteImage();
	void TestByteImage();//Training()後に実行すること
	void InputTexture(int Tno);
	void InputPixel(BYTE *pix);
	void Query();
	void LearningDecay(float in, float scale);
	void Training();
	void Test();
	void NNDraw();
	void PODraw();
	void CNDraw();
	void INDraw(float x, float y, float xsize, float ysize);
	void SPDraw();
	int Geterrer();
	int Getcurrout();
	int Gettestout();
	float Getcurrtar();
	void textDraw(UINT stateNum, float x, float y);
	void SaveData();
	void LoadData();
};

#endif