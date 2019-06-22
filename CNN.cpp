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

CNN::CNN(UINT srcW, UINT srcH, UINT width, UINT height, UINT* numNode, int depth, UINT NumFilter, UINT SearchMaxNum) {
	NumConv = 3;
	NumPool = 3;
	if (depth > MAX_DEPTH_NUM)NumDepth = MAX_DEPTH_NUM;
	else
		NumDepth = depth;

	cn = new Convolution * [NumConv];
	po = new Pooling * [NumPool];
	cn[0] = new Convolution(width, height, NumFilter, SearchMaxNum, 7, 1);
	UINT wid = cn[0]->GetOutWidth();
	UINT hei = cn[0]->GetOutHeight();

	po[0] = new Pooling(wid, hei, NumFilter, SearchMaxNum);
	wid = po[0]->GetOutWidth();
	hei = po[0]->GetOutHeight();

	cn[1] = new Convolution(wid, hei, NumFilter, SearchMaxNum, 5, 1);
	wid = cn[1]->GetOutWidth();
	hei = cn[1]->GetOutHeight();

	po[1] = new Pooling(wid, hei, NumFilter, SearchMaxNum);
	wid = po[1]->GetOutWidth();
	hei = po[1]->GetOutHeight();

	cn[2] = new Convolution(wid, hei, NumFilter, SearchMaxNum, 3, 1);
	wid = cn[2]->GetOutWidth();
	hei = cn[2]->GetOutHeight();

	gc = new GradCAM(srcW, srcH, wid, hei, 3 * 3, NumFilter, SearchMaxNum);

	po[2] = new Pooling(wid, hei, NumFilter, SearchMaxNum);
	wid = po[2]->GetOutWidth();
	hei = po[2]->GetOutHeight();

	UINT numN[MAX_DEPTH_NUM];
	numN[0] = wid * hei;
	for (UINT i = 1; i < NumDepth; i++)numN[i] = numNode[i - 1];

	nn = new Affine(wid, hei, numN, NumDepth, NumFilter, SearchMaxNum);
}

CNN::~CNN() {
	for (UINT i = 0; i < NumConv; i++)S_DELETE(cn[i]);
	ARR_DELETE(cn);
	for (UINT i = 0; i < NumPool; i++)S_DELETE(po[i]);
	ARR_DELETE(po);
	S_DELETE(nn);
	S_DELETE(gc);
}

void CNN::RunConvolutionToPooling(UINT ind) {
	cn[ind]->Query();
	po[ind]->SetInputResource(cn[ind]->GetOutputResource());
}

void CNN::RunPoolingToConvolution(UINT ind) {
	po[ind]->Query();
	cn[ind + 1]->SetInputResource(po[ind]->GetOutputResource());
}

void CNN::RunPoolingToNN(UINT ind) {
	po[ind]->Query();
	nn->SetInputResource(po[ind]->GetOutputResource());
}

void CNN::RunConvolutionToPoolingDetec(UINT ind, UINT SearchNum) {
	cn[ind]->Detection(SearchNum);
	po[ind]->SetInputResource(cn[ind]->GetOutputResource());
}

void CNN::RunPoolingToConvolutionDetec(UINT ind, UINT SearchNum) {
	po[ind]->Detection(SearchNum);
	cn[ind + 1]->SetInputResource(po[ind]->GetOutputResource());
}

void CNN::RunPoolingToNNDetec(UINT ind, UINT SearchNum) {
	po[ind]->Detection(SearchNum);
	nn->SetInputResource(po[ind]->GetOutputResource());
}

void CNN::RunConvolutionToPoolingTest(UINT ind) {
	cn[ind]->Test();
	po[ind]->SetInputResource(cn[ind]->GetOutputResource());
}

void CNN::RunPoolingToConvolutionTest(UINT ind) {
	po[ind]->Test();
	cn[ind + 1]->SetInputResource(po[ind]->GetOutputResource());
}

void CNN::RunPoolingToNNTest(UINT ind) {
	po[ind]->Test();
	nn->SetInputResource(po[ind]->GetOutputResource());
}

void CNN::NNToPoolingBackPropagation(UINT ind) {
	po[ind]->SetInErrorResource(nn->GetOutErrorResource());
	po[ind]->Training();
}

void CNN::ConvolutionToPoolingBackPropagation(UINT ind) {
	po[ind]->SetInErrorResource(cn[ind + 1]->GetOutErrorResource());
	po[ind]->Training();
}

void CNN::PoolingToConvolutionBackPropagation(UINT ind) {
	cn[ind]->SetInErrorResource(po[ind]->GetOutErrorResource());
	cn[ind]->Training();
}

void CNN::query() {
	RunConvolutionToPooling(0);
	RunPoolingToConvolution(0);
	RunConvolutionToPooling(1);
	RunPoolingToConvolution(1);
	RunConvolutionToPooling(2);
	RunPoolingToNN(2);
}

void CNN::queryDetec(UINT SearchNum) {
	RunConvolutionToPoolingDetec(0, SearchNum);
	RunPoolingToConvolutionDetec(0, SearchNum);
	RunConvolutionToPoolingDetec(1, SearchNum);
	RunPoolingToConvolutionDetec(1, SearchNum);
	RunConvolutionToPoolingDetec(2, SearchNum);
	RunPoolingToNNDetec(2, SearchNum);
}

void CNN::queryTest() {
	RunConvolutionToPoolingTest(0);
	RunPoolingToConvolutionTest(0);
	RunConvolutionToPoolingTest(1);
	RunPoolingToConvolutionTest(1);
	RunConvolutionToPoolingTest(2);
	RunPoolingToNNTest(2);
}

void CNN::Detection(UINT SearchNum) {
	queryDetec(SearchNum);
	nn->Query(SearchNum);
}

void CNN::DetectionGradCAM(UINT SearchNum, UINT srcMapWid, UINT mapslide) {
	queryDetec(SearchNum);
	gc->SetFeatureMap(cn[2]->GetOutputResource());//ÅIConv‚Ìo—Í‚ð‹L˜^()
	nn->SetTargetEl(0.99f, 0);
	nn->QueryAndBackPropagation(SearchNum);//ƒtƒBƒ‹ƒ^[XV–³‚µ‚Ì‹t“`”d
	NNToPoolingBackPropagation(2);
	cn[2]->SetInErrorResource(po[2]->GetOutErrorResource());
	cn[2]->BackPropagationNoWeightUpdate();//ƒtƒBƒ‹ƒ^[XV–³‚µ‚Ì‹t“`”d
	gc->SetGradient(cn[2]->GetGradient());//
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
	query();
	float drop[MAX_DEPTH_NUM];
	drop[0] = 0.0f;
	drop[1] = 0.1f;
	drop[2] = 0.0f;
	drop[3] = 0.0f;
	drop[4] = 0.0f;
	nn->SetdropThreshold(drop);
	nn->Training();

	NNToPoolingBackPropagation(2);
	PoolingToConvolutionBackPropagation(2);
	ConvolutionToPoolingBackPropagation(1);
	PoolingToConvolutionBackPropagation(1);
	ConvolutionToPoolingBackPropagation(0);
	PoolingToConvolutionBackPropagation(0);
}

void CNN::Test() {
	for (UINT i = 0; i < NumConv; i++)cn[i]->SetdropThreshold(0.0f);
	queryTest();
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