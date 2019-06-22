//*****************************************************************************************//
//**                                                                                     **//
//**                                    CNN                                              **//
//**                                                                                     **//
//*****************************************************************************************//

#include "CNN.h"

GradCAM::GradCAM(UINT srcWid, UINT srcHei, UINT SizeFeatureMapW, UINT SizeFeatureMapH, UINT NumGradientEl, UINT NumFil, UINT inputsetnum) :
	DxGradCAM(SizeFeatureMapW, SizeFeatureMapH, NumGradientEl, NumFil, inputsetnum) {

	ComCreate(srcWid, srcHei, 1.0f);
	dgc.SetCommandList(0);
	dgc.GetVBarray2D(1);
	dgc.TextureInit(srcWid, srcHei);
	dgc.TexOn();
	dgc.CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
}

void GradCAM::Draw(float x, float y) {
	dgc.Update(x, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 300.0f, 200.0f);
	dgc.CopyResource(GetPixel(), GetNNTextureResourceStates());
	dgc.Draw();
}

Affine::Affine(UINT inW, UINT inH, UINT* numNode, int depth, UINT split, UINT inputsetnum) :
	DxNeuralNetwork(numNode, depth, split, inputsetnum) {

	NumFilter = split;
	ComCreateReLU();
	SetLeakyReLUAlpha(0.05f);
	SetLearningLate(0.12f);
	CreareNNTexture(inW, inH, NumFilter);
	dnn.SetCommandList(0);
	dnn.GetVBarray2D(1);
	dnn.TextureInit(inW, inH * NumFilter);
	dnn.TexOn();
	dnn.CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
}

void Affine::Draw(float x, float y) {
	dnn.Update(x, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 20.0f, 20.0f * NumFilter);
	dnn.CopyResource(GetNNTextureResource(), GetNNTextureResourceStates());
	dnn.Draw();
}

void Affine::ErrConnection() {
	if (errPo) {
		errPo->SetInErrorResource(GetOutErrorResource());
		errPo->Training();
		errPo->ErrConnection();
	}
	if (errCn) {
		errCn->SetInErrorResource(GetOutErrorResource());
		errCn->Training();
		errCn->ErrConnection();
	}
}

Pooling::Pooling(UINT width, UINT height, UINT poolNum, UINT inputsetnum) :
	DxPooling(width, height, poolNum, inputsetnum) {

	NumFilter = poolNum;
	ComCreate();
	UINT wid = GetOutWidth();
	UINT hei = GetOutHeight();
	CreareNNTexture(wid, hei, NumFilter);
	dpo.SetCommandList(0);
	dpo.GetVBarray2D(1);
	dpo.TextureInit(wid, hei * NumFilter);
	dpo.TexOn();
	dpo.CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
}

void Pooling::Draw(float x, float y) {
	dpo.Update(x, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 20.0f, 20.0f * NumFilter);
	dpo.CopyResource(GetNNTextureResource(), GetNNTextureResourceStates());
	dpo.Draw();
}

void Pooling::InConnection() {
	Query();
	if (inCn) {
		inCn->SetInputResource(GetOutputResource());
		inCn->InConnection();
	}
	if (inAf) {
		inAf->SetInputResource(GetOutputResource());
	}
}

void Pooling::ErrConnection() {
	if (errCn) {
		errCn->SetInErrorResource(GetOutErrorResource());
		errCn->Training();
		errCn->ErrConnection();
	}
}

void Pooling::TestConnection() {
	Test();
	if (inCn) {
		inCn->SetInputResource(GetOutputResource());
		inCn->TestConnection();
	}
	if (inAf) {
		inAf->SetInputResource(GetOutputResource());
	}
}

void Pooling::DetectionConnection(UINT SearchNum) {
	Detection(SearchNum);
	if (inCn) {
		inCn->SetInputResource(GetOutputResource());
		inCn->DetectionConnection(SearchNum);
	}
	if (inAf) {
		inAf->SetInputResource(GetOutputResource());
	}
}

Convolution::Convolution(UINT width, UINT height, UINT filNum, UINT inputsetnum, UINT elnumwid, UINT filstep) :
	DxConvolution(width, height, filNum, inputsetnum, elnumwid, filstep) {

	NumFilter = filNum;
	ComCreateReLU();
	SetLeakyReLUAlpha(0.05f);
	CreareNNTexture(elnumwid, elnumwid, NumFilter);
	dcn.SetCommandList(0);
	dcn.GetVBarray2D(1);
	dcn.TextureInit(elnumwid, elnumwid * NumFilter);
	dcn.TexOn();
	dcn.CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
}

void Convolution::Draw(float x, float y) {
	dcn.Update(x, y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 20.0f, 20.0f * NumFilter);
	dcn.CopyResource(GetNNTextureResource(), GetNNTextureResourceStates());
	dcn.Draw();
}

