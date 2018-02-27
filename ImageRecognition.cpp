//*****************************************************************************************//
//**                                                                                     **//
//**                              ImageRecognition                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "ImageRecognition.h"

ImageRecognition::ImageRecognition(UINT width, UINT height, UINT *numNode, int depth, UINT filnum, UCHAR type, int testImageNum) {

	TestImageNum = testImageNum;
	out = new float[TestImageNum];
	Width = width;
	Height = height;
	filNum = filnum;
	Type = type;

	unsigned int wid = Width;
	unsigned int hei = Height;
	if (Type == 'C' || Type == 'D') {
		cn[0] = new ConvolutionNN(wid, hei, filNum, 3, 1);
		wid = cn[0]->GetOutWidth();
		hei = cn[0]->GetOutHeight();

		pixCN[0] = new UINT*[hei * filNum];
		for (UINT i = 0; i < hei * filNum; i++) {
			pixCN[0][i] = new UINT[wid];
		}

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
		po[0] = new DxPooling(wid, hei, filNum);
		po[0]->ComCreate();
		wid = po[0]->GetOutWidth();
		hei = po[0]->GetOutHeight();

		pixPO[0] = new UINT*[hei * filNum];
		for (UINT i = 0; i < hei * filNum; i++) {
			pixPO[0][i] = new UINT[wid];
		}

		dpo[0].SetCommandList(0);
		dpo[0].GetVBarray2D(1);
		dpo[0].TextureInit(wid, hei * filNum);
		dpo[0].TexOn();
		dpo[0].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	}
	if (Type == 'D') {
		cn[1] = new ConvolutionNN(wid, hei, filNum);
		wid = cn[1]->GetOutWidth();
		hei = cn[1]->GetOutHeight();

		pixCN[1] = new UINT*[hei * filNum];
		for (UINT i = 0; i < hei * filNum; i++) {
			pixCN[1][i] = new UINT[wid];
		}

		dcn[1].SetCommandList(0);
		dcn[1].GetVBarray2D(1);
		dcn[1].TextureInit(wid, hei * filNum);
		dcn[1].TexOn();
		dcn[1].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);

		po[1] = new DxPooling(wid, hei, filNum);
		po[1]->ComCreate();
		wid = po[1]->GetOutWidth();
		hei = po[1]->GetOutHeight();

		pixPO[1] = new UINT*[hei * filNum];
		for (UINT i = 0; i < hei * filNum; i++) {
			pixPO[1][i] = new UINT[wid];
		}

		dpo[1].SetCommandList(0);
		dpo[1].GetVBarray2D(1);
		dpo[1].TextureInit(wid, hei * filNum);
		dpo[1].TexOn();
		dpo[1].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	}

	pixNN = new UINT*[hei * filNum];
	for (UINT i = 0; i < hei * filNum; i++)pixNN[i] = new UINT[wid];
	numN = new UINT[depth + 1];
	numN[0] = wid * hei;
	for (int i = 1; i < depth + 1; i++)numN[i] = numNode[i - 1];
	nn = new DxNeuralNetwork(numN, depth + 1, filNum);
	nn->ComCreate();

	dnn.SetCommandList(0);
	dnn.GetVBarray2D(1);
	dnn.TextureInit(wid, hei * filNum);
	dnn.TexOn();
	dnn.CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	nnWidth = wid;
	nnHeight = hei;

	Depth = depth + 1;

	din = new PolygonData2D[TestImageNum];
	for (int i = 0; i < TestImageNum; i++) {
		din[i].SetCommandList(0);
		din[i].GetVBarray2D(1);
		din[i].TextureInit(Width, Height);
		din[i].TexOn();
		din[i].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	}
	pixIn = new UINT**[TestImageNum];
	for (int k = 0; k < TestImageNum; k++) {
		pixIn[k] = new UINT*[Height];
	}
	for (int k = 0; k < TestImageNum; k++) {
		for (UINT i = 0; i < Height; i++) {
			pixIn[k][i] = new UINT[Width];
		}
	}
	SetCommandList(0);
	GetVBarray2D(1);
	TextureInit(Width, Height);
	TexOn();
	CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
}

ImageRecognition::~ImageRecognition() {
	for (UINT k = 0; k < 2; k++) {
		if (pixCN[k] != nullptr) {
			for (UINT i = 0; i < cn[k]->GetOutHeight() * filNum; i++)ARRAY_DELETE(pixCN[k][i]);
			ARRAY_DELETE(pixCN[k]);
		}
		if (pixPO[k] != nullptr) {
			for (UINT i = 0; i < po[k]->GetOutHeight() * filNum; i++)ARRAY_DELETE(pixPO[k][i]);
			ARRAY_DELETE(pixPO[k]);
		}
	}
	for (UINT i = 0; i < nnHeight * filNum; i++)ARRAY_DELETE(pixNN[i]);
	ARRAY_DELETE(pixNN);
	for (int k = 0; k < TestImageNum; k++) {
		for (UINT i = 0; i < Height; i++)ARRAY_DELETE(pixIn[k][i]);
	}
	for (int k = 0; k < TestImageNum; k++)ARRAY_DELETE(pixIn[k]);
	ARRAY_DELETE(pixIn);
	ARRAY_DELETE(numN);
	SIN_DELETE(nn);
	SIN_DELETE(po[0]);
	SIN_DELETE(cn[0]);
	SIN_DELETE(po[1]);
	SIN_DELETE(cn[1]);
	ARRAY_DELETE(din);
	ARRAY_DELETE(out);
}

