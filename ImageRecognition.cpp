//*****************************************************************************************//
//**                                                                                     **//
//**                              ImageRecognition                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "ImageRecognition.h"
#include "TextureLoader.h"
#define LEARTEXWID 64

SP::SP(UINT srcwid, UINT srchei, UINT seawid, UINT seahei, float outscale, UINT step, UINT outNum, float Threshold, bool searchOn) {
	Sp = new SearchPixel(srcwid, srchei, seawid, seahei, outscale, step, outNum, Threshold);
	Sp->ComCreate();

	UINT spow = Sp->GetOutWid();
	UINT spoh = Sp->GetOutHei();
	if (searchOn)SearchMaxNum = Sp->GetSearchNum();
	else SearchMaxNum = 1;
	SearchNum = SearchMaxNum;
	Searchflg = new bool[SearchMaxNum];
	SearchOutInd = new int[SearchMaxNum];
	for (int i = 0; i < SearchMaxNum; i++) {
		Searchflg[i] = true;
		SearchOutInd[i] = i;
	}

	out = new float[SearchMaxNum];
	spPix = new float[srcwid * srchei];
	Sp->CreareNNTexture(spow, spoh, 1);
	dsp[0].SetCommandList(0);
	dsp[0].GetVBarray2D(1);
	dsp[0].TextureInit(spow, spoh);
	dsp[0].TexOn();
	dsp[0].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	dsp[1].SetCommandList(0);
	dsp[1].GetVBarray2D(1);
	dsp[1].TextureInit(srcwid, srchei);
	dsp[1].TexOn();
	dsp[1].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
}

SP::~SP() {
	ARR_DELETE(spPix);
	S_DELETE(Sp);
	ARR_DELETE(out);
	ARR_DELETE(Searchflg);
	ARR_DELETE(SearchOutInd);
}