void Convolution::InConnection() {
	Query();
	if (inCn) {
		inCn->SetInputResource(GetOutputResource());
		inCn->InConnection();
	}
	if (inPo) {
		inPo->SetInputResource(GetOutputResource());
		inPo->InConnection();
	}
	if (inAf) {
		inAf->SetInputResource(GetOutputResource());
	}
}

void Convolution::ErrConnection() {
	if (errCn) {
		errCn->SetInErrorResource(GetOutErrorResource());
		errCn->Training();
		errCn->ErrConnection();
	}
	if (errPo) {
		errPo->SetInErrorResource(GetOutErrorResource());
		errPo->Training();
		errPo->ErrConnection();
	}
}

void Convolution::TestConnection() {
	Test();
	if (inCn) {
		inCn->SetInputResource(GetOutputResource());
		inCn->TestConnection();
	}
	if (inPo) {
		inPo->SetInputResource(GetOutputResource());
		inPo->TestConnection();
	}
	if (inAf) {
		inAf->SetInputResource(GetOutputResource());
	}
}

void Convolution::DetectionConnection(UINT SearchNum) {
	Detection(SearchNum);
	if (inCn) {
		inCn->SetInputResource(GetOutputResource());
		inCn->DetectionConnection(SearchNum);
	}
	if (inPo) {
		inPo->SetInputResource(GetOutputResource());
		inPo->DetectionConnection(SearchNum);
	}
	if (inAf) {
		inAf->SetInputResource(GetOutputResource());
	}
}

CNN::CNN(UINT srcW, UINT srcH, Layer* layer, UINT layersize) {
	NumConv = 0;
	NumPool = 0;
	layerSize = layersize;
	//レイヤーカウント
	for (UINT i = 0; i < layerSize; i++) {
		switch (layer[i].layerName) {
		case CONV:
			NumConv++;
			break;
		case POOL:
			NumPool++;
			break;
		}
	}

	cn = new Convolution * [NumConv];
	po = new Pooling * [NumPool];
	UINT NumDepth = 0;

	if (layer[NumConv + NumPool].NumDepthNotInput > MAX_DEPTH_NUM - 1)NumDepth = MAX_DEPTH_NUM;
	else
		NumDepth = layer[NumConv + NumPool].NumDepthNotInput + 1;

	//レイヤー生成
	UINT convCnt = 0;
	UINT poolCnt = 0;
	UINT wid = layer[0].mapWid;
	UINT hei = layer[0].mapHei;
	UINT numN[MAX_DEPTH_NUM];
	for (UINT i = 0; i < layerSize; i++) {
		switch (layer[i].layerName) {
		case CONV:
			cn[convCnt] = new Convolution(wid, hei, layer[i].NumFilter, layer[i].maxThread,
				layer[i].NumConvFilterWid, layer[i].NumConvFilterSlide);
			wid = cn[convCnt]->GetOutWidth();
			hei = cn[convCnt++]->GetOutHeight();
			if (convCnt == NumConv) {
				gc = new GradCAM(srcW, srcH, wid, hei, layer[i].NumConvFilterWid * layer[i].NumConvFilterWid,
					layer[i].NumFilter, layer[i].maxThread);
			}
			break;
		case POOL:
			po[poolCnt] = new Pooling(wid, hei, layer[i].NumFilter, layer[i].maxThread);
			wid = po[poolCnt]->GetOutWidth();
			hei = po[poolCnt++]->GetOutHeight();
			break;
		case AFFINE:
			numN[0] = wid * hei;
			for (UINT i1 = 1; i1 < NumDepth; i1++)numN[i1] = layer[i].numNode[i1 - 1];
			nn = new Affine(wid, hei, numN, NumDepth, layer[i].NumFilter, layer[i].maxThread);
			break;
		}
	}

	//レイヤー接続
	convCnt = 0;
	poolCnt = 0;
	for (UINT i = 0; i < layerSize; i++) {
		switch (layer[i].layerName) {
		case CONV:
			//誤差側接続
			if (i > 0) {
				if (layer[i - 1].layerName == CONV)cn[convCnt]->errCn = cn[convCnt - 1];
				if (layer[i - 1].layerName == POOL)cn[convCnt]->errPo = po[poolCnt - 1];
			}
			//入力側接続
			if (layer[i + 1].layerName == CONV)cn[convCnt]->inCn = cn[convCnt + 1];
			if (layer[i + 1].layerName == POOL)cn[convCnt]->inPo = po[poolCnt];
			if (layer[i + 1].layerName == AFFINE)cn[convCnt]->inAf = nn;
			convCnt++;
			break;
		case POOL:
			//誤差側接続
			if (layer[i - 1].layerName == CONV)po[poolCnt]->errCn = cn[convCnt - 1];
			//入力側接続
			if (layer[i + 1].layerName == CONV)po[poolCnt]->inCn = cn[convCnt];
			if (layer[i + 1].layerName == AFFINE)po[poolCnt]->inAf = nn;
			poolCnt++;
			break;
		case AFFINE:
			//誤差側接続
			if (layer[i - 1].layerName == CONV)nn->errCn = cn[convCnt - 1];
			if (layer[i - 1].layerName == POOL)nn->errPo = po[poolCnt - 1];
			break;
		}
	}
}

