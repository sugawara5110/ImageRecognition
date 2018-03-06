//*****************************************************************************************//
//**                                                                                     **//
//**                   �@  �@�@�@DxNeuralNetwork                                         **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_DxNeuralNetwork_Header
#define Class_DxNeuralNetwork_Header

#include "DxNNCommon.h"

class DxNeuralNetwork :public DxNNCommon {

protected:
	int                        com_no = 0;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatureCom = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSOCom[5] = { nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> mNodeUpBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mNodeBuffer[5] = { nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> mNodeReadBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mWeightUpBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mWeightBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mWeightReadBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mErrorBuffer[5] = { nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> mErrorReadBuffer = nullptr;

	CONSTANT_BUFFER_NeuralNetwork cb;
	UploadBuffer<CONSTANT_BUFFER_NeuralNetwork> *mObjectCB = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> pCS[5] = { nullptr };

	bool firstIn = false;
	float *input = nullptr;
	float *weight = nullptr;
	float *error = nullptr;
	float *output = nullptr;

	UINT Split = 1;

	float *target;
	float learningRate = 0.1f;

	UINT *NumNode = nullptr;//�e�w�̃m�[�h�̐�
	UINT *NumNodeStIndex = nullptr;//�e�w�̃X�^�[�gindex
	UINT nodeNumAll = 0;//�S��
	UINT *NumWeight;//�e�w��weight��
	UINT *NumWeightStIndex = nullptr;//�e�w�̃X�^�[�gindex
	UINT weightNumAll = 0;//�S��
	UINT64 node_byteSize = 0;
	UINT64 weight_byteSize = 0;
	int Depth;

	DxNeuralNetwork() {}
	void InputResourse();
	void ForwardPropagation();
	void InverseQuery();
	void BackPropagation();
	void CopyOutputResourse();
	void CopyWeightResourse();
	void CopyErrorResourse();

public:
	DxNeuralNetwork(UINT *numNode, int depth, UINT split);
	~DxNeuralNetwork();
	void SetCommandList(int no);
	void ComCreate();
	void SetLearningLate(float rate);
	void SetTarget(float *target);
	void SetTargetEl(float el, UINT ElNum);
	void FirstInput(float el, UINT ElNum);
	void InputArray(float *inArr, UINT arrNum);
	void InputArrayEl(float el, UINT arrNum, UINT ElNum);
	void Query();
	void Training();
	void GetOutput(float *out);
	float GetOutputEl(UINT ElNum);
	float *GetError(UINT arrNum);
	float GetErrorEl(UINT arrNum, UINT ElNum);
	void SetInputResource(ID3D12Resource *res);
	ID3D12Resource *GetOutErrorResource();
	void SaveData();
	void LoadData();
};

#endif