//*****************************************************************************************//
//**                                                                                     **//
//**                   　  　　　  DxPooling                                             **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_DxPooling_Header
#define Class_DxPooling_Header

#include "DxNNCommon.h"
#define PONUM 2

class DxPooling :public DxNNCommon {

protected:
	int                        com_no = 0;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatureCom = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSOCom[2] = { nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> mInputUpBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mInputBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mOutputBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mOutputReadBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mInErrorUpBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mInErrorBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mOutErrorBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mOutErrorReadBuffer = nullptr;

	CONSTANT_BUFFER_Pooling cb;
	UploadBuffer<CONSTANT_BUFFER_Pooling> *mObjectCB = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> pCS[2] = { nullptr };

	UINT Width;
	UINT Height;
	UINT OutWid;
	UINT OutHei;
	UINT OddNumWid = 0;
	UINT OddNumHei = 0;
	float *input = nullptr;
	float *output = nullptr;
	float *inerror = nullptr;
	float *outerror = nullptr;

	UINT input_outerrOneNum = 0;
	UINT output_inerrOneNum = 0;
	UINT64 input_outerrOneSize = 0;
	UINT64 output_inerrOneSize = 0;
	UINT PoolNum = 1;

	DxPooling() {}
	void ForwardPropagation();
	void BackPropagation();
	void InputResourse();
	void InputErrResourse();
	void CopyOutputResourse();
	void CopyOutputErrResourse();

public:
	DxPooling(UINT width, UINT height, UINT poolNum);
	~DxPooling();
	void SetCommandList(int no);
	void ComCreate();
	void FirstInput(float el, UINT ElNum);
	void Input(float *inArr, UINT arrNum);
	void InputEl(float el, UINT arrNum, UINT ElNum);
	void InputError(float *inArr, UINT arrNum);
	void InputErrorEl(float el, UINT arrNum, UINT ElNum);
	void Query();
	void Training();
	float *Output(UINT arrNum);
	float OutputEl(UINT arrNum, UINT ElNum);
	float *GetError(UINT arrNum);
	float GetErrorEl(UINT arrNum, UINT ElNum);
	UINT GetOutWidth();
	UINT GetOutHeight();
};

#endif