ImageRecognition::ImageRecognition(UINT srcWid, UINT srcHei, UINT width, UINT height, UINT *numNode, int depth, UINT filnum, UCHAR type, bool searchOn, float threshold) {

	Threshold = threshold;
	Width = width;
	Height = height;
	filNum = filnum;
	Type = type;
	InW = srcWid;
	InH = srcHei;

	sp[0] = new SP(InW, InH, Width, Height, 1.0f, 16, numNode[depth - 1], Threshold, searchOn);
	sp[1] = new SP(InW, InH, Width * 1.5, Height * 1.5, 0.67f, 24, numNode[depth - 1], Threshold, searchOn);
	sp[2] = new SP(InW, InH, Width * 2, Height * 2, 0.5f, 32, numNode[depth - 1], Threshold, searchOn);

	SearchMaxNum = sp[0]->SearchMaxNum;

	unsigned int wid = Width;
	unsigned int hei = Height;
	if (Type == 'C' || Type == 'D') {
		cn[0] = new DxConvolution(wid, hei, filNum, SearchMaxNum, 5, 2);
		cn[0]->ComCreate();
		wid = cn[0]->GetOutWidth();
		hei = cn[0]->GetOutHeight();
		cn[0]->CreareNNTexture(5, 5, filNum);

		dcn[0].SetCommandList(0);
		dcn[0].GetVBarray2D(1);
		dcn[0].TextureInit(5, 5 * filNum);
		dcn[0].TexOn();
		dcn[0].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	}
	if (Type == 'C' || Type == 'P' || Type == 'D') {
		wid = Width;
		hei = Height;
		if (Type == 'C' || Type == 'D') {
			wid = cn[0]->GetOutWidth();
			hei = cn[0]->GetOutHeight();
		}
		po[0] = new DxPooling(wid, hei, filNum, SearchMaxNum);
		po[0]->ComCreate();
		wid = po[0]->GetOutWidth();
		hei = po[0]->GetOutHeight();
		po[0]->CreareNNTexture(wid, hei, filNum);

		dpo[0].SetCommandList(0);
		dpo[0].GetVBarray2D(1);
		dpo[0].TextureInit(wid, hei * filNum);
		dpo[0].TexOn();
		dpo[0].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	}
	if (Type == 'D') {
		cn[1] = new DxConvolution(wid, hei, filNum, SearchMaxNum, 3, 1);
		cn[1]->ComCreate();
		wid = cn[1]->GetOutWidth();
		hei = cn[1]->GetOutHeight();
		cn[1]->CreareNNTexture(3, 3, filNum);

		dcn[1].SetCommandList(0);
		dcn[1].GetVBarray2D(1);
		dcn[1].TextureInit(3, 3 * filNum);
		dcn[1].TexOn();
		dcn[1].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);

		po[1] = new DxPooling(wid, hei, filNum, SearchMaxNum);
		po[1]->ComCreate();
		wid = po[1]->GetOutWidth();
		hei = po[1]->GetOutHeight();
		po[1]->CreareNNTexture(wid, hei, filNum);

		dpo[1].SetCommandList(0);
		dpo[1].GetVBarray2D(1);
		dpo[1].TextureInit(wid, hei * filNum);
		dpo[1].TexOn();
		dpo[1].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	}

	numN = new UINT[depth + 1];
	numN[0] = wid * hei;
	for (int i = 1; i < depth + 1; i++)numN[i] = numNode[i - 1];
	nn = new DxNeuralNetwork(numN, depth + 1, filNum, SearchMaxNum);
	nn->ComCreate();
	nn->CreareNNTexture(wid, hei, filNum);

	dnn.SetCommandList(0);
	dnn.GetVBarray2D(1);
	dnn.TextureInit(wid, hei * filNum);
	dnn.TexOn();
	dnn.CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	nnWidth = wid;
	nnHeight = hei;

	Depth = depth + 1;

	din = new PolygonData2D[SearchMaxNum];
	for (UINT i = 0; i < SearchMaxNum; i++) {
		din[i].SetCommandList(0);
		din[i].GetVBarray2D(1);
		din[i].TextureInit(Width, Height);
		din[i].TexOn();
		din[i].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	}
	pixIn = new UINT**[SearchMaxNum];
	for (UINT i = 0; i < SearchMaxNum; i++) {
		pixIn[i] = new UINT*[Height];
	}
	for (UINT i = 0; i < SearchMaxNum; i++) {
		for (UINT k = 0; k < Height; k++) {
			pixIn[i][k] = new UINT[Width];
		}
	}

	SetCommandList(0);
	GetVBarray2D(1);
	TextureInit(Width, Height);
	TexOn();
	CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
}

ImageRecognition::~ImageRecognition() {

	for (UINT k = 0; k < SearchMaxNum; k++) {
		for (UINT i = 0; i < Height; i++)ARR_DELETE(pixIn[k][i]);
	}
	for (UINT k = 0; k < SearchMaxNum; k++)ARR_DELETE(pixIn[k]);
	ARR_DELETE(pixIn);
	ARR_DELETE(numN);
	S_DELETE(nn);
	S_DELETE(po[0]);
	S_DELETE(cn[0]);
	S_DELETE(po[1]);
	S_DELETE(cn[1]);
	S_DELETE(sp[0]);
	S_DELETE(sp[1]);
	S_DELETE(sp[2]);
	ARR_DELETE(din);
	ARR_DELETE(learTexsepNum);
	ARR_DELETE(learTexsepInd);
}

void ImageRecognition::SetTarget(float *tar) {
	target = tar;
}

void ImageRecognition::query() {
	if (Type == 'C' || Type == 'D') {
		RunConvolutionToPooling(0);
	}
	if (Type == 'C' || Type == 'P') {
		RunPoolingToNN(0);
	}
	if (Type == 'D') {
		RunPoolingToConvolution();
		RunConvolutionToPooling(1);
		RunPoolingToNN(1);
	}
}

void ImageRecognition::queryDetec() {
	if (Type == 'C' || Type == 'D') {
		RunConvolutionToPoolingDetec(0);
	}
	if (Type == 'C' || Type == 'P') {
		RunPoolingToNNDetec(0);
	}
	if (Type == 'D') {
		RunPoolingToConvolutionDetec();
		RunConvolutionToPoolingDetec(1);
		RunPoolingToNNDetec(1);
	}
}