void ImageRecognition::SetTargetEl(float el, unsigned int Num) {
	nn->SetTargetEl(el, Num);
}

void ImageRecognition::query() {
	if (Type == 'C' || Type == 'D') {
		RunConvolutionToPooling(0);
		ConvolutionInputPixel(0);
	}
	if (Type == 'C' || Type == 'P') {
		RunPoolingToNN(0);
		PoolingInputPixel(0);
	}
	if (Type == 'D') {
		RunPoolingToConvolution();
		PoolingInputPixel(0);
		RunConvolutionToPooling(1);
		ConvolutionInputPixel(1);
		RunPoolingToNN(1);
		PoolingInputPixel(1);
	}
}

void ImageRecognition::Query() {
	query();
	nn->Query();
	out[testimInd] = nn->GetOutputEl(0);
	if (testimInd == TestImageNum - 1)textDrawOn = true;
	testimInd++;
	testimInd = testimInd % TestImageNum;
	InverseQueryInputPixel();
}

void ImageRecognition::Training() {
	query();
	nn->Training();
	InverseQueryInputPixel();
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
	cn[ind]->ForwardPropagation();
	for (unsigned int k = 0; k < filNum; k++) {
		po[ind]->Input(cn[ind]->Output(k), k);
	}
}

void ImageRecognition::RunPoolingToConvolution() {
	po[0]->Query();
	for (unsigned int k = 0; k < filNum; k++) {
		cn[1]->Input(po[0]->Output(k), k);
	}
}

void ImageRecognition::RunPoolingToNN(UINT ind) {
	po[ind]->Query();
	for (unsigned int k = 0; k < filNum; k++) {
		nn->InputArray(po[ind]->Output(k), k);
	}
}

void ImageRecognition::NNToPoolingBackPropagation(UINT ind) {
	for (unsigned int k = 0; k < filNum; k++) {
		po[ind]->InputError(nn->GetError(k), k);
	}
	po[ind]->Training();
}

void ImageRecognition::ConvolutionToPoolingBackPropagation() {
	for (unsigned int k = 0; k < filNum; k++)
		po[0]->InputError(cn[1]->GetError(k), k);
	po[0]->Training();
}

void ImageRecognition::PoolingToConvolutionBackPropagation(UINT ind) {
	for (unsigned int k = 0; k < filNum; k++)
		cn[ind]->InputError(po[ind]->GetError(k), k);
	cn[ind]->BackPropagation();
}

