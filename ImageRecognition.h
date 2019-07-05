//*****************************************************************************************//
//**                                                                                     **//
//**                              ImageRecognition                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_ImageRecognition_Header
#define Class_ImageRecognition_Header

#include "../../../Common/Direct3DWrapper/Dx_NN.h"
#include "../../../Common/Direct3DWrapper/DxText.h"
#include "../../../CNN/CNN.h"
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
	SP* sp[3] = { nullptr };
	UINT spInd = 0;
	UINT InW;
	UINT InH;

	CNN* cnn = nullptr;

	UINT Width; //入力画像サイズ
	UINT Height;//入力画像サイズ
	UINT nnWidth;
	UINT nnHeight;
	int Depth;
	UINT filNum = 1;
	float Threshold;//閾値

	PolygonData2D* din;
	UINT*** pixIn = nullptr;
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
	float* target = nullptr;
	float currentTarget = 0.0f;
	int errer = 0;
	int currout = 0;
	int testOut = 0;
	int testOut1 = 0;
	int testCountp = 0;
	int testOutpArr[BADGENUM] = { 0 };
	int testCountn = 0;
	int testOutnArr[BADGENUM] = { 0 };

	BYTE* posImageTrain = nullptr;
	BYTE* negaImageTrain = nullptr;
	BYTE* posImageTest = nullptr;
	BYTE* negaImageTest = nullptr;
	UINT posTraNum, posTestNum, negaTraNum, negaTestNum;

	void searchPixel();
	void LearningImagebyteContrastAdjustment(BYTE* arr, UINT imageNum);
	void searchPixelContrastAdjustment(float* arr, float max, float min);

public:
	ImageRecognition(UINT srcWid, UINT srcHei, UINT width, UINT height, UINT* numNode, int depth, UINT filNum, bool searchOn, float Threshold);
	~ImageRecognition();
	void SetTarget(float* tar);
	void SetLearningNum(UINT texNum, UINT ppmNum);
	void CreateLearningImagebyte(float RateOftrainImage, BYTE* ppm);
	void LearningByteImage();
	void TestByteImage();//Training()後に実行すること
	void InputTexture(int Tno);
	void InputPixel(BYTE* pix);
	void Query();
	void QueryGradCAM();
	void LearningDecay(float in, float scale);
	void Training();
	void Test();
	void NNDraw();
	void INDraw(float x, float y, float xsize, float ysize);
	void SPDraw();
	int Geterrer();
	int Getcurrout();
	int Gettestout();
	int Gettestout1();
	float Getcurrtar();
	float GetcrossEntropyError();
	float GetcrossEntropyErrorTest();
	void textDraw(UINT stateNum, float x, float y);
	void SaveData();
	void LoadData();
	void SaveDataSet();
	bool LoadDataSet();
};

#endif