void ImageRecognition::Query() {
	queryDetec();
	nn->Query(sp[spInd]->SearchNum);

	UINT cnt = 0;
	for (int i = 0; i < sp[spInd]->SearchMaxNum; i++) {
		if (sp[spInd]->SearchOutInd[i] == -1)continue;
		sp[spInd]->out[i] = nn->GetOutputEl(0, cnt);
		if (sp[spInd]->out[i] < Threshold)sp[spInd]->Searchflg[i] = false;
		cnt++;
	}

	sp[spInd]->Sp->SetNNoutput(sp[spInd]->out);
	sp[spInd]->Sp->TextureDraw();

	sp[spInd]->Search10cnt++;
	if (sp[spInd]->Search10cnt > 5) {
		for (int i = 0; i < sp[spInd]->SearchMaxNum; i++)sp[spInd]->Searchflg[i] = true;
		sp[spInd]->Search10cnt = 0;
	}
}

void ImageRecognition::Training() {
	query();
	nn->Training();
	if (Type == 'D') {
		NNToPoolingBackPropagation(1);
		PoolingToConvolutionBackPropagation(1);
		ConvolutionToPoolingBackPropagation();
		PoolingToConvolutionBackPropagation(0);
	}
	if (Type == 'P') {
		NNToPoolingBackPropagation(0);
	}
	if (Type == 'C') {
		NNToPoolingBackPropagation(0);
		PoolingToConvolutionBackPropagation(0);
	}
}

void ImageRecognition::RunConvolutionToPooling(UINT ind) {
	cn[ind]->Query();
	po[ind]->SetInputResource(cn[ind]->GetOutputResource());
}

void ImageRecognition::RunPoolingToConvolution() {
	po[0]->Query();
	cn[1]->SetInputResource(po[0]->GetOutputResource());
}

void ImageRecognition::RunPoolingToNN(UINT ind) {
	po[ind]->Query();
	nn->SetInputResource(po[ind]->GetOutputResource());
}

void ImageRecognition::RunConvolutionToPoolingDetec(UINT ind) {
	cn[ind]->Detection(sp[spInd]->SearchNum);
	po[ind]->SetInputResource(cn[ind]->GetOutputResource());
}

void ImageRecognition::RunPoolingToConvolutionDetec() {
	po[0]->Detection(sp[spInd]->SearchNum);
	cn[1]->SetInputResource(po[0]->GetOutputResource());
}

void ImageRecognition::RunPoolingToNNDetec(UINT ind) {
	po[ind]->Detection(sp[spInd]->SearchNum);
	nn->SetInputResource(po[ind]->GetOutputResource());
}

void ImageRecognition::NNToPoolingBackPropagation(UINT ind) {
	po[ind]->SetInErrorResource(nn->GetOutErrorResource());
	po[ind]->Training();
}

void ImageRecognition::ConvolutionToPoolingBackPropagation() {
	po[0]->SetInErrorResource(cn[1]->GetOutErrorResource());
	po[0]->Training();
}

void ImageRecognition::PoolingToConvolutionBackPropagation(UINT ind) {
	cn[ind]->SetInErrorResource(po[ind]->GetOutErrorResource());
	cn[ind]->Training();
}

void ImageRecognition::SetLearningNum(UINT num) {
	learTexNum = num;
	learTexsepNum = new UINT[learTexNum];
	learTexsepInd = new UINT[learTexNum];
	for (UINT i = 0; i < learTexNum; i++) {
		D3D12_RESOURCE_DESC texdesc = GetTexture(i)->GetDesc();
		UINT width = texdesc.Width;
		UINT height = texdesc.Height;
		learTexsepNum[i] = (width / LEARTEXWID) * (height / LEARTEXWID);
		learTexsepInd[i] = 0;
		if (i < TextureLoader::GetlearningCorrectFaceFirstInd()) {
			negaNum += learTexsepNum[i];
		}
		else {
			posNum += learTexsepNum[i];
		}
	}
}