CNN::~CNN() {
	for (UINT i = 0; i < NumConv; i++)S_DELETE(cn[i]);
	ARR_DELETE(cn);
	for (UINT i = 0; i < NumPool; i++)S_DELETE(po[i]);
	ARR_DELETE(po);
	S_DELETE(nn);
	S_DELETE(gc);
}

void CNN::Detection(UINT SearchNum) {
	cn[0]->DetectionConnection(SearchNum);
	nn->Query(SearchNum);
}

void CNN::DetectionGradCAM(UINT SearchNum, UINT srcMapWid, UINT mapslide) {
	cn[0]->DetectionConnection(SearchNum);
	gc->SetFeatureMap(cn[NumConv - 1]->GetOutputResource());//最終Convの出力を記録()
	nn->SetTargetEl(0.99f, 0);
	nn->QueryAndBackPropagation(SearchNum);//フィルター更新無しの逆伝播
	if (nn->errPo) {
		nn->errPo->SetInErrorResource(nn->GetOutErrorResource());
		nn->errPo->Training();
		nn->errPo->errCn->SetInErrorResource(nn->errPo->GetOutErrorResource());
		nn->errPo->errCn->BackPropagationNoWeightUpdate();//フィルター更新無しの逆伝播
	}
	if (nn->errCn) {
		nn->errCn->SetInErrorResource(nn->GetOutErrorResource());
		nn->errCn->BackPropagationNoWeightUpdate();
	}

	gc->SetGradient(cn[NumConv - 1]->GetGradient());
	gc->ComGAP();
	gc->ComGradCAM(SearchNum);
	gc->GradCAMSynthesis(srcMapWid, srcMapWid, mapslide);
}

void CNN::SetLearningLate(float nN, float cN) {
	for (UINT i = 0; i < NumConv; i++)cn[i]->SetLearningLate(cN, 0.0f);
	nn->SetLearningLate(nN);
}

void CNN::Training() {
	for (UINT i = 0; i < NumConv; i++)cn[i]->SetdropThreshold(0.0f);
	cn[0]->InConnection();
	float drop[MAX_DEPTH_NUM];
	drop[0] = 0.0f;
	drop[1] = 0.1f;
	drop[2] = 0.0f;
	drop[3] = 0.0f;
	drop[4] = 0.0f;
	nn->SetdropThreshold(drop);
	nn->Training();
	nn->ErrConnection();
}

void CNN::Test() {
	for (UINT i = 0; i < NumConv; i++)cn[i]->SetdropThreshold(0.0f);
	cn[0]->InConnection();
	float drop[MAX_DEPTH_NUM];
	drop[0] = 0.0f;
	drop[1] = 0.0f;
	drop[2] = 0.0f;
	drop[3] = 0.0f;
	drop[4] = 0.0f;
	nn->SetdropThreshold(drop);
	nn->Test();
}

void CNN::TrainingDraw() {
	float x = 0.0f;
	for (UINT i = 0; i < NumConv; i++) {
		cn[i]->Draw(x, 0.0f);
		x += 20.0f;
	}
	for (UINT i = 0; i < NumConv; i++) {
		po[i]->Draw(x, 0.0f);
		x += 20.0f;
	}
	nn->Draw(x, 0.0f);
}

void CNN::GradCAMDraw() {
	gc->Draw(50.0f, 200.0f);
}

float CNN::GetOutputEl(UINT ElNum, UINT inputsetInd) {
	return nn->GetOutputEl(ElNum, inputsetInd);
}

void CNN::SetTargetEl(float el, UINT ElNum) {
	nn->SetTargetEl(el, ElNum);
}

void CNN::FirstInput(float el, UINT ElNum, UINT inputsetInd) {
	cn[0]->FirstInput(el, ElNum, inputsetInd);
}

void CNN::SetPixel3ch(ID3D12Resource* pi) {
	gc->SetPixel3ch(pi);
}

void CNN::SetPixel3ch(BYTE* pi) {
	gc->SetPixel3ch(pi);
}

void CNN::SaveData() {
	for (UINT i = 0; i < NumConv; i++)cn[i]->SaveData(i);
	nn->SaveData();
}

void CNN::LoadData() {
	for (UINT i = 0; i < NumConv; i++)cn[i]->LoadData(i);
	nn->LoadData();
}