void ImageRecognition::InputTexture(int Tno, int dir) {

	D3D12_RESOURCE_DESC texdesc;
	texdesc = GetTexture(Tno)->GetDesc();
	//テクスチャの横サイズ取得
	float width = (float)texdesc.Width;
	//テクスチャの縦サイズ取得
	float height = (float)texdesc.Height;

	D3D12_SUBRESOURCE_DATA texResource;

	GetTextureUp(Tno)->Map(0, nullptr, reinterpret_cast<void**>(&texResource));

	unsigned char *ptex = (unsigned char*)texResource.pData;

	for (int j = 0; j < height; j++) {
		unsigned int j1 = (unsigned int)((float)j * (width * 4.0f));//RowPitchデータの行ピッチ、行幅、または物理サイズ (バイト単位)
		for (int i = 0; i < width; i++) {
			unsigned int ptexI = i * 4 + j1;
			unsigned int pt = (ptex[ptexI + 0] + ptex[ptexI + 1] + ptex[ptexI + 2]) / 3;
			float el = ((float)(255.0f - pt) / 255.0f * 0.99f) + 0.01f;
			UINT ind = 0;

			UINT i2 = (UINT)(((float)Width / width) * (float)i);
			UINT j2 = (UINT)(((float)Height / height) * (float)j);
			pixIn[testimInd][j2][i2] = (ptex[ptexI + 2] & 0xff) << 16;
			pixIn[testimInd][j2][i2] += (ptex[ptexI + 1] & 0xff) << 8;
			pixIn[testimInd][j2][i2] += (ptex[ptexI + 0] & 0xff);

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
}

void ImageRecognition::InputPixel(UINT **pix, UINT width, UINT height) {
	for (UINT j = 0; j < height; j++) {
		for (UINT i = 0; i < width; i++) {
			UINT pt = ((pix[j][i] >> 16 & 0xff) + (pix[j][i] >> 8 & 0xff) + (pix[j][i] & 0xff)) / 3;
			float el = ((float)(255.0f - pt) / 255.0f * 0.99f) + 0.01f;
			UINT ind = width * j + i;
			pixIn[0][j][i] = pix[j][i];
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
}

void ImageRecognition::InverseQueryInputPixel() {
	for (unsigned int k = 0; k < filNum; k++) {
		for (unsigned int j = 0; j < nnHeight; j++) {
			for (unsigned int i = 0; i < nnWidth; i++) {
				float tmp = nn->GetInverseOutputEl(k, nnWidth * j + i);
				pixNN[nnHeight * k + j][i] = (((unsigned int)(tmp * 255)) & 0xff) << 16;
				pixNN[nnHeight * k + j][i] += (((unsigned int)(tmp * 255)) & 0xff) << 8;
				pixNN[nnHeight * k + j][i] += ((unsigned int)(tmp * 255)) & 0xff;
			}
		}
	}
}

void ImageRecognition::PoolingInputPixel(UINT ind) {
	unsigned int wid = po[ind]->GetOutWidth();
	unsigned int hei = po[ind]->GetOutHeight();

	for (unsigned int k = 0; k < filNum; k++) {
		for (unsigned int j = 0; j < hei; j++) {
			for (unsigned int i = 0; i < wid; i++) {
				float tmp = po[ind]->OutputEl(k, wid * j + i);
				pixPO[ind][hei * k + j][i] = (((unsigned int)(tmp * 255)) & 0xff) << 16;
				pixPO[ind][hei * k + j][i] += (((unsigned int)(tmp * 255)) & 0xff) << 8;
				pixPO[ind][hei * k + j][i] += ((unsigned int)(tmp * 255)) & 0xff;
			}
		}
	}
}

void ImageRecognition::ConvolutionInputPixel(UINT ind) {
	unsigned int wid = cn[ind]->GetOutWidth();
	unsigned int hei = cn[ind]->GetOutHeight();

	for (unsigned int k = 0; k < filNum; k++) {
		for (unsigned int j = 0; j < hei; j++) {
			for (unsigned int i = 0; i < wid; i++) {
				float tmp = cn[ind]->OutputEl(k, wid * j + i);
				pixCN[ind][hei * k + j][i] = (((unsigned int)(tmp * 255)) & 0xff) << 16;
				pixCN[ind][hei * k + j][i] += (((unsigned int)(tmp * 255)) & 0xff) << 8;
				pixCN[ind][hei * k + j][i] += ((unsigned int)(tmp * 255)) & 0xff;
			}
		}
	}
}

void ImageRecognition::NNDraw() {
	dnn.Update(705.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 75.0f, 75.0f * filNum);
	dnn.SetTextureMPixel(pixNN, 0xff, 0xff, 0xff, 255);
	dnn.Draw();
}

void ImageRecognition::PODraw() {
	if (Type == 'C' || Type == 'P' || Type == 'D') {
		dpo[0].Update(125.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 75.0f, 75.0f * filNum);
		dpo[0].SetTextureMPixel(pixPO[0], 0xff, 0xff, 0xff, 255);
		dpo[0].Draw();
	}
	if (Type == 'D') {
		dpo[1].Update(365.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 75.0f, 75.0f * filNum);
		dpo[1].SetTextureMPixel(pixPO[1], 0xff, 0xff, 0xff, 255);
		dpo[1].Draw();
	}
}

void ImageRecognition::CNDraw() {
	if (Type == 'C' || Type == 'D') {
		dcn[0].Update(5.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 75.0f, 75.0f * filNum);
		dcn[0].SetTextureMPixel(pixCN[0], 0xff, 0xff, 0xff, 255);
		dcn[0].Draw();
	}
	if (Type == 'D') {
		dcn[1].Update(245.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 75.0f, 75.0f * filNum);
		dcn[1].SetTextureMPixel(pixCN[1], 0xff, 0xff, 0xff, 255);
		dcn[1].Draw();
	}
}

void ImageRecognition::INDraw(float x, float y, float xsize, float ysize) {
	for (int i = 0; i < TestImageNum; i++) {
		din[i].Update(i * 52.0f + x, 548.0f + y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 52.0f + xsize, 52.0f + ysize);
		din[i].SetTextureMPixel(pixIn[i], 0xff, 0xff, 0xff, 255);
		din[i].Draw();
	}
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
		if (!textDrawOn)break;
		DxText::GetInstance()->UpDateText(L"顏である確率 ", 10.0f + x, 500.0f + y, 15.0f, { 0.0f, 1.0f, 0.0f, 1.0f });
		for (int i = 0; i < TestImageNum; i++) {
			DxText::GetInstance()->UpDateValue(out[i] * 100, 5.0f + i * 52.0f + x, 530.0f + y, 15.0f, 2, { 0.0f, 1.0f, 0.0f, 1.0f });
			DxText::GetInstance()->UpDateText(L"%\ ", 30.0f + i * 52.0f + x, 530.0f + y, 15.0f, { 0.0f, 1.0f, 0.0f, 1.0f });
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
