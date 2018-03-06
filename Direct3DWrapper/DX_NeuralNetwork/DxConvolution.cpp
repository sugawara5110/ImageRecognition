//*****************************************************************************************//
//**                                                                                     **//
//**                              DxConvolution                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#include "DxConvolution.h"
#include <random>
#include "ShaderNN\ShaderConvolution.h"

void DxConvolution::SetLearningLate(float rate) {
	learningRate = rate;
}

DxConvolution::DxConvolution(UINT width, UINT height, UINT filNum, UINT elnumwid, UINT filstep) {

	elNumWid = elnumwid;
	ElNum = elNumWid * elNumWid;
	filterStep = filstep;

	if (elNumWid % 2 == 0)MessageBoxA(0, "filter�v�f���͊�̂ݗL���ł�", 0, MB_OK);
	if (elNumWid > 7)MessageBoxA(0, "filter�v�f����Max7�ł�", 0, MB_OK);
	if (filterStep > 8)MessageBoxA(0, "filter�X���C�h����Max8�ł�", 0, MB_OK);
	if (filterStep != 8 && filterStep != 4 &&
		filterStep != 2 && filterStep != 1)MessageBoxA(0, "filter�X���C�h����2�̗ݏ�̂ݗL���ł�", 0, MB_OK);

	Width = width;
	Height = height;
	OutWid = Width / filterStep;
	OutHei = Height / filterStep;
	FilNum = filNum;
	filSize = ElNum * sizeof(float);
	input_outerrOneNum = Width * Height;
	input_outerrOneSize = input_outerrOneNum * sizeof(float);
	output_inerrOneNum = OutWid * OutHei;
	output_inerrOneSize = output_inerrOneNum * sizeof(float);
	fil = new float[FilNum * ElNum];
	input = new float[FilNum * input_outerrOneNum];
	output = new float[FilNum * output_inerrOneNum];
	outputError = new float[FilNum * input_outerrOneNum];
	inputError = new float[FilNum * output_inerrOneNum];
	for (UINT i = 0; i < FilNum * input_outerrOneNum; i++)
		input[i] = 0.0f;
	for (UINT i = 0; i < FilNum * output_inerrOneNum; i++)
		inputError[i] = 0.0f;

	std::random_device seed_gen;
	std::default_random_engine engine(seed_gen());
	//�t�B���^�����l
	std::normal_distribution<> dist(0.0, pow(ElNum, 0.2));

	for (UINT i = 0; i < FilNum * ElNum; i++)
		fil[i] = dist(engine);

	dx = Dx12Process::GetInstance();
	mCommandList = dx->dx_sub[0].mCommandList.Get();
	mObjectCB = new UploadBuffer<CONSTANT_BUFFER_Convolution>(dx->md3dDevice.Get(), 1, true);
	cb.WidHei.x = Width;
	cb.WidHei.y = Height;
	cb.filWid_filStep.x = elNumWid;
	cb.filWid_filStep.y = filterStep;
	mObjectCB->CopyData(0, cb);
}

DxConvolution::~DxConvolution() {

	ARR_DELETE(input);
	ARR_DELETE(output);
	ARR_DELETE(fil);
	ARR_DELETE(outputError);
	ARR_DELETE(inputError);
	S_DELETE(mObjectCB);
}

void DxConvolution::SetCommandList(int no) {
	com_no = no;
	mCommandList = dx->dx_sub[com_no].mCommandList.Get();
}

