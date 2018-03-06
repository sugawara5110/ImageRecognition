//*****************************************************************************************//
//**                                                                                     **//
//**                              DxConvolution                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_DxConvolution_Header
#define Class_DxConvolution_Header

#include "DxNNCommon.h"

class DxConvolution :public DxNNCommon {

protected:
	int                        com_no = 0;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatureCom = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSOCom[4] = { nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> mInputUpBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mInputBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mOutputBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mOutputReadBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mInErrorUpBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mInErrorBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mOutErrorBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mOutErrorReadBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mFilterUpBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mFilterBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mFilterReadBuffer = nullptr;

	CONSTANT_BUFFER_Convolution cb;
	UploadBuffer<CONSTANT_BUFFER_Convolution> *mObjectCB = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> pCS[4] = { nullptr };
	UINT Width; //���͉摜�T�C�Y
	UINT Height;//���͉摜�T�C�Y
	UINT OutWid;//�o�͉摜�T�C�Y
	UINT OutHei;//�o�͉摜�T�C�Y
	bool firstIn = false;

	UINT elNumWid = 3;//��̂�Max7
	UINT ElNum = 9;  //elNumWid * elNumWid
	UINT FilNum = 1;
	UINT filterStep = 1;//2�̗ݏ�̂�,Max8�܂�

	UINT filSize = 0;
	UINT input_outerrOneNum = 0;
	UINT output_inerrOneNum = 0;
	UINT64 input_outerrOneSize = 0;
	UINT64 output_inerrOneSize = 0;

	//�􍞂݃t�B���^�[
	float *fil = nullptr;
	//�􍞂ݑO
	float *input = nullptr;
	//�􍞂݌�
	float *output = nullptr;
	//���w����̌덷
	float *inputError = nullptr;
	//���w����̌덷�E�G�C�g�v�Z���w�ɑ��鎞�Ɏg�p
	float *outputError = nullptr;

	float learningRate = 0.1f;

	DxConvolution() {}
	void ForwardPropagation();
	void BackPropagation();
	void InputResourse();
	void InputErrResourse();
	void CopyOutputResourse();
	void CopyOutputErrResourse();
	void CopyFilterResourse();

	void TestFilter();
	void TestInput();
	void TestInErr();
	void TestOutput();
	void TestOutErr();

public:
	DxConvolution(UINT width, UINT height, UINT filNum, UINT elnumwid = 3, UINT filstep = 1);
	~DxConvolution();
	void SetCommandList(int no);
	void ComCreate();
	void Query();
	void Training();
	void SetLearningLate(float rate);
	void FirstInput(float el, UINT ElNum);
	void Input(float *inArr, UINT arrNum);
	void InputEl(float el, UINT arrNum, UINT ElNum);
	void InputError(float *inArr, UINT arrNum);
	float *Output(UINT arrNum);
	float OutputEl(UINT arrNum, UINT ElNum);
	float OutputFilter(UINT arrNum, UINT ElNum);
	float *GetError(UINT arrNum);
	float GetErrorEl(UINT arrNum, UINT ElNum);
	void SetInputResource(ID3D12Resource *res);
	void SetInErrorResource(ID3D12Resource *res);
	ID3D12Resource *GetOutErrorResource();
	ID3D12Resource *GetOutputResource();
	UINT GetOutWidth();
	UINT GetOutHeight();
	void SaveData(UINT Nnm);
	void LoadData(UINT Num);
};

#endif