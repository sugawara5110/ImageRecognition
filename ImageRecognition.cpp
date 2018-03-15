//*****************************************************************************************//
//**                                                                                     **//
//**                              ImageRecognition                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "ImageRecognition.h"

ImageRecognition::ImageRecognition(UINT srcWid, UINT srcHei, UINT width, UINT height, UINT *numNode, int depth, UINT filnum, UCHAR type, bool searchOn, float threshold) {

	Threshold = threshold;
	Width = width;
	Height = height;
	filNum = filnum;
	Type = type;
	InW = srcWid;
	InH = srcHei;

	sp = new SearchPixel(InW, InH, Width, Height, 32, numNode[depth - 1], Threshold);
	sp->ComCreate();
	UINT spow = sp->GetOutWid();
	UINT spoh = sp->GetOutHei();
	if (searchOn)SearchNum = sp->GetSearchNum();
	else SearchNum = 1;
	out = new float[SearchNum];
	spPix = new float[InW * InH];
	sp->CreareNNTexture(spow, spoh, 1);
	dsp[0].SetCommandList(0);
	dsp[0].GetVBarray2D(1);
	dsp[0].TextureInit(spow, spoh);
	dsp[0].TexOn();
	dsp[0].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	dsp[1].SetCommandList(0);
	dsp[1].GetVBarray2D(1);
	dsp[1].TextureInit(InW, InH);
	dsp[1].TexOn();
	dsp[1].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);

	unsigned int wid = Width;
	unsigned int hei = Height;
	if (Type == 'C' || Type == 'D') {
		cn[0] = new DxConvolution(wid, hei, filNum, SearchNum, 5, 1);
		cn[0]->ComCreate();
		wid = cn[0]->GetOutWidth();
		hei = cn[0]->GetOutHeight();
		cn[0]->CreareNNTexture(wid, hei, filNum);

		dcn[0].SetCommandList(0);
		dcn[0].GetVBarray2D(1);
		dcn[0].TextureInit(wid, hei * filNum);
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
		po[0] = new DxPooling(wid, hei, filNum, SearchNum);
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
		cn[1] = new DxConvolution(wid, hei, filNum, SearchNum, 3, 1);
		cn[1]->ComCreate();
		wid = cn[1]->GetOutWidth();
		hei = cn[1]->GetOutHeight();
		cn[1]->CreareNNTexture(wid, hei, filNum);

		dcn[1].SetCommandList(0);
		dcn[1].GetVBarray2D(1);
		dcn[1].TextureInit(wid, hei * filNum);
		dcn[1].TexOn();
		dcn[1].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);

		po[1] = new DxPooling(wid, hei, filNum, SearchNum);
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
	nn = new DxNeuralNetwork(numN, depth + 1, filNum, SearchNum);
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

	din = new PolygonData2D[SearchNum];
	for (UINT i = 0; i < SearchNum; i++) {
		din[i].SetCommandList(0);
		din[i].GetVBarray2D(1);
		din[i].TextureInit(Width, Height);
		din[i].TexOn();
		din[i].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	}
	pixIn = new UINT**[SearchNum];
	for (UINT i = 0; i < SearchNum; i++) {
		pixIn[i] = new UINT*[Height];
	}
	for (UINT i = 0; i < SearchNum; i++) {
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

	for (UINT k = 0; k < SearchNum; k++) {
		for (UINT i = 0; i < Height; i++)ARR_DELETE(pixIn[k][i]);
	}
	for (UINT k = 0; k < SearchNum; k++)ARR_DELETE(pixIn[k]);
	ARR_DELETE(pixIn);
	ARR_DELETE(spPix);
	ARR_DELETE(pixIn);
	ARR_DELETE(numN);
	SIN_DELETE(nn);
	SIN_DELETE(sp);
	SIN_DELETE(po[0]);
	SIN_DELETE(cn[0]);
	SIN_DELETE(po[1]);
	SIN_DELETE(cn[1]);
	ARR_DELETE(din);
	ARR_DELETE(out);
}

void ImageRecognition::SetTargetEl(float el, unsigned int Num) {
	nn->SetTargetEl(el, Num);
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
	nn->Query();
	for (int i = 0; i < SearchNum; i++)
		out[i] = nn->GetOutputEl(0, i);

	sp->SetNNoutput(nn->GetOutputResource());
	sp->TextureDraw();
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
	cn[ind]->Detection();
	po[ind]->SetInputResource(cn[ind]->GetOutputResource());
}

void ImageRecognition::RunPoolingToConvolutionDetec() {
	po[0]->Detection();
	cn[1]->SetInputResource(po[0]->GetOutputResource());
}

