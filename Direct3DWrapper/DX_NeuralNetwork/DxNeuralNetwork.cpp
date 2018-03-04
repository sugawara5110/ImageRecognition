//*****************************************************************************************//
//**                                                                                     **//
//**                   　  　　  　DxNeuralNetwork                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "DxNeuralNetwork.h"
#include <random>
#include "ShaderNN\ShaderNeuralNetwork.h"

DxNeuralNetwork::DxNeuralNetwork(UINT *numNode, int depth, UINT split) {

	Split = split;
	NumNode = numNode;
	NumNode[0] *= Split;
	if (depth > 5)Depth = 5;
	else
		Depth = depth;
	if (NumNode[Depth - 1] > 10)NumNode[Depth - 1] = 10;

	NumNodeStIndex = new UINT[Depth];
	NumWeight = new UINT[Depth - 1];
	NumWeightStIndex = new UINT[Depth - 1];

	UINT cnt = 0;
	for (int i = 0; i < Depth; i++) {
		NumNodeStIndex[i] = cnt;
		cnt += NumNode[i];
	}
	nodeNumAll = cnt;
	cnt = 0;
	for (int i = 0; i < Depth - 1; i++) {
		NumWeightStIndex[i] = cnt;
		cnt += NumNode[i] * NumNode[i + 1];
		NumWeight[i] = NumNode[i] * NumNode[i + 1];
	}
	weightNumAll = cnt;

	node = new float[nodeNumAll];
	error = new float[nodeNumAll];
	weight = new float[weightNumAll];

	for (UINT i = 0; i < nodeNumAll; i++) {
		node[i] = 0.0f;
		error[i] = 0.0f;
	}

	inverse = new float[NumNode[0]];
	output = new float[NumNode[Depth - 1]];
	target = new float[NumNode[Depth - 1]];
	for (UINT i = 0; i < NumNode[Depth - 1]; i++)target[i] = 0.0f;

	std::random_device seed_gen;
	std::default_random_engine engine(seed_gen());
	for (int i = 0; i < 5; i++) {
		if (NumNode[i] < 1)NumNode[i] = 1;
	}
	//ウエイト初期値
	// 平均0.0、標準偏差pow(NumNode[], -0.5)で分布させる
	std::normal_distribution<> dist0(0.0, pow(NumNode[1], -0.5));
	std::normal_distribution<> dist1(0.0, pow(NumNode[2], -0.5));
	std::normal_distribution<> dist2(0.0, pow(NumNode[3], -0.5));
	std::normal_distribution<> dist3(0.0, pow(NumNode[4], -0.5));

	for (int k = 0; k < Depth - 1; k++) {
		for (UINT i = 0; i < NumWeight[k]; i++) {
			double rnd;
			switch (k) {
			case 0:
				rnd = dist0(engine);
				break;
			case 1:
				rnd = dist1(engine);
				break;
			case 2:
				rnd = dist2(engine);
				break;
			case 3:
				rnd = dist3(engine);
				break;
			}
			weight[NumWeightStIndex[k] + i] = (float)rnd;
		}
	}

	dx = Dx12Process::GetInstance();
	mCommandList = dx->dx_sub[0].mCommandList.Get();
	mObjectCB = new UploadBuffer<CONSTANT_BUFFER_NeuralNetwork>(dx->md3dDevice.Get(), 1, true);
}

DxNeuralNetwork::~DxNeuralNetwork() {

	ARR_DELETE(output);
	ARR_DELETE(target);
	ARR_DELETE(inverse);

	ARR_DELETE(node);
	ARR_DELETE(error);
	ARR_DELETE(weight);

	ARR_DELETE(NumNodeStIndex);
	ARR_DELETE(NumWeight);
	ARR_DELETE(NumWeightStIndex);

	S_DELETE(mObjectCB);
}

void DxNeuralNetwork::SetCommandList(int no) {
	com_no = no;
	mCommandList = dx->dx_sub[com_no].mCommandList.Get();
}

void DxNeuralNetwork::ComCreate() {

	for (int i = 0; i < Depth; i++) {
		cb.NumNode[i].as((float)NumNode[i], NumNodeStIndex[i], 0.0f, 0.0f);
	}
	for (int i = 0; i < Depth - 1; i++) {
		cb.NumWeight[i].x = NumWeightStIndex[i];
	}
	cb.Lear_Depth.z = (float)Depth - 1;

	node_byteSize = nodeNumAll * sizeof(float);
	weight_byteSize = weightNumAll * sizeof(float);

	//RWStructuredBuffer用gNode
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(node_byteSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mNodeBuffer));
	//RWStructuredBuffer用gWeight
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(weight_byteSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mWeightBuffer));
	//RWStructuredBuffer用gError
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(node_byteSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mErrorBuffer));
	//up用gNode
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(node_byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mNodeUpBuffer));
	//up用gWeight
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(weight_byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mWeightUpBuffer));
	//up用gError
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(node_byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mErrorUpBuffer));
	//read用gNode
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(node_byteSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&mNodeReadBuffer));
	//read用gWeight
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(weight_byteSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&mWeightReadBuffer));
	//read用gError
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(node_byteSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&mErrorReadBuffer));

	D3D12_SUBRESOURCE_DATA subResourceDataNode = {};
	subResourceDataNode.pData = node;
	subResourceDataNode.RowPitch = nodeNumAll;
	subResourceDataNode.SlicePitch = subResourceDataNode.RowPitch;

	D3D12_SUBRESOURCE_DATA subResourceDataWeight = {};
	subResourceDataWeight.pData = weight;
	subResourceDataWeight.RowPitch = weightNumAll;
	subResourceDataWeight.SlicePitch = subResourceDataWeight.RowPitch;

	D3D12_SUBRESOURCE_DATA subResourceDataError = {};
	subResourceDataError.pData = error;
	subResourceDataError.RowPitch = nodeNumAll;
	subResourceDataError.SlicePitch = subResourceDataError.RowPitch;

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mNodeBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(mCommandList, mNodeBuffer.Get(), mNodeUpBuffer.Get(), 0, 0, 1, &subResourceDataNode);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mNodeBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mWeightBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(mCommandList, mWeightBuffer.Get(), mWeightUpBuffer.Get(), 0, 0, 1, &subResourceDataWeight);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mWeightBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mErrorBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(mCommandList, mErrorBuffer.Get(), mErrorUpBuffer.Get(), 0, 0, 1, &subResourceDataError);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mErrorBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	//ルートシグネチャ
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];
	slotRootParameter[0].InitAsUnorderedAccessView(0);//RWStructuredBuffer(u0)
	slotRootParameter[1].InitAsUnorderedAccessView(1);//RWStructuredBuffer(u1)
	slotRootParameter[2].InitAsUnorderedAccessView(2);//RWStructuredBuffer(u2)
	slotRootParameter[3].InitAsConstantBufferView(0);//mObjectCB(b0)
	mRootSignatureCom = CreateRsCompute(4, slotRootParameter);

	pCS[0] = CompileShader(ShaderNeuralNetwork, strlen(ShaderNeuralNetwork), "NNFPCS", "cs_5_0");
	pCS[1] = CompileShader(ShaderNeuralNetwork, strlen(ShaderNeuralNetwork), "InTargetCS", "cs_5_0");
	pCS[2] = CompileShader(ShaderNeuralNetwork, strlen(ShaderNeuralNetwork), "NNBPCS0", "cs_5_0");
	pCS[3] = CompileShader(ShaderNeuralNetwork, strlen(ShaderNeuralNetwork), "NNBPCS1", "cs_5_0");
	pCS[4] = CompileShader(ShaderNeuralNetwork, strlen(ShaderNeuralNetwork), "NNInverseCS", "cs_5_0");
	for (int i = 0; i < 5; i++)
		mPSOCom[i] = CreatePsoCompute(pCS[i].Get(), mRootSignatureCom.Get());
}

void DxNeuralNetwork::ForwardPropagation() {
	for (int i = 0; i < Depth - 1; i++) {
		dx->Bigin(com_no);
		mCommandList->SetPipelineState(mPSOCom[0].Get());
		mCommandList->SetComputeRootSignature(mRootSignatureCom.Get());
		mCommandList->SetComputeRootUnorderedAccessView(0, mNodeBuffer->GetGPUVirtualAddress());
		mCommandList->SetComputeRootUnorderedAccessView(1, mWeightBuffer->GetGPUVirtualAddress());
		mCommandList->SetComputeRootUnorderedAccessView(2, mErrorBuffer->GetGPUVirtualAddress());
		cb.Lear_Depth.x = learningRate;
		cb.Lear_Depth.y = i;
		for (UINT i1 = 0; i1 < NumNode[Depth - 1]; i1++)
			cb.Target[i1].x = target[i1];
		mObjectCB->CopyData(0, cb);
		mCommandList->SetComputeRootConstantBufferView(3, mObjectCB->Resource()->GetGPUVirtualAddress());
		//Dispatchは1回毎にGPU処理完了させる事
		mCommandList->Dispatch(NumNode[i + 1], 1, 1);
		dx->End(com_no);
		dx->WaitFenceCurrent();
	}
	dx->Bigin(com_no);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mNodeBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
	mCommandList->CopyResource(mNodeReadBuffer.Get(), mNodeBuffer.Get());
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mNodeBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	dx->End(com_no);
	dx->WaitFenceCurrent();
}

void DxNeuralNetwork::BackPropagation() {
	//target入力
	dx->Bigin(com_no);
	mCommandList->SetPipelineState(mPSOCom[1].Get());
	mCommandList->SetComputeRootSignature(mRootSignatureCom.Get());
	mCommandList->SetComputeRootUnorderedAccessView(0, mNodeBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(1, mWeightBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(2, mErrorBuffer->GetGPUVirtualAddress());
	cb.Lear_Depth.y = Depth - 1;
	mObjectCB->CopyData(0, cb);
	mCommandList->SetComputeRootConstantBufferView(3, mObjectCB->Resource()->GetGPUVirtualAddress());
	mCommandList->Dispatch(NumNode[Depth - 1], 1, 1);
	dx->End(com_no);
	dx->WaitFenceCurrent();

	//逆伝搬
	for (int i = Depth - 2; i >= 0; i--) {
		dx->Bigin(com_no);
		mCommandList->SetPipelineState(mPSOCom[2].Get());
		mCommandList->SetComputeRootSignature(mRootSignatureCom.Get());
		mCommandList->SetComputeRootUnorderedAccessView(0, mNodeBuffer->GetGPUVirtualAddress());
		mCommandList->SetComputeRootUnorderedAccessView(1, mWeightBuffer->GetGPUVirtualAddress());
		mCommandList->SetComputeRootUnorderedAccessView(2, mErrorBuffer->GetGPUVirtualAddress());
		cb.Lear_Depth.y = i;
		mObjectCB->CopyData(0, cb);
		mCommandList->SetComputeRootConstantBufferView(3, mObjectCB->Resource()->GetGPUVirtualAddress());
		mCommandList->Dispatch(NumNode[i], 1, 1);
		dx->End(com_no);
		dx->WaitFenceCurrent();
	}

	//weight値更新
	for (int i = Depth - 2; i >= 0; i--) {
		dx->Bigin(com_no);
		mCommandList->SetPipelineState(mPSOCom[3].Get());
		mCommandList->SetComputeRootSignature(mRootSignatureCom.Get());
		mCommandList->SetComputeRootUnorderedAccessView(0, mNodeBuffer->GetGPUVirtualAddress());
		mCommandList->SetComputeRootUnorderedAccessView(1, mWeightBuffer->GetGPUVirtualAddress());
		mCommandList->SetComputeRootUnorderedAccessView(2, mErrorBuffer->GetGPUVirtualAddress());
		cb.Lear_Depth.y = i;
		mObjectCB->CopyData(0, cb);
		mCommandList->SetComputeRootConstantBufferView(3, mObjectCB->Resource()->GetGPUVirtualAddress());
		mCommandList->Dispatch(NumNode[i], NumNode[i + 1], 1);
		dx->End(com_no);
		dx->WaitFenceCurrent();
	}

	dx->Bigin(com_no);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mErrorBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
	mCommandList->CopyResource(mErrorReadBuffer.Get(), mErrorBuffer.Get());
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mErrorBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mWeightBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
	mCommandList->CopyResource(mWeightReadBuffer.Get(), mWeightBuffer.Get());
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mWeightBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	dx->End(com_no);
	dx->WaitFenceCurrent();
}

void DxNeuralNetwork::CopyNodeResourse() {
	D3D12_RANGE range;
	range.Begin = 0;
	range.End = node_byteSize;
	D3D12_SUBRESOURCE_DATA subResource;
	mNodeReadBuffer->Map(0, &range, reinterpret_cast<void**>(&subResource));
	float *nod = (float*)subResource.pData;
	UINT subInd = 0;
	for (int k = 0; k < Depth; k++) {
		for (UINT i = 0; i < NumNode[k]; i++) {
			node[NumNodeStIndex[k] + i] = nod[subInd++];
		}
	}
	mNodeReadBuffer->Unmap(0, nullptr);
}

void DxNeuralNetwork::CopyOutput() {
	for (UINT i = 0; i < NumNode[Depth - 1]; i++) {
		output[i] = node[NumNodeStIndex[Depth - 1] + i];
	}
}

void DxNeuralNetwork::CopyInverse() {
	for (UINT i = 0; i < NumNode[0]; i++) {
		inverse[i] = node[i];
	}
}

void DxNeuralNetwork::CopyWeightResourse() {
	D3D12_RANGE range;
	range.Begin = 0;
	range.End = weight_byteSize;
	float *wei = nullptr;
	mWeightReadBuffer->Map(0, &range, reinterpret_cast<void**>(&wei));
	UINT subInd = 0;
	for (int k = 0; k < Depth - 1; k++) {
		for (UINT i = 0; i < NumWeight[k]; i++) {
			weight[NumWeightStIndex[k] + i] = wei[subInd++];
		}
	}
	mWeightReadBuffer->Unmap(0, nullptr);
}
//↓↑どちらの書き方でもOK
void DxNeuralNetwork::CopyErrorResourse() {
	D3D12_RANGE range;
	range.Begin = 0;
	range.End = node_byteSize;
	D3D12_SUBRESOURCE_DATA subResource;
	mErrorReadBuffer->Map(0, &range, reinterpret_cast<void**>(&subResource));
	float *err = (float*)subResource.pData;
	UINT subInd = 0;
	for (int k = 0; k < Depth; k++) {
		for (UINT i = 0; i < NumNode[k]; i++) {
			error[NumNodeStIndex[k] + i] = err[subInd++];
		}
	}
	mErrorReadBuffer->Unmap(0, nullptr);
}

void DxNeuralNetwork::SetLearningLate(float rate) {
	learningRate = rate;
}

void DxNeuralNetwork::SetTarget(float *tar) {
	memcpy(target, tar, sizeof(float) * NumNode[Depth - 1]);
}

void DxNeuralNetwork::SetTargetEl(float el, UINT ElNum) {
	target[ElNum] = el;
}

void DxNeuralNetwork::FirstInput(float el, UINT ElNum) {
	for (UINT i = 0; i < Split; i++)InputArrayEl(el, i, ElNum);
}

void DxNeuralNetwork::InputArray(float *inArr, UINT arrNum) {
	memcpy(&node[(NumNode[0] / Split) * arrNum], inArr, sizeof(float) * (NumNode[0] / Split));
}

void DxNeuralNetwork::InputArrayEl(float el, UINT arrNum, UINT ElNum) {
	node[(NumNode[0] / Split) * arrNum + ElNum] = el;
}

void DxNeuralNetwork::InputResourse() {
	D3D12_SUBRESOURCE_DATA subResourceDataNode = {};
	subResourceDataNode.pData = node;
	subResourceDataNode.RowPitch = nodeNumAll;
	subResourceDataNode.SlicePitch = subResourceDataNode.RowPitch;
	dx->Bigin(com_no);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mNodeBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(mCommandList, mNodeBuffer.Get(), mNodeUpBuffer.Get(), 0, 0, 1, &subResourceDataNode);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mNodeBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	dx->End(com_no);
	dx->WaitFenceCurrent();
}

void DxNeuralNetwork::GetOutput(float *out) {
	for (UINT i = 0; i < NumNode[Depth - 1]; i++) {
		out[i] = output[i];
	}
}

float DxNeuralNetwork::GetOutputEl(UINT ElNum) {
	return output[ElNum];
}

void DxNeuralNetwork::Query() {
	InputResourse();
	ForwardPropagation();
	CopyNodeResourse();
	CopyOutput();
	InverseQuery();
	TextureCopy(mNodeBuffer.Get(), com_no);
	CopyNodeResourse();
	CopyInverse();
}

void DxNeuralNetwork::Training() {
	InputResourse();
	ForwardPropagation();
	CopyNodeResourse();
	CopyOutput();

	BackPropagation();
	CopyWeightResourse();
	CopyErrorResourse();

	//↓BackPropagation()の直前に実行しない事
	InverseQuery();
	TextureCopy(mNodeBuffer.Get(), com_no);
	CopyNodeResourse();
	CopyInverse();
}

float *DxNeuralNetwork::GetError(UINT arrNum) {
	return &error[(NumNode[0] / Split) * arrNum];
}

float DxNeuralNetwork::GetErrorEl(UINT arrNum, UINT ElNum) {
	return error[(NumNode[0] / Split) * arrNum + ElNum];
}

void DxNeuralNetwork::InverseQuery() {
	//逆入力
	for (int i = Depth - 1; i >= 1; i--) {
		dx->Bigin(com_no);
		mCommandList->SetPipelineState(mPSOCom[4].Get());
		mCommandList->SetComputeRootSignature(mRootSignatureCom.Get());
		mCommandList->SetComputeRootUnorderedAccessView(0, mNodeBuffer->GetGPUVirtualAddress());
		mCommandList->SetComputeRootUnorderedAccessView(1, mWeightBuffer->GetGPUVirtualAddress());
		mCommandList->SetComputeRootUnorderedAccessView(2, mErrorBuffer->GetGPUVirtualAddress());
		cb.Lear_Depth.y = i;
		mObjectCB->CopyData(0, cb);
		mCommandList->SetComputeRootConstantBufferView(3, mObjectCB->Resource()->GetGPUVirtualAddress());
		mCommandList->Dispatch(NumNode[i - 1], 1, 1);
		dx->End(com_no);
		dx->WaitFenceCurrent();
	}

	dx->Bigin(com_no);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mNodeBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
	mCommandList->CopyResource(mNodeReadBuffer.Get(), mNodeBuffer.Get());
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mNodeBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	dx->End(com_no);
	dx->WaitFenceCurrent();
}

float *DxNeuralNetwork::GetInverseOutput(UINT arrNum) {
	return &inverse[(NumNode[0] / Split) * arrNum];
}

float DxNeuralNetwork::GetInverseOutputEl(UINT arrNum, UINT ElNum) {
	return inverse[(NumNode[0] / Split) * arrNum + ElNum];
}

void DxNeuralNetwork::SaveData() {
	FILE *fp = fopen("save/save.da", "wb");
	float *weightArr = new float[weightNumAll];

	UINT cnt = 0;
	for (int k = 0; k < Depth - 1; k++) {
		for (UINT i = 0; i < NumWeight[k]; i++) {
			weightArr[cnt++] = weight[NumWeightStIndex[k] + i];
		}
	}

	fwrite(weightArr, sizeof(float) * weightNumAll, 1, fp);
	fclose(fp);
	ARR_DELETE(weightArr);
}

void DxNeuralNetwork::LoadData() {
	FILE *fp = fopen("save/save.da", "rb");
	float *weightArr = new float[weightNumAll];
	fread(weightArr, sizeof(float) * weightNumAll, 1, fp);
	fclose(fp);

	UINT cnt = 0;
	for (int k = 0; k < Depth - 1; k++) {
		for (UINT i = 0; i < NumWeight[k]; i++) {
			weight[NumWeightStIndex[k] + i] = weightArr[cnt++];
		}
	}
	ARR_DELETE(weightArr);

	D3D12_SUBRESOURCE_DATA subResourceDataWeight = {};
	subResourceDataWeight.pData = weight;
	subResourceDataWeight.RowPitch = weightNumAll;
	subResourceDataWeight.SlicePitch = subResourceDataWeight.RowPitch;
	dx->Bigin(com_no);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mWeightBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(mCommandList, mWeightBuffer.Get(), mWeightUpBuffer.Get(), 0, 0, 1, &subResourceDataWeight);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mWeightBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	dx->End(com_no);
	dx->WaitFenceCurrent();
}