void ImageRecognition::LearningTexture() {

	D3D12_SUBRESOURCE_DATA texResource;
	GetTextureUp(learTexInd)->Map(0, nullptr, reinterpret_cast<void**>(&texResource));
	unsigned char *ptex = (unsigned char*)texResource.pData;
	D3D12_RESOURCE_DESC texdesc = GetTexture(learTexInd)->GetDesc();
	UINT Wid = texdesc.Width;
	UINT Hei = texdesc.Height;
	UINT sepW = Wid / LEARTEXWID;
	UINT sepH = Hei / LEARTEXWID;
	UINT pixWst = learTexsepInd[learTexInd] % sepW * LEARTEXWID;
	UINT pixHst = learTexsepInd[learTexInd] / sepW * LEARTEXWID;
	nn->SetTargetEl(target[learTexInd], 0);
	for (int j = pixHst; j < LEARTEXWID + pixHst; j++) {
		for (int i = pixWst; i < LEARTEXWID + pixWst; i++) {
			UINT pixWidNum = Wid * 4;
			UINT pInd = pixWidNum * j + i * 4;
			UINT pt = (ptex[pInd + 0] + ptex[pInd + 1] + ptex[pInd + 2]) / 3;
			pixIn[0][j - pixHst][i - pixWst] = ((UINT)ptex[pInd + 2] << 16) + ((UINT)ptex[pInd + 1] << 8) + ((UINT)ptex[pInd + 0]);
			float el = ((float)pt / 255.0f * 0.99f) + 0.01f;

			if (el < 0.0f) {
				MessageBoxA(0, "LearningTexture()ƒGƒ‰[", 0, MB_OK);
			}

			UINT nInd = LEARTEXWID * (j - pixHst) + (i - pixWst);
			switch (Type) {
			case 'C':
			case 'D':
				cn[0]->FirstInput(el, nInd);
				break;
			case 'P':
				po[0]->FirstInput(el, nInd);
				break;
			case 'N':
				nn->FirstInput(el, nInd);
				break;
			}
		}
	}
	GetTextureUp(learTexInd)->Unmap(0, nullptr);

	learTexsepInd[learTexInd]++;
	if (learTexsepInd[learTexInd] >= learTexsepNum[learTexInd]) {
		learTexsepInd[learTexInd] = 0;

		if (positivef == 0) {
			if (++posInd + TextureLoader::GetlearningCorrectFaceFirstInd() >= TextureLoader::GetLearningImageNum())posInd = 0;
		}
		else {
			if (++negaInd >= TextureLoader::GetlearningCorrectFaceFirstInd())negaInd = 0;
		}
	}

	if (++positivef > 2)positivef = 0;
	if (positivef == 0) {
		learTexInd = posInd + TextureLoader::GetlearningCorrectFaceFirstInd();
		poscnt++;
	}
	else {
		learTexInd = negaInd;
		negacnt++;
	}
}

void ImageRecognition::searchPixel() {
	sp[spInd]->Sp->SetPixel(sp[spInd]->spPix);
	sp[spInd]->Sp->SeparationTexture();
	UINT seaNum = sp[spInd]->SearchMaxNum;

	UINT cnt = 0;
	UINT seacnt = 0;
	for (UINT k = 0; k < seaNum; k++) {
		if (!sp[spInd]->Searchflg[k]) {
			sp[spInd]->SearchOutInd[k] = -1;
			cnt += (Width * Height);
			continue;
		}
		sp[spInd]->SearchOutInd[k] = k;
		for (UINT i = 0; i < Width * Height; i++) {
			float el = sp[spInd]->Sp->GetOutputEl(cnt++);
			UINT pixX = i % Width;
			UINT pixY = i / Width;
			pixIn[seacnt][pixY][pixX] = ((UINT)(el * 255.0f) << 16) + ((UINT)(el * 255.0f) << 8) + ((UINT)(el * 255.0f));
			switch (Type) {
			case 'C':
			case 'D':
				cn[0]->FirstInput(el, i, seacnt);
				break;
			case 'P':
				po[0]->FirstInput(el, i, seacnt);
				break;
			case 'N':
				nn->FirstInput(el, i, seacnt);
				break;
			}
		}
		seacnt++;
	}
	sp[spInd]->SearchNum = seacnt;
}

