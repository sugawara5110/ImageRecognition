//*****************************************************************************************//
//**                                                                                     **//
//**                              ImageRecognition                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#include "NeuralNetwork\NeuralNetwork.h"
#include "NeuralNetwork\Pooling.h"
#include "NeuralNetwork\ConvolutionNN.h"
#include "Direct3DWrapper\Dx12Process.h"
#include "Direct3DWrapper/DxText.h"
#include "Direct3DWrapper\Dx_NN.h"

class ImageRecognition : public PolygonData2D {

private:
	DxNeuralNetwork *nn = nullptr;
	DxPooling *po[2] = { nullptr };
	DxConvolution *cn[2] = { nullptr };
	UINT InW;
	UINT InH;
	SearchPixel *sp = nullptr;
	float *spPix = nullptr;
	PolygonData2D dsp[2];

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
	float *out = nullptr;
	UINT SearchNum;
	bool InTex = false;

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

public:
	ImageRecognition(UINT srcWid, UINT srcHei, UINT width, UINT height, UINT *numNode, int depth, UINT filNum, UCHAR type, bool searchOn, float Threshold);
	~ImageRecognition();
	void SetTargetEl(float el, unsigned int Num);
	void LearningTexture(int Tno, int dir);
	void searchPixel(int Tno);
	void InputPixel(UINT **pix, UINT width, UINT height);
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

