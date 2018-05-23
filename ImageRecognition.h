//*****************************************************************************************//
//**                                                                                     **//
//**                              ImageRecognition                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#include "../../../Common/Direct3DWrapper/Dx_NN.h"
#include "../../../Common/Direct3DWrapper/DxText.h"

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
	DxPooling *po[2] = { nullptr };
	DxConvolution *cn[2] = { nullptr };
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
	UCHAR Type;//C:畳込みプーリングNN, P:プーリングNN, N:NNのみ, D:ディープ
	float Threshold;//閾値

	PolygonData2D dnn, dpo[2], dcn[2];
	PolygonData2D *din;
	UINT ***pixIn = nullptr;
	int TexNo = -1;
	UINT SearchMaxNum;

	UINT learTexNum = 0;
	UINT learTexInd = 0;
	UINT *learTexsepInd = nullptr;
	UINT *learTexsepNum = nullptr;
	float *target = nullptr;

	void RunConvolutionToPooling(UINT ind);
	void RunPoolingToConvolution();
	void RunPoolingToNN(UINT ind);
	void RunConvolutionToPoolingDetec(UINT ind);
	void RunPoolingToConvolutionDetec();
	void RunPoolingToNNDetec(UINT ind);
	void NNToPoolingBackPropagation(UINT ind);
	void ConvolutionToPoolingBackPropagation();
	void PoolingToConvolutionBackPropagation(UINT ind);
	void query();
	void queryDetec();
	void searchPixel();

public:
	ImageRecognition(UINT srcWid, UINT srcHei, UINT width, UINT height, UINT *numNode, int depth, UINT filNum, UCHAR type, bool searchOn, float Threshold);
	~ImageRecognition();
	void SetTarget(float *tar);
	void SetLearningNum(UINT num);
	void LearningTexture();
	void InputTexture(int Tno);
	void InputPixel(BYTE *pix);
	void Query();
	void Training();
	void NNDraw();
	void PODraw();
	void CNDraw();
	void INDraw(float x, float y, float xsize, float ysize);
	void SPDraw();
	void textDraw(UINT stateNum, float x, float y);
	void SaveData();
	void LoadData();
};

