//*****************************************************************************************//
//**                                                                                     **//
//**                              ImageRecognition                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "ImageRecognition.h"
#include "TextureLoader.h"
#define LEARTEXWID 64
#define BADGENUM 16

SP::SP(UINT srcwid, UINT srchei, UINT seawid, UINT seahei, float outscale, UINT step, UINT outNum, float Threshold, bool searchOn) {
	Sp = new SearchPixel(srcwid, srchei, seawid, seahei, outscale, step, outNum, Threshold);
	Sp->ComCreate();

	UINT spow = Sp->GetOutWid();
	UINT spoh = Sp->GetOutHei();
	if (searchOn)SearchMaxNum = Sp->GetSearchNum();
	else SearchMaxNum = BADGENUM;
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
	searchon = searchOn;

	sp[0] = new SP(InW, InH, Width, Height, 1.0f, 16, numNode[depth - 1], Threshold, searchOn);
	sp[1] = new SP(InW, InH, Width * 1.5, Height * 1.5, 0.67f, 24, numNode[depth - 1], Threshold, searchOn);
	sp[2] = new SP(InW, InH, Width * 2, Height * 2, 0.5f, 32, numNode[depth - 1], Threshold, searchOn);

	SearchMaxNum = sp[0]->SearchMaxNum;

	unsigned int wid = Width;
	unsigned int hei = Height;
	if (Type == 'C' || Type == 'D' || Type == 'S') {
		cn[0] = new DxConvolution(wid, hei, filNum, SearchMaxNum, 7, 2);
		cn[0]->ComCreateReLU();
		wid = cn[0]->GetOutWidth();
		hei = cn[0]->GetOutHeight();
		cn[0]->CreareNNTexture(7, 7, filNum);

		dcn[0].SetCommandList(0);
		dcn[0].GetVBarray2D(1);
		dcn[0].TextureInit(7, 7 * filNum);
		dcn[0].TexOn();
		dcn[0].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	}
	if (Type == 'C' || Type == 'P' || Type == 'D' || Type == 'S') {
		wid = Width;
		hei = Height;
		if (Type == 'C' || Type == 'D' || Type == 'S') {
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
	if (Type == 'D' || Type == 'S') {
		cn[1] = new DxConvolution(wid, hei, filNum, SearchMaxNum, 5, 1);
		cn[1]->ComCreateReLU();
		wid = cn[1]->GetOutWidth();
		hei = cn[1]->GetOutHeight();
		cn[1]->CreareNNTexture(5, 5, filNum);

		dcn[1].SetCommandList(0);
		dcn[1].GetVBarray2D(1);
		dcn[1].TextureInit(5, 5 * filNum);
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

	if (Type == 'S') {
		cn[2] = new DxConvolution(wid, hei, filNum, SearchMaxNum, 5, 1);
		cn[2]->ComCreateReLU();
		wid = cn[2]->GetOutWidth();
		hei = cn[2]->GetOutHeight();
		cn[2]->CreareNNTexture(5, 5, filNum);

		dcn[2].SetCommandList(0);
		dcn[2].GetVBarray2D(1);
		dcn[2].TextureInit(5, 5 * filNum);
		dcn[2].TexOn();
		dcn[2].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);

		po[2] = new DxPooling(wid, hei, filNum, SearchMaxNum);
		po[2]->ComCreate();
		wid = po[2]->GetOutWidth();
		hei = po[2]->GetOutHeight();
		po[2]->CreareNNTexture(wid, hei, filNum);

		dpo[2].SetCommandList(0);
		dpo[2].GetVBarray2D(1);
		dpo[2].TextureInit(wid, hei * filNum);
		dpo[2].TexOn();
		dpo[2].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	}

	numN = new UINT[depth + 1];
	numN[0] = wid * hei;
	for (int i = 1; i < depth + 1; i++)numN[i] = numNode[i - 1];
	nn = new DxNeuralNetwork(numN, depth + 1, filNum, SearchMaxNum);
	nn->ComCreateReLU();
	nn->SetLearningLate(0.12f);
	nn->CreareNNTexture(wid, hei, filNum);

	dnn.SetCommandList(0);
	dnn.GetVBarray2D(1);
	dnn.TextureInit(wid, hei * filNum);
	dnn.TexOn();
	dnn.CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	nnWidth = wid;
	nnHeight = hei;

	Depth = depth + 1;

	UINT p2dNum = SearchMaxNum;
	if (!searchon)p2dNum = 1;
	din = new PolygonData2D[p2dNum];
	for (UINT i = 0; i < p2dNum; i++) {
		din[i].SetCommandList(0);
		din[i].GetVBarray2D(1);
		din[i].TextureInit(Width, Height);
		din[i].TexOn();
		din[i].CreateBox(0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, TRUE, TRUE);
	}
	pixIn = new UINT**[p2dNum];
	for (UINT i = 0; i < p2dNum; i++) {
		pixIn[i] = new UINT*[Height];
	}
	for (UINT i = 0; i < p2dNum; i++) {
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

	UINT p2dNum = SearchMaxNum;
	if (!searchon)p2dNum = 1;
	for (UINT k = 0; k < p2dNum; k++) {
		for (UINT i = 0; i < Height; i++)ARR_DELETE(pixIn[k][i]);
	}
	for (UINT k = 0; k < p2dNum; k++)ARR_DELETE(pixIn[k]);
	ARR_DELETE(pixIn);
	ARR_DELETE(numN);
	S_DELETE(nn);
	S_DELETE(po[0]);
	S_DELETE(cn[0]);
	S_DELETE(po[1]);
	S_DELETE(cn[1]);
	S_DELETE(po[2]);
	S_DELETE(cn[2]);
	S_DELETE(sp[0]);
	S_DELETE(sp[1]);
	S_DELETE(sp[2]);
	ARR_DELETE(din);
	ARR_DELETE(posImageTrain);
	ARR_DELETE(negaImageTrain);
	ARR_DELETE(posImageTest);
	ARR_DELETE(negaImageTest);
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
		RunPoolingToConvolution(0);
		RunConvolutionToPooling(1);
		RunPoolingToNN(1);
	}
	if (Type == 'S') {
		RunConvolutionToPooling(0);
		RunPoolingToConvolution(0);
		RunConvolutionToPooling(1);
		RunPoolingToConvolution(1);
		RunConvolutionToPooling(2);
		RunPoolingToNN(2);
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
		RunPoolingToConvolutionDetec(0);
		RunConvolutionToPoolingDetec(1);
		RunPoolingToNNDetec(1);
	}
	if (Type == 'S') {
		RunConvolutionToPoolingDetec(0);
		RunPoolingToConvolutionDetec(0);
		RunConvolutionToPoolingDetec(1);
		RunPoolingToConvolutionDetec(1);
		RunConvolutionToPoolingDetec(2);
		RunPoolingToNNDetec(2);
	}
}

void ImageRecognition::queryTest() {
	if (Type == 'C' || Type == 'D') {
		RunConvolutionToPoolingTest(0);
	}
	if (Type == 'C' || Type == 'P') {
		RunPoolingToNNTest(0);
	}
	if (Type == 'D') {
		RunPoolingToConvolutionTest(0);
		RunConvolutionToPoolingTest(1);
		RunPoolingToNNTest(1);
	}
	if (Type == 'S') {
		RunConvolutionToPoolingTest(0);
		RunPoolingToConvolutionTest(0);
		RunConvolutionToPoolingTest(1);
		RunPoolingToConvolutionTest(1);
		RunConvolutionToPoolingTest(2);
		RunPoolingToNNTest(2);
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
	if (sp[spInd]->Search10cnt > 3) {
		for (int i = 0; i < sp[spInd]->SearchMaxNum; i++)sp[spInd]->Searchflg[i] = true;
		sp[spInd]->Search10cnt = 0;
	}
}

void ImageRecognition::LearningDecay(float in, float scale) {

	float c = 0.05f * pow((1.0f - in), 3) * scale;
	float c1 = 0.0005f * pow((1.0f - in), 3) * scale;
	float n = 0.01f * pow((1.0f - in), 3) * scale;

	if (Type == 'C' || Type == 'D' || Type == 'S')
		cn[0]->SetLearningLate(c, c1);
	if (Type == 'D' || Type == 'S')
		cn[1]->SetLearningLate(c, c1);
	if (Type == 'S')
		cn[2]->SetLearningLate(c, c1);
	nn->SetLearningLate(n);
}

void ImageRecognition::Training() {
	query();
	float *drop = new float[2];
	drop[0] = 0.0f;
	drop[1] = 0.5f;
	nn->SetdropThreshold(drop);
	ARR_DELETE(drop);
	nn->Training();

	if (Type == 'D') {
		NNToPoolingBackPropagation(1);
		PoolingToConvolutionBackPropagation(1);
		ConvolutionToPoolingBackPropagation(0);
		PoolingToConvolutionBackPropagation(0);
	}
	if (Type == 'P') {
		NNToPoolingBackPropagation(0);
	}
	if (Type == 'C') {
		NNToPoolingBackPropagation(0);
		PoolingToConvolutionBackPropagation(0);
	}
	if (Type == 'S') {
		NNToPoolingBackPropagation(2);
		PoolingToConvolutionBackPropagation(2);
		ConvolutionToPoolingBackPropagation(1);
		PoolingToConvolutionBackPropagation(1);
		ConvolutionToPoolingBackPropagation(0);
		PoolingToConvolutionBackPropagation(0);
	}

	currout = nn->GetOutputEl(0) * 100.0f;
}

void ImageRecognition::Test() {
	queryTest();
	float *drop = new float[2];
	drop[0] = 0.0f;
	drop[1] = 0.0f;
	nn->SetdropThreshold(drop);
	ARR_DELETE(drop);
	nn->Test();

	testOut = nn->GetOutputEl(0) * 100.0f;
}

void ImageRecognition::RunConvolutionToPooling(UINT ind) {
	cn[ind]->SetdropThreshold(0.0f);
	cn[ind]->Query();
	po[ind]->SetInputResource(cn[ind]->GetOutputResource());
}

void ImageRecognition::RunPoolingToConvolution(UINT ind) {
	po[ind]->Query();
	cn[ind + 1]->SetInputResource(po[ind]->GetOutputResource());
}

void ImageRecognition::RunPoolingToNN(UINT ind) {
	po[ind]->Query();
	nn->SetInputResource(po[ind]->GetOutputResource());
}

void ImageRecognition::RunConvolutionToPoolingDetec(UINT ind) {
	cn[ind]->Detection(sp[spInd]->SearchNum);
	po[ind]->SetInputResource(cn[ind]->GetOutputResource());
}

void ImageRecognition::RunPoolingToConvolutionDetec(UINT ind) {
	po[ind]->Detection(sp[spInd]->SearchNum);
	cn[ind + 1]->SetInputResource(po[ind]->GetOutputResource());
}

void ImageRecognition::RunPoolingToNNDetec(UINT ind) {
	po[ind]->Detection(sp[spInd]->SearchNum);
	nn->SetInputResource(po[ind]->GetOutputResource());
}

void ImageRecognition::RunConvolutionToPoolingTest(UINT ind) {
	cn[ind]->SetdropThreshold(0.0f);
	cn[ind]->Test();
	po[ind]->SetInputResource(cn[ind]->GetOutputResource());
}

void ImageRecognition::RunPoolingToConvolutionTest(UINT ind) {
	po[ind]->Test();
	cn[ind + 1]->SetInputResource(po[ind]->GetOutputResource());
}

void ImageRecognition::RunPoolingToNNTest(UINT ind) {
	po[ind]->Test();
	nn->SetInputResource(po[ind]->GetOutputResource());
}

void ImageRecognition::NNToPoolingBackPropagation(UINT ind) {
	po[ind]->SetInErrorResource(nn->GetOutErrorResource());
	po[ind]->Training();
}

void ImageRecognition::ConvolutionToPoolingBackPropagation(UINT ind) {
	po[ind]->SetInErrorResource(cn[ind + 1]->GetOutErrorResource());
	po[ind]->Training();
}

void ImageRecognition::PoolingToConvolutionBackPropagation(UINT ind) {
	cn[ind]->SetInErrorResource(po[ind]->GetOutErrorResource());
	cn[ind]->Training();
}

void ImageRecognition::SetLearningNum(UINT texNum, UINT ppmNum) {
	learTexNum = texNum;
	ppmPosNum = ppmNum;
	for (UINT i = 0; i < learTexNum; i++) {
		D3D12_RESOURCE_DESC texdesc = GetTexture(i)->GetDesc();
		UINT width = texdesc.Width;
		UINT height = texdesc.Height;
		UINT sepNum = (width / LEARTEXWID) * (height / LEARTEXWID);
		if (i < TextureLoader::GetlearningCorrectFaceFirstInd()) {
			negaNum += sepNum;
		}
		else {
			texPosNum += sepNum;
		}
	}
	posNum = texPosNum + ppmPosNum;
}

void ImageRecognition::CreateLearningImagebyte(float RateOftrainImage, BYTE *ppm) {

	srand((unsigned)time(NULL));

	BYTE *texPosImage = new BYTE[texPosNum * LEARTEXWID * LEARTEXWID];
	BYTE *negaImage = new BYTE[negaNum * LEARTEXWID * LEARTEXWID];
	int negaInd = 0;
	int posInd = 0;

	for (UINT i = 0; i < learTexNum; i++) {
		D3D12_SUBRESOURCE_DATA texResource;
		GetTextureUp(i)->Map(0, nullptr, reinterpret_cast<void**>(&texResource));
		unsigned char *ptex = (unsigned char*)texResource.pData;
		D3D12_RESOURCE_DESC texdesc = GetTexture(i)->GetDesc();
		UINT Wid = texdesc.Width;//画像サイズ
		UINT Hei = texdesc.Height;
		UINT sepW = Wid / LEARTEXWID;//画像個数
		UINT sepH = Hei / LEARTEXWID;
		UINT pixWidNum = Wid * 4;

		for (UINT y = 0; y < sepH; y++) {
			for (UINT x = 0; x < sepW; x++) {
				UINT heist = y * LEARTEXWID;
				UINT widst = x * LEARTEXWID;
				for (UINT hei = heist; hei < LEARTEXWID + heist; hei++) {
					for (UINT wid = widst; wid < LEARTEXWID + widst; wid++) {
						UINT pInd = pixWidNum * hei + wid * 4;
						UINT pt = (ptex[pInd + 0] + ptex[pInd + 1] + ptex[pInd + 2]) / 3;
						if (i < TextureLoader::GetlearningCorrectFaceFirstInd()) {
							negaImage[negaInd++] = pt;
						}
						else {
							texPosImage[posInd++] = pt;
						}
					}
				}
			}
		}

		GetTextureUp(i)->Unmap(0, nullptr);
	}

	//トレーニングデータ, テストデータを分ける
	UINT texPosTraNum = texPosNum * RateOftrainImage;
	UINT texPosTestNum = texPosNum - texPosTraNum;
	UINT ppmPosTraNum = ppmPosNum * RateOftrainImage;
	UINT ppmPosTestNum = ppmPosNum - ppmPosTraNum;

	posTraNum = texPosTraNum + ppmPosTraNum;
	negaTraNum = negaNum * RateOftrainImage;
	posTestNum = texPosTestNum + ppmPosTestNum;
	negaTestNum = negaNum - negaTraNum;

	posImageTrain = new BYTE[posTraNum * LEARTEXWID * LEARTEXWID];
	posImageTest = new BYTE[posTestNum * LEARTEXWID * LEARTEXWID];
	negaImageTrain = new BYTE[negaTraNum * LEARTEXWID * LEARTEXWID];
	negaImageTest = new BYTE[negaTestNum * LEARTEXWID * LEARTEXWID];

	size_t texPosTraSize = sizeof(BYTE) * texPosTraNum * LEARTEXWID * LEARTEXWID;
	size_t ppmPosTraSize = sizeof(BYTE) * ppmPosTraNum * LEARTEXWID * LEARTEXWID;
	size_t negaTraSize = sizeof(BYTE) * negaTraNum * LEARTEXWID * LEARTEXWID;
	size_t texPosTestSize = sizeof(BYTE) * texPosTestNum * LEARTEXWID * LEARTEXWID;
	size_t ppmPosTestSize = sizeof(BYTE) * ppmPosTestNum * LEARTEXWID * LEARTEXWID;
	size_t negaTestSize = sizeof(BYTE) * negaTestNum * LEARTEXWID * LEARTEXWID;

	memcpy(posImageTrain, texPosImage, texPosTraSize);
	memcpy(&posImageTrain[texPosTraSize], ppm, ppmPosTraSize);
	memcpy(negaImageTrain, negaImage, negaTraSize);
	memcpy(posImageTest, &texPosImage[texPosTraSize], texPosTestSize);
	memcpy(&posImageTest[texPosTestSize], &ppm[ppmPosTraSize], ppmPosTestSize);
	memcpy(negaImageTest, &negaImage[negaTraSize], negaTestSize);
	ARR_DELETE(texPosImage);
	ARR_DELETE(negaImage);
}

void ImageRecognition::LearningByteImage() {

	int byteInd = 0;
	if (++positivef > 1)positivef = 0;

	if (positivef == 0) {
		currentTarget = 0.99f;
		nn->SetTargetEl(0.99f, 0);
	}
	else {
		currentTarget = 0.01f;
		nn->SetTargetEl(0.01f, 0);
	}

	for (int k = 0; k < BADGENUM; k++) {

		if (positivef == 0) {
			byteInd = rand() % posTraNum;
			poscnt++;
		}
		else {
			byteInd = rand() % negaTraNum;
			negacnt++;
		}
		UINT imInd = byteInd * LEARTEXWID * LEARTEXWID;

		int rev = rand() % 2;
		for (UINT i = 0; i < LEARTEXWID * LEARTEXWID; i++) {

			UINT pixX = i % LEARTEXWID;
			if (rev == 0)pixX = LEARTEXWID - (i % LEARTEXWID) - 1;
			UINT pixY = i / LEARTEXWID;

			UINT traInd = LEARTEXWID * pixY + pixX;
			float el;
			if (positivef == 0) {
				el = ((float)posImageTrain[traInd + imInd] / 255.0f * 0.99f) + 0.01f;
			}
			else {
				el = ((float)negaImageTrain[traInd + imInd] / 255.0f * 0.99f) + 0.01f;
			}

			pixIn[0][pixY][pixX] = ((UINT)(el * 255.0f) << 16) + ((UINT)(el * 255.0f) << 8) + ((UINT)(el * 255.0f));

			switch (Type) {
			case 'C':
			case 'D':
			case 'S':
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
}

void ImageRecognition::TestByteImage() {

	int byteInd = 0;
	if (positivef == 0) {
		byteInd = rand() % posTestNum;
	}
	else {
		byteInd = rand() % negaTestNum;
	}

	for (UINT i = 0; i < LEARTEXWID * LEARTEXWID; i++) {
		float el;
		UINT imInd = byteInd * LEARTEXWID * LEARTEXWID;
		if (positivef == 0) {
			el = ((float)posImageTest[i + imInd] / 255.0f * 0.99f) + 0.01f;
		}
		else {
			el = ((float)negaImageTest[i + imInd] / 255.0f * 0.99f) + 0.01f;
		}

		switch (Type) {
		case 'C':
		case 'D':
		case 'S':
			cn[0]->FirstInput(el, i);
			break;
		case 'P':
			po[0]->FirstInput(el, i);
			break;
		case 'N':
			nn->FirstInput(el, i);
			break;
		}
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
			case 'S':
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
	dnn.Update(735.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 20.0f, 20.0f * filNum);
	dnn.CopyResource(nn->GetNNTextureResource(), nn->GetNNTextureResourceStates());
	dnn.Draw();
}

void ImageRecognition::PODraw() {
	if (Type == 'C' || Type == 'P' || Type == 'D' || Type == 'S') {
		dpo[0].Update(125.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 20.0f, 20.0f * filNum);
		dpo[0].CopyResource(po[0]->GetNNTextureResource(), po[0]->GetNNTextureResourceStates());
		dpo[0].Draw();
	}
	if (Type == 'D' || Type == 'S') {
		dpo[1].Update(365.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 20.0f, 20.0f * filNum);
		dpo[1].CopyResource(po[1]->GetNNTextureResource(), po[1]->GetNNTextureResourceStates());
		dpo[1].Draw();
	}
	if (Type == 'S') {
		dpo[2].Update(605.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 20.0f, 20.0f * filNum);
		dpo[2].CopyResource(po[2]->GetNNTextureResource(), po[2]->GetNNTextureResourceStates());
		dpo[2].Draw();
	}
}

void ImageRecognition::CNDraw() {
	if (Type == 'C' || Type == 'D' || Type == 'S') {
		dcn[0].Update(5.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 20.0f, 20.0f * filNum);
		dcn[0].CopyResource(cn[0]->GetNNTextureResource(), cn[0]->GetNNTextureResourceStates());
		dcn[0].Draw();
	}
	if (Type == 'D' || Type == 'S') {
		dcn[1].Update(245.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 20.0f, 20.0f * filNum);
		dcn[1].CopyResource(cn[1]->GetNNTextureResource(), cn[1]->GetNNTextureResourceStates());
		dcn[1].Draw();
	}
	if (Type == 'S') {
		dcn[2].Update(485.0f, 20.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 20.0f, 20.0f * filNum);
		dcn[2].CopyResource(cn[2]->GetNNTextureResource(), cn[2]->GetNNTextureResourceStates());
		dcn[2].Draw();
	}
}

void ImageRecognition::INDraw(float x, float y, float xsize, float ysize) {

	UINT cnt = 0;
	UINT p2dNum = sp[spInd]->SearchMaxNum;
	if (!searchon)p2dNum = 1;
	for (int i = 0; i < p2dNum; i++) {
		din[i].Update(cnt * 52.0f + x, 548.0f + y, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 52.0f + xsize, 52.0f + ysize);
		if (sp[spInd]->SearchMaxNum == BADGENUM || Threshold <= sp[spInd]->out[i]) {
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

	DxText::GetInstance()->UpDateText(L"入力画像 ", 10.0f, 480.0f, 15.0f, { 1.0f, 0.0f, 0.0f, 1.0f });
	if (Type != 'P' && Type != 'N')DxText::GetInstance()->UpDateText(L"畳込み層", 0.0f, 7.0f, 15.0f, { 1.0f, 0.5f, 0.5f, 1.0f });
	if (Type != 'N')DxText::GetInstance()->UpDateText(L"プーリング層", 100.0f, 7.0f, 15.0f, { 0.5f, 1.0f, 0.5f, 1.0f });
	DxText::GetInstance()->UpDateText(L"全結合層逆方向出力 ", 680.0f, 7.0f, 15.0f, { 0.5f, 0.5f, 1.0f, 1.0f });
	if (Type == 'D' || Type == 'S') {
		DxText::GetInstance()->UpDateText(L"畳込み層", 240.0f, 7.0f, 15.0f, { 1.0f, 0.5f, 0.5f, 1.0f });
		DxText::GetInstance()->UpDateText(L"プーリング層", 340.0f, 7.0f, 15.0f, { 0.5f, 1.0f, 0.5f, 1.0f });
	}
	if (Type == 'S') {
		DxText::GetInstance()->UpDateText(L"畳込み層", 480.0f, 7.0f, 15.0f, { 1.0f, 0.5f, 0.5f, 1.0f });
		DxText::GetInstance()->UpDateText(L"プーリング層", 580.0f, 7.0f, 15.0f, { 0.5f, 1.0f, 0.5f, 1.0f });
	}

	switch (stateNum) {
	case 1:
		DxText::GetInstance()->UpDateText(L"学習中出力 ", 600.0f, 400.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateValue(currout, 710.0f, 400.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateText(L"Target ", 600.0f, 415.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateValue(currentTarget * 100.0f, 710.0f, 415.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateText(L"誤差 ", 600.0f, 430.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		errer = abs((int)((currentTarget * 100.0f) - currout));
		DxText::GetInstance()->UpDateValue(errer, 710.0f, 430.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateText(L"正解画像学習 ", 600.0f, 445.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateText(L"不正解画像学習 ", 600.0f, 460.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateValue(poscnt, 750.0f, 445.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateValue(negacnt, 750.0f, 460.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateText(L"正解画像数 ", 600.0f, 475.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateText(L"不正解画像数 ", 600.0f, 490.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateValue(posNum, 750.0f, 475.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateValue(negaNum, 750.0f, 490.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		break;
	case 2:
		DxText::GetInstance()->UpDateText(L"顏である確率 ", 10.0f + x, 500.0f + y, 15.0f, { 0.0f, 1.0f, 0.0f, 1.0f });
		UINT cnt = 0;
		for (int i = 0; i < sp[spInd]->SearchMaxNum && cnt < 16; i++) {
			if (Threshold <= sp[spInd]->out[i]) {
				float sout = 0.0f;
				if (sp[spInd]->out[i] > 0.0f)sout = sp[spInd]->out[i];
				DxText::GetInstance()->UpDateValue(sout * 100, cnt * 52.0f + x, 530.0f + y, 15.0f, 3, { 0.0f, 1.0f, 0.0f, 1.0f });
				DxText::GetInstance()->UpDateText(L"%\ ", 40.0f + cnt * 52.0f + x, 530.0f + y, 15.0f, { 0.0f, 1.0f, 0.0f, 1.0f });
				cnt++;
			}
		}
		break;
	}
}

int ImageRecognition::Getcurrout() {
	return currout;
}

int ImageRecognition::Gettestout() {
	return testOut;
}

float ImageRecognition::Getcurrtar() {
	return currentTarget;
}

int ImageRecognition::Geterrer() {
	return errer;
}

void ImageRecognition::SaveData() {
	if (cn[0])cn[0]->SaveData(0);
	if (cn[1])cn[1]->SaveData(1);
	if (cn[2])cn[2]->SaveData(2);
	nn->SaveData();
}

void ImageRecognition::LoadData() {
	if (cn[0])cn[0]->LoadData(0);
	if (cn[1])cn[1]->LoadData(1);
	if (cn[2])cn[2]->LoadData(2);
	nn->LoadData();
}