void ImageRecognition::RunPoolingToNNDetec(UINT ind) {
	po[ind]->Detection();
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

void ImageRecognition::LearningTexture(int Tno, int dir) {

	D3D12_RESOURCE_DESC texdesc;
	texdesc = GetTexture(Tno)->GetDesc();
	//テクスチャの横サイズ取得
	float width = (float)texdesc.Width;
	//テクスチャの縦サイズ取得
	float height = (float)texdesc.Height;
	TexNo = Tno;
	D3D12_SUBRESOURCE_DATA texResource;

	GetTextureUp(Tno)->Map(0, nullptr, reinterpret_cast<void**>(&texResource));

	unsigned char *ptex = (unsigned char*)texResource.pData;

	for (int j = 0; j < height; j++) {
		unsigned int j1 = (unsigned int)((float)j * (width * 4.0f));//RowPitchデータの行ピッチ、行幅、または物理サイズ (バイト単位)
		for (int i = 0; i < width; i++) {
			unsigned int ptexI = i * 4 + j1;
			unsigned int pt = (ptex[ptexI + 0] + ptex[ptexI + 1] + ptex[ptexI + 2]) / 3;
			float el = ((float)pt / 255.0f * 0.99f) + 0.01f;
			UINT ind = 0;

			UINT i2 = (UINT)(((float)Width / width) * (float)i);
			UINT j2 = (UINT)(((float)Height / height) * (float)j);

			switch (dir) {
			case 0:
				ind = Width * j2 + i2;
				break;
			case 1:
				ind = Width * (Height - 1 - j2) + i2;
				break;
			case 2:
				ind = Width * j2 + (Width - 1 - i2);
				break;
			case 3:
				ind = Width * (Height - 1 - j2) + (Width - 1 - i2);
				break;
			}
			switch (Type) {
			case 'C':
			case 'D':
				cn[0]->FirstInput(el, ind);
				break;
			case 'P':
				po[0]->FirstInput(el, ind);
				break;
			case 'N':
				nn->FirstInput(el, ind);
				break;
			}
		}
	}
	GetTextureUp(Tno)->Unmap(0, nullptr);
	InTex = true;
}

void ImageRecognition::searchPixel(int Tno) {

	D3D12_SUBRESOURCE_DATA texResource;
	GetTextureUp(Tno)->Map(0, nullptr, reinterpret_cast<void**>(&texResource));
	UCHAR *ptex = (UCHAR*)texResource.pData;
	for (UINT i = 0; i < InW * InH; i++) {
		UCHAR tmp = (ptex[i * 4] + ptex[i * 4 + 1] + ptex[i * 4 + 2]) / 3;
		spPix[i] = ((float)tmp / 255.0f * 0.99f) + 0.01f;
	}
	GetTextureUp(Tno)->Unmap(0, nullptr);
	sp->SetPixel3ch(GetTexture(Tno));
	sp->SetPixel(spPix);
	sp->SeparationTexture();
	UINT seaNum = sp->GetSearchNum();

	UINT cnt = 0;
	for (UINT k = 0; k < seaNum; k++) {
		for (UINT i = 0; i < Width * Height; i++) {
			float el = sp->GetOutputEl(cnt++);
			UINT pixX = i % Width;
			UINT pixY = i / Width;
			pixIn[k][pixY][pixX] = ((UINT)(el * 255.0f) << 16) + ((UINT)(el * 255.0f) << 8) + ((UINT)(el * 255.0f));
			switch (Type) {
			case 'C':
			case 'D':
				cn[0]->FirstInput(el, i, k);
				break;
			case 'P':
				po[0]->FirstInput(el, i, k);
				break;
			case 'N':
				nn->FirstInput(el, i, k);
				break;
			}
		}
	}
	InTex = false;
}

void ImageRecognition::InputPixel(BYTE *pix) {

	for (UINT i = 0; i < InW * InH; i++) {
		BYTE tmp = (pix[i * 4] + pix[i * 4 + 1] + pix[i * 4 + 2]) / 3;
		spPix[i] = ((float)tmp / 255.0f * 0.99f) + 0.01f;
	}

	sp->SetPixel3ch(pix);
	sp->SetPixel(spPix);
	sp->SeparationTexture();
	UINT seaNum = sp->GetSearchNum();

	UINT cnt = 0;
	for (UINT k = 0; k < seaNum; k++) {
		for (UINT i = 0; i < Width * Height; i++) {
			float el = sp->GetOutputEl(cnt++);
			UINT pixX = i % Width;
			UINT pixY = i / Width;
			pixIn[k][pixY][pixX] = ((UINT)(el * 255.0f) << 16) + ((UINT)(el * 255.0f) << 8) + ((UINT)(el * 255.0f));
			switch (Type) {
			case 'C':
			case 'D':
				cn[0]->FirstInput(el, i, k);
				break;
			case 'P':
				po[0]->FirstInput(el, i, k);
				break;
			case 'N':
				nn->FirstInput(el, i, k);
				break;
			}
		}
	}
	InTex = false;
}

void ImageRecognition::NNDraw() {
	dnn.Update(705.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 75.0f, 75.0f * filNum);
	dnn.CopyResource(nn->GetNNTextureResource(), nn->GetNNTextureResourceStates());
	dnn.Draw();
}

void ImageRecognition::PODraw() {
	if (Type == 'C' || Type == 'P' || Type == 'D') {
		dpo[0].Update(125.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 75.0f, 75.0f * filNum);
		dpo[0].CopyResource(po[0]->GetNNTextureResource(), po[0]->GetNNTextureResourceStates());
		dpo[0].Draw();
	}
	if (Type == 'D') {
		dpo[1].Update(365.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 75.0f, 75.0f * filNum);
		dpo[1].CopyResource(po[1]->GetNNTextureResource(), po[1]->GetNNTextureResourceStates());
		dpo[1].Draw();
	}
}

void ImageRecognition::CNDraw() {
	if (Type == 'C' || Type == 'D') {
		dcn[0].Update(5.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 75.0f, 75.0f * filNum);
		dcn[0].CopyResource(cn[0]->GetNNTextureResource(), cn[0]->GetNNTextureResourceStates());
		dcn[0].Draw();
	}
	if (Type == 'D') {
		dcn[1].Update(245.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 75.0f, 75.0f * filNum);
		dcn[1].CopyResource(cn[1]->GetNNTextureResource(), cn[1]->GetNNTextureResourceStates());
		dcn[1].Draw();
	}
}

void ImageRecognition::INDraw(float x, float y, float xsize, float ysize) {

	UINT cnt = 0;
	for (int i = 0; i < SearchNum; i++) {
		din[i].Update(cnt * 52.0f + x, 548.0f + y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 52.0f + xsize, 52.0f + ysize);
		if (!InTex) {
			if (Threshold <= out[i]) {
				din[i].SetTextureMPixel(pixIn[i], 0xff, 0xff, 0xff, 255);
				cnt++;
			}
		}
		else
			din[i].CopyResource(GetTexture(TexNo), GetTextureStates());
		din[i].Draw();
	}
}

void ImageRecognition::SPDraw() {
	dsp[0].Update(50.0f, 200.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 300.0f, 200.0f);
	dsp[0].CopyResource(sp->GetNNTextureResource(), sp->GetNNTextureResourceStates());
	dsp[0].Draw();
	dsp[1].Update(350.0f, 200.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 300.0f, 200.0f);
	dsp[1].CopyResource(sp->GetOutputResource(), sp->GetNNTextureResourceStates());
	dsp[1].Draw();
}

void ImageRecognition::textDraw(UINT stateNum, float x, float y) {

	if (stateNum == 0)return;

	DxText::GetInstance()->UpDateText(L"入力画像 ", 10.0f, 480.0f, 15.0f, { 1.0f, 0.0f, 0.0f, 1.0f });
	if (Type != 'P' && Type != 'N')DxText::GetInstance()->UpDateText(L"畳込み層出力 ", 0.0f, 7.0f, 15.0f, { 1.0f, 0.5f, 0.5f, 1.0f });
	if (Type != 'N')DxText::GetInstance()->UpDateText(L"プーリング層出力 ", 100.0f, 7.0f, 15.0f, { 0.5f, 1.0f, 0.5f, 1.0f });
	DxText::GetInstance()->UpDateText(L"全結合層逆方向出力 ", 650.0f, 7.0f, 15.0f, { 0.5f, 0.5f, 1.0f, 1.0f });
	if (Type == 'D') {
		DxText::GetInstance()->UpDateText(L"畳込み層出力 ", 240.0f, 7.0f, 15.0f, { 1.0f, 0.5f, 0.5f, 1.0f });
		DxText::GetInstance()->UpDateText(L"プーリング層出力 ", 340.0f, 7.0f, 15.0f, { 0.5f, 1.0f, 0.5f, 1.0f });
	}

	switch (stateNum) {
	case 1:
		DxText::GetInstance()->UpDateText(L"学習中出力 ", 600.0f, 460.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateValue(nn->GetOutputEl(0) * 100, 710.0f, 460.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		break;
	case 2:
		DxText::GetInstance()->UpDateText(L"顏である確率 ", 10.0f + x, 500.0f + y, 15.0f, { 0.0f, 1.0f, 0.0f, 1.0f });
		UINT cnt = 0;
		for (int i = 0; i < SearchNum; i++) {
			if (Threshold <= out[i]) {
				DxText::GetInstance()->UpDateValue(out[i] * 100, 5.0f + cnt * 52.0f + x, 530.0f + y, 15.0f, 2, { 0.0f, 1.0f, 0.0f, 1.0f });
				DxText::GetInstance()->UpDateText(L"%\ ", 30.0f + cnt * 52.0f + x, 530.0f + y, 15.0f, { 0.0f, 1.0f, 0.0f, 1.0f });
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