void ImageRecognition::InputTexture(int Tno) {
	spInd++;
	if (spInd > 2)spInd = 0;
	D3D12_SUBRESOURCE_DATA texResource;
	GetTextureUp(Tno)->Map(0, nullptr, reinterpret_cast<void**>(&texResource));
	UCHAR *ptex = (UCHAR*)texResource.pData;
	for (UINT i = 0; i < InW * InH; i++) {
		UCHAR tmp = (ptex[i * 4] + ptex[i * 4 + 1] + ptex[i * 4 + 2]) / 3;
		sp[spInd]->spPix[i] = ((float)tmp / 255.0f * 0.99f) + 0.01f;
	}
	GetTextureUp(Tno)->Unmap(0, nullptr);

	sp[spInd]->Sp->SetPixel3ch(GetTexture(Tno));
	searchPixel();
}

void ImageRecognition::InputPixel(BYTE *pix) {
	spInd++;
	if (spInd > 2)spInd = 0;
	for (UINT i = 0; i < InW * InH; i++) {
		BYTE tmp = (pix[i * 4] + pix[i * 4 + 1] + pix[i * 4 + 2]) / 3;
		sp[spInd]->spPix[i] = ((float)tmp / 255.0f * 0.99f) + 0.01f;
	}

	sp[spInd]->Sp->SetPixel3ch(pix);
	searchPixel();
}

void ImageRecognition::NNDraw() {
	dnn.Update(705.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, 25.0f * filNum);
	dnn.CopyResource(nn->GetNNTextureResource(), nn->GetNNTextureResourceStates());
	dnn.Draw();
}

void ImageRecognition::PODraw() {
	if (Type == 'C' || Type == 'P' || Type == 'D') {
		dpo[0].Update(125.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, 25.0f * filNum);
		dpo[0].CopyResource(po[0]->GetNNTextureResource(), po[0]->GetNNTextureResourceStates());
		dpo[0].Draw();
	}
	if (Type == 'D') {
		dpo[1].Update(365.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, 25.0f * filNum);
		dpo[1].CopyResource(po[1]->GetNNTextureResource(), po[1]->GetNNTextureResourceStates());
		dpo[1].Draw();
	}
}

void ImageRecognition::CNDraw() {
	if (Type == 'C' || Type == 'D') {
		dcn[0].Update(5.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, 25.0f * filNum);
		dcn[0].CopyResource(cn[0]->GetNNTextureResource(), cn[0]->GetNNTextureResourceStates());
		dcn[0].Draw();
	}
	if (Type == 'D') {
		dcn[1].Update(245.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, 25.0f * filNum);
		dcn[1].CopyResource(cn[1]->GetNNTextureResource(), cn[1]->GetNNTextureResourceStates());
		dcn[1].Draw();
	}
}

void ImageRecognition::INDraw(float x, float y, float xsize, float ysize) {

	UINT cnt = 0;
	for (int i = 0; i < sp[spInd]->SearchMaxNum; i++) {
		din[i].Update(cnt * 52.0f + x, 548.0f + y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 52.0f + xsize, 52.0f + ysize);
		if (sp[spInd]->SearchMaxNum == 1 || Threshold <= sp[spInd]->out[i]) {
			din[i].SetTextureMPixel(pixIn[i], 0xff, 0xff, 0xff, 255);
			cnt++;
			din[i].Draw();
		}
	}
}

void ImageRecognition::SPDraw() {
	sp[spInd]->dsp[0].Update(50.0f, 200.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 300.0f, 200.0f);
	sp[spInd]->dsp[0].CopyResource(sp[spInd]->Sp->GetNNTextureResource(), sp[spInd]->Sp->GetNNTextureResourceStates());
	sp[spInd]->dsp[0].Draw();
	sp[spInd]->dsp[1].Update(350.0f, 200.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 300.0f, 200.0f);
	sp[spInd]->dsp[1].CopyResource(sp[spInd]->Sp->GetOutputResource(), sp[spInd]->Sp->GetNNTextureResourceStates());
	sp[spInd]->dsp[1].Draw();
}

void ImageRecognition::textDraw(UINT stateNum, float x, float y) {

	if (stateNum == 0)return;

	DxText::GetInstance()->UpDateText(L"“ü—Í‰æ‘œ ", 10.0f, 480.0f, 15.0f, { 1.0f, 0.0f, 0.0f, 1.0f });
	if (Type != 'P' && Type != 'N')DxText::GetInstance()->UpDateText(L"ôž‚Ý‘w", 0.0f, 7.0f, 15.0f, { 1.0f, 0.5f, 0.5f, 1.0f });
	if (Type != 'N')DxText::GetInstance()->UpDateText(L"ƒv[ƒŠƒ“ƒO‘w", 100.0f, 7.0f, 15.0f, { 0.5f, 1.0f, 0.5f, 1.0f });
	DxText::GetInstance()->UpDateText(L"‘SŒ‹‡‘w‹t•ûŒüo—Í ", 650.0f, 7.0f, 15.0f, { 0.5f, 0.5f, 1.0f, 1.0f });
	if (Type == 'D') {
		DxText::GetInstance()->UpDateText(L"ôž‚Ý‘w", 240.0f, 7.0f, 15.0f, { 1.0f, 0.5f, 0.5f, 1.0f });
		DxText::GetInstance()->UpDateText(L"ƒv[ƒŠƒ“ƒO‘w", 340.0f, 7.0f, 15.0f, { 0.5f, 1.0f, 0.5f, 1.0f });
	}

	int tm;
	float tm1;
	switch (stateNum) {
	case 1:
		DxText::GetInstance()->UpDateText(L"ŠwK’†o—Í ", 600.0f, 430.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		tm1 = nn->GetOutputEl(0);
		tm = tm1 * 100;
		if (tm < 0 || tm > 100) {
			MessageBoxA(0, "ŠwK’†o—ÍƒGƒ‰[", 0, MB_OK);
			char st1[50];
			sprintf(st1, "%d", tm);
			MessageBoxA(0, st1, 0, MB_OK);
			char st2[50];
			sprintf(st2, "%f", tm1);
			MessageBoxA(0, st2, 0, MB_OK);
		}
		DxText::GetInstance()->UpDateValue(tm, 710.0f, 430.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateText(L"³‰ð‰æ‘œŠwK ", 600.0f, 445.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateText(L"•s³‰ð‰æ‘œŠwK ", 600.0f, 460.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateValue(poscnt, 750.0f, 445.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateValue(negacnt, 750.0f, 460.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateText(L"³‰ð‰æ‘œ” ", 600.0f, 475.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateText(L"•s³‰ð‰æ‘œ” ", 600.0f, 490.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateValue(posNum, 750.0f, 475.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateValue(negaNum, 750.0f, 490.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		break;
	case 2:
		DxText::GetInstance()->UpDateText(L"èø‚Å‚ ‚éŠm—¦ ", 10.0f + x, 500.0f + y, 15.0f, { 0.0f, 1.0f, 0.0f, 1.0f });
		UINT cnt = 0;
		for (int i = 0; i < sp[spInd]->SearchMaxNum && cnt < 16; i++) {
			if (Threshold <= sp[spInd]->out[i]) {
				DxText::GetInstance()->UpDateValue(sp[spInd]->out[i] * 100, cnt * 52.0f + x, 530.0f + y, 15.0f, 3, { 0.0f, 1.0f, 0.0f, 1.0f });
				DxText::GetInstance()->UpDateText(L"%\ ", 40.0f + cnt * 52.0f + x, 530.0f + y, 15.0f, { 0.0f, 1.0f, 0.0f, 1.0f });
				cnt++;
			}
		}
		break;
	}
}

void ImageRecognition::SaveData() {
	if (cn[0])cn[0]->SaveData(0);
	if (cn[1])cn[1]->SaveData(1);
	nn->SaveData();
}

void ImageRecognition::LoadData() {
	if (cn[0])cn[0]->LoadData(0);
	if (cn[1])cn[1]->LoadData(1);
	nn->LoadData();
}