void DxConvolution::ComCreate() {

	//RWStructuredBuffer�pgInput
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(input_outerrOneSize * FilNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mInputBuffer));
	//RWStructuredBuffer�pgOutput
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(output_inerrOneSize * FilNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mOutputBuffer));
	//RWStructuredBuffer�pgInErr
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(output_inerrOneSize * FilNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mInErrorBuffer));
	//RWStructuredBuffer�pgOutErr
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(input_outerrOneSize * FilNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mOutErrorBuffer));
	//RWStructuredBuffer�pgFilter
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(filSize * FilNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mFilterBuffer));
	//up�pgInput
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(input_outerrOneSize * FilNum),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mInputUpBuffer));
	//up�pgInErr
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(output_inerrOneSize * FilNum),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mInErrorUpBuffer));
	//up�pgFilter
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(filSize * FilNum),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mFilterUpBuffer));
	//read�pgOutput
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(output_inerrOneSize * FilNum),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&mOutputReadBuffer));
	//read�pgOutErr
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(input_outerrOneSize * FilNum),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&mOutErrorReadBuffer));
	//read�pgFilter
	dx->md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(filSize * FilNum),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&mFilterReadBuffer));

	D3D12_SUBRESOURCE_DATA subResourceDataInput = {};
	subResourceDataInput.pData = input;
	subResourceDataInput.RowPitch = input_outerrOneNum * FilNum;
	subResourceDataInput.SlicePitch = subResourceDataInput.RowPitch;

	D3D12_SUBRESOURCE_DATA subResourceDataInerror = {};
	subResourceDataInerror.pData = inputError;
	subResourceDataInerror.RowPitch = output_inerrOneNum * FilNum;
	subResourceDataInerror.SlicePitch = subResourceDataInerror.RowPitch;

	D3D12_SUBRESOURCE_DATA subResourceDataFilter = {};
	subResourceDataFilter.pData = fil;
	subResourceDataFilter.RowPitch = ElNum * FilNum;
	subResourceDataFilter.SlicePitch = subResourceDataFilter.RowPitch;

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mInputBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(mCommandList, mInputBuffer.Get(), mInputUpBuffer.Get(), 0, 0, 1, &subResourceDataInput);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mInputBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mInErrorBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(mCommandList, mInErrorBuffer.Get(), mInErrorUpBuffer.Get(), 0, 0, 1, &subResourceDataInerror);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mInErrorBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mFilterBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(mCommandList, mFilterBuffer.Get(), mFilterUpBuffer.Get(), 0, 0, 1, &subResourceDataFilter);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mFilterBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	//���[�g�V�O�l�`��
	CD3DX12_ROOT_PARAMETER slotRootParameter[6];
	slotRootParameter[0].InitAsUnorderedAccessView(0);//RWStructuredBuffer(u0)
	slotRootParameter[1].InitAsUnorderedAccessView(1);//RWStructuredBuffer(u1)
	slotRootParameter[2].InitAsUnorderedAccessView(2);//RWStructuredBuffer(u2)
	slotRootParameter[3].InitAsUnorderedAccessView(3);//RWStructuredBuffer(u3)
	slotRootParameter[4].InitAsUnorderedAccessView(4);//RWStructuredBuffer(u4)
	slotRootParameter[5].InitAsConstantBufferView(0);//mObjectCB(b0)
	mRootSignatureCom = CreateRsCompute(6, slotRootParameter);

	pCS[0] = CompileShader(ShaderConvolution, strlen(ShaderConvolution), "CNFPCS", "cs_5_0");
	pCS[1] = CompileShader(ShaderConvolution, strlen(ShaderConvolution), "CNBPCS0", "cs_5_0");
	pCS[2] = CompileShader(ShaderConvolution, strlen(ShaderConvolution), "CNBPCS1", "cs_5_0");
	pCS[3] = CompileShader(ShaderConvolution, strlen(ShaderConvolution), "CNBPCS2", "cs_5_0");
	for (int i = 0; i < 4; i++)
		mPSOCom[i] = CreatePsoCompute(pCS[i].Get(), mRootSignatureCom.Get());
}

void DxConvolution::FirstInput(float el, UINT ElNum) {
	for (UINT i = 0; i < FilNum; i++)InputEl(el - 0.5f, i, ElNum);
	firstIn = true;
}

void DxConvolution::InputEl(float el, UINT arrNum, UINT ElNum) {
	input[arrNum * input_outerrOneNum + ElNum] = el;
}

void DxConvolution::Input(float *inArr, UINT arrNum) {
	memcpy(&input[arrNum * input_outerrOneNum], inArr, input_outerrOneSize);
}

void DxConvolution::InputError(float *inArr, UINT arrNum) {
	memcpy(&inputError[arrNum * output_inerrOneNum], inArr, output_inerrOneSize);
}

void DxConvolution::ForwardPropagation() {
	cb.Lear.x = learningRate;
	mObjectCB->CopyData(0, cb);
	dx->Bigin(com_no);
	mCommandList->SetPipelineState(mPSOCom[0].Get());
	mCommandList->SetComputeRootSignature(mRootSignatureCom.Get());
	mCommandList->SetComputeRootUnorderedAccessView(0, mInputBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(1, mOutputBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(2, mInErrorBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(3, mOutErrorBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(4, mFilterBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootConstantBufferView(5, mObjectCB->Resource()->GetGPUVirtualAddress());
	mCommandList->Dispatch(OutWid, OutHei * FilNum, 1);
	dx->End(com_no);
	dx->WaitFenceCurrent();

	dx->Bigin(com_no);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mOutputBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
	mCommandList->CopyResource(mOutputReadBuffer.Get(), mOutputBuffer.Get());
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mOutputBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	dx->End(com_no);
	dx->WaitFenceCurrent();
}

void DxConvolution::BackPropagation() {
	dx->Bigin(com_no);
	mCommandList->SetPipelineState(mPSOCom[1].Get());
	mCommandList->SetComputeRootSignature(mRootSignatureCom.Get());
	mCommandList->SetComputeRootUnorderedAccessView(0, mInputBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(1, mOutputBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(2, mInErrorBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(3, mOutErrorBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(4, mFilterBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootConstantBufferView(5, mObjectCB->Resource()->GetGPUVirtualAddress());
	mCommandList->Dispatch(Width, Height * FilNum, 1);
	dx->End(com_no);
	dx->WaitFenceCurrent();

	dx->Bigin(com_no);
	mCommandList->SetPipelineState(mPSOCom[2].Get());
	mCommandList->SetComputeRootSignature(mRootSignatureCom.Get());
	mCommandList->SetComputeRootUnorderedAccessView(0, mInputBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(1, mOutputBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(2, mInErrorBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(3, mOutErrorBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(4, mFilterBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootConstantBufferView(5, mObjectCB->Resource()->GetGPUVirtualAddress());
	mCommandList->Dispatch(OutWid, OutHei * FilNum, 1);
	dx->End(com_no);
	dx->WaitFenceCurrent();

	dx->Bigin(com_no);
	mCommandList->SetPipelineState(mPSOCom[3].Get());
	mCommandList->SetComputeRootSignature(mRootSignatureCom.Get());
	mCommandList->SetComputeRootUnorderedAccessView(0, mInputBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(1, mOutputBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(2, mInErrorBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(3, mOutErrorBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootUnorderedAccessView(4, mFilterBuffer->GetGPUVirtualAddress());
	mCommandList->SetComputeRootConstantBufferView(5, mObjectCB->Resource()->GetGPUVirtualAddress());
	mCommandList->Dispatch(elNumWid, elNumWid * FilNum, 1);
	dx->End(com_no);
	dx->WaitFenceCurrent();

	dx->Bigin(com_no);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mOutErrorBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
	mCommandList->CopyResource(mOutErrorReadBuffer.Get(), mOutErrorBuffer.Get());
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mOutErrorBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mFilterBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
	mCommandList->CopyResource(mFilterReadBuffer.Get(), mFilterBuffer.Get());
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mFilterBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	dx->End(com_no);
	dx->WaitFenceCurrent();
}

void DxConvolution::Query() {
	//TestInput();
	InputResourse();
	ForwardPropagation();
	//CopyOutputResourse();
	TextureCopy(mOutputBuffer.Get(), com_no);
	//TestOutput();
}

void DxConvolution::Training() {
	//TestInErr();
	//TestFilter();
	//InputErrResourse();//���ڃ��\�[�X���R�s�[�̏ꍇ�g�p���Ȃ�(�J���̔z�񂪃R�s�[����Ă��܂�)
	BackPropagation();
	//CopyOutputErrResourse();
	CopyFilterResourse();
	//TestFilter();
	//TestOutErr();
}

void DxConvolution::TestFilter() {
	for (UINT i = 0; i < ElNum * FilNum; i++) {
		if (fil[i] > 3000 || fil[i] < -3000) {
			char st0[50];
			sprintf(st0, "fil[%d]�ُ�l�ł�", i);
			MessageBoxA(0, st0, 0, MB_OK);
			char st1[50];
			sprintf(st1, "%f", fil[i]);
			MessageBoxA(0, st1, 0, MB_OK);
			return;
		}
	}
}

void DxConvolution::TestInput() {
	for (UINT i = 0; i < input_outerrOneNum * FilNum; i++) {
		if (input[i] > 3000 || input[i] < -3000) {
			char st0[50];
			sprintf(st0, "input[%d]�ُ�l�ł�", i);
			MessageBoxA(0, st0, 0, MB_OK);
			char st1[50];
			sprintf(st1, "%f", input[i]);
			MessageBoxA(0, st1, 0, MB_OK);
			return;
		}
	}
}

void DxConvolution::TestInErr() {
	for (UINT i = 0; i < output_inerrOneNum * FilNum; i++) {
		if (inputError[i] > 3000 || inputError[i] < -3000) {
			char st0[50];
			sprintf(st0, "inputError[%d]�ُ�l�ł�", i);
			MessageBoxA(0, st0, 0, MB_OK);
			char st1[50];
			sprintf(st1, "%f", inputError[i]);
			MessageBoxA(0, st1, 0, MB_OK);
			return;
		}
	}
}

void DxConvolution::TestOutput() {
	for (UINT i = 0; i < output_inerrOneNum * FilNum; i++) {
		if (output[i] > 3000 || output[i] < -3000) {
			char st0[50];
			sprintf(st0, "output[%d]�ُ�l�ł�", i);
			MessageBoxA(0, st0, 0, MB_OK);
			char st1[50];
			sprintf(st1, "%f", output[i]);
			MessageBoxA(0, st1, 0, MB_OK);
			return;
		}
	}
}

void DxConvolution::TestOutErr() {
	for (UINT i = 0; i < input_outerrOneNum * FilNum; i++) {
		if (outputError[i] > 2000000 || outputError[i] < -2000000) {
			char st0[50];
			sprintf(st0, "outputError[%d]�ُ�l�ł�", i);
			MessageBoxA(0, st0, 0, MB_OK);
			char st1[50];
			sprintf(st1, "%f", outputError[i]);
			MessageBoxA(0, st1, 0, MB_OK);
			return;
		}
	}
}

void DxConvolution::InputResourse() {
	if (!firstIn)return;
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = input;
	subResourceData.RowPitch = input_outerrOneNum * FilNum;
	subResourceData.SlicePitch = subResourceData.RowPitch;
	dx->Bigin(com_no);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mInputBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(mCommandList, mInputBuffer.Get(), mInputUpBuffer.Get(), 0, 0, 1, &subResourceData);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mInputBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	dx->End(com_no);
	dx->WaitFenceCurrent();
	firstIn = false;
}

void DxConvolution::InputErrResourse() {
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = inputError;
	subResourceData.RowPitch = output_inerrOneNum * FilNum;
	subResourceData.SlicePitch = subResourceData.RowPitch;
	dx->Bigin(com_no);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mInErrorBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(mCommandList, mInErrorBuffer.Get(), mInErrorUpBuffer.Get(), 0, 0, 1, &subResourceData);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mInErrorBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	dx->End(com_no);
	dx->WaitFenceCurrent();
}

void DxConvolution::CopyOutputResourse() {
	D3D12_RANGE range;
	range.Begin = 0;
	range.End = output_inerrOneSize * FilNum;
	float *out = nullptr;
	mOutputReadBuffer->Map(0, &range, reinterpret_cast<void**>(&out));
	memcpy(output, out, output_inerrOneSize * FilNum);
	mOutputReadBuffer->Unmap(0, nullptr);
}

void DxConvolution::CopyOutputErrResourse() {
	D3D12_RANGE range;
	range.Begin = 0;
	range.End = input_outerrOneSize * FilNum;
	float *out = nullptr;
	mOutErrorReadBuffer->Map(0, &range, reinterpret_cast<void**>(&out));
	memcpy(outputError, out, input_outerrOneSize * FilNum);
	mOutErrorReadBuffer->Unmap(0, nullptr);
}

void DxConvolution::CopyFilterResourse() {
	D3D12_RANGE range;
	range.Begin = 0;
	range.End = filSize * FilNum;
	float *fi = nullptr;
	mFilterReadBuffer->Map(0, &range, reinterpret_cast<void**>(&fi));
	memcpy(fil, fi, filSize * FilNum);
	mFilterReadBuffer->Unmap(0, nullptr);
}

float *DxConvolution::Output(UINT arrNum) {
	return &output[arrNum * output_inerrOneNum];
}

float DxConvolution::OutputEl(UINT arrNum, UINT ElNum) {
	return output[arrNum * output_inerrOneNum + ElNum];
}

float DxConvolution::OutputFilter(UINT arrNum, UINT elNum) {
	return fil[arrNum * ElNum + elNum];
}

float *DxConvolution::GetError(UINT arrNum) {
	return &outputError[arrNum * input_outerrOneNum];
}

float DxConvolution::GetErrorEl(UINT arrNum, UINT ElNum) {
	return outputError[arrNum * input_outerrOneNum + ElNum];
}

void DxConvolution::SaveData(UINT Num) {
	FILE *fp;
	if (Num == 0)
		fp = fopen("save/save1.da", "wb");
	else
		fp = fopen("save/save2.da", "wb");
	float *tmp = new float[ElNum * FilNum];
	for (UINT k = 0; k < FilNum; k++) {
		for (UINT i = 0; i < ElNum; i++) {
			tmp[ElNum * k + i] = fil[k * ElNum + i];
		}
	}
	fwrite(tmp, sizeof(float) * ElNum * FilNum, 1, fp);
	fclose(fp);
	ARR_DELETE(tmp);
}

void DxConvolution::LoadData(UINT Num) {
	FILE *fp;
	if (Num == 0)
		fp = fopen("save/save1.da", "rb");
	else
		fp = fopen("save/save2.da", "rb");
	float *tmp = new float[ElNum * FilNum];
	fread(tmp, sizeof(float) * ElNum * FilNum, 1, fp);
	for (UINT k = 0; k < FilNum; k++) {
		for (UINT i = 0; i < ElNum; i++) {
			fil[k * ElNum + i] = tmp[ElNum * k + i];
		}
	}
	fclose(fp);
	ARR_DELETE(tmp);

	D3D12_SUBRESOURCE_DATA subResourceDataFilter = {};
	subResourceDataFilter.pData = fil;
	subResourceDataFilter.RowPitch = ElNum * FilNum;
	subResourceDataFilter.SlicePitch = subResourceDataFilter.RowPitch;
	dx->Bigin(com_no);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mFilterBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(mCommandList, mFilterBuffer.Get(), mFilterUpBuffer.Get(), 0, 0, 1, &subResourceDataFilter);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mFilterBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	dx->End(com_no);
	dx->WaitFenceCurrent();
}

void DxConvolution::SetInputResource(ID3D12Resource *res) {
	dx->Bigin(com_no);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mInputBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(res,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

	mCommandList->CopyResource(mInputBuffer.Get(), res);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(res,
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mInputBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	dx->End(com_no);
	dx->WaitFenceCurrent();
}

void DxConvolution::SetInErrorResource(ID3D12Resource *res) {
	dx->Bigin(com_no);
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mInErrorBuffer.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(res,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));

	mCommandList->CopyResource(mInErrorBuffer.Get(), res);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(res,
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mInErrorBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	dx->End(com_no);
	dx->WaitFenceCurrent();
}

ID3D12Resource *DxConvolution::GetOutErrorResource() {
	return mOutErrorBuffer.Get();
}

ID3D12Resource *DxConvolution::GetOutputResource() {
	return mOutputBuffer.Get();
}

UINT DxConvolution::GetOutWidth() {
	return OutWid;
}

UINT DxConvolution::GetOutHeight() {
	return OutHei;
}