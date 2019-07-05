//*****************************************************************************************//
//**                                                                                     **//
//**                              ImageRecognition                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "ImageRecognition.h"
#include "TextureLoader.h"

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

ImageRecognition::ImageRecognition(UINT srcWid, UINT srcHei, UINT width, UINT height, UINT* numNode, int depth, UINT filnum, bool searchOn, float threshold) {

	Threshold = threshold;
	Width = width;
	Height = height;
	filNum = filnum;
	InW = srcWid;
	InH = srcHei;
	searchon = searchOn;
	Depth = depth + 1;

	sp[0] = new SP(InW, InH, Width, Height, 1.0f, 16, numNode[depth - 1], Threshold, searchOn);
	sp[1] = new SP(InW, InH, Width * 1.5, Height * 1.5, 0.666667f, 24, numNode[depth - 1], Threshold, searchOn);
	sp[2] = new SP(InW, InH, Width * 2, Height * 2, 0.5f, 32, numNode[depth - 1], Threshold, searchOn);

	SearchMaxNum = sp[0]->SearchMaxNum;

	Layer layer[7];
	UINT layerCnt = 0;

	layer[0].mapWid = width;
	layer[0].mapHei = height;
	layer[0].maxThread = SearchMaxNum;
	layer[layerCnt].layerName = CONV;
	layer[layerCnt].acName = ReLU;
	layer[layerCnt].NumFilter = filnum;
	layer[layerCnt].NumConvFilterWid = 7;
	layer[layerCnt++].NumConvFilterSlide = 1;

	layer[layerCnt].layerName = POOL;
	layer[layerCnt++].NumFilter = filnum;

	layer[layerCnt].layerName = CONV;
	layer[layerCnt].acName = ReLU;
	layer[layerCnt].NumFilter = filnum * 2;
	layer[layerCnt].NumConvFilterWid = 5;
	layer[layerCnt++].NumConvFilterSlide = 1;

	layer[layerCnt].layerName = POOL;
	layer[layerCnt++].NumFilter = filnum * 2;

	layer[layerCnt].layerName = CONV;
	layer[layerCnt].acName = ReLU;
	layer[layerCnt].NumFilter = filnum * 4;
	layer[layerCnt].NumConvFilterWid = 3;
	layer[layerCnt++].NumConvFilterSlide = 1;

	layer[layerCnt].layerName = POOL;
	layer[layerCnt++].NumFilter = filnum * 4;

	layer[layerCnt].layerName = AFFINE;
	layer[layerCnt].acName = ReLU;
	layer[layerCnt].topAcName = CrossEntropySigmoid;
	layer[layerCnt].NumFilter = filnum * 4;
	layer[layerCnt].numNode[0] = 64;
	layer[layerCnt].numNode[1] = 1;
	layer[layerCnt++].NumDepthNotInput = 2;

	cnn = new CNN(srcWid, srcHei, layer, layerCnt);

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
	pixIn = new UINT * *[p2dNum];
	for (UINT i = 0; i < p2dNum; i++) {
		pixIn[i] = new UINT * [Height];
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
	S_DELETE(cnn);
	ARR_DELETE(din);
	ARR_DELETE(posImageTrain);
	ARR_DELETE(negaImageTrain);
	ARR_DELETE(posImageTest);
	ARR_DELETE(negaImageTest);
}

void ImageRecognition::SetTarget(float *tar) {
	target = tar;
}

void ImageRecognition::Query() {
	cnn->Detection(sp[spInd]->SearchNum);

	UINT cnt = 0;
	for (int i = 0; i < sp[spInd]->SearchMaxNum; i++) {
		if (sp[spInd]->SearchOutInd[i] == -1)continue;
		sp[spInd]->out[i] = cnn->GetOutputEl(0, cnt);
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

void ImageRecognition::QueryGradCAM() {
	UINT srcMap = 64;
	UINT mapsli = 1;
	switch (spInd) {
	case 0:
		srcMap = 64;
		mapsli = 16;
		break;
	case 1:
		srcMap = 96;
		mapsli = 24;
		break;
	case 2:
		srcMap = 128;
		mapsli = 32;
		break;
	}

	cnn->DetectionGradCAM(sp[spInd]->SearchNum, srcMap, mapsli);

	UINT cnt = 0;
	for (int i = 0; i < sp[spInd]->SearchMaxNum; i++) {
		if (sp[spInd]->SearchOutInd[i] == -1)continue;
		sp[spInd]->out[i] = cnn->GetOutputEl(0, cnt);
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
	//test
	for (int i = 0; i < sp[spInd]->SearchMaxNum; i++)sp[spInd]->Searchflg[i] = true;
}

void ImageRecognition::LearningDecay(float in, float scale) {

	float c = 0.10f * pow((1.0f - in), 3) * scale;
	float n = 0.05f * pow((1.0f - in), 3) * scale;
	cnn->SetLearningLate(n, c);
}

void ImageRecognition::Training() {
	cnn->Training();

	currout = 0;
	for (int i = 0; i < BADGENUM; i++)
		currout += cnn->GetOutputEl(0, i) * 100.0f;
	currout /= BADGENUM;
}

void ImageRecognition::Test() {
	cnn->Test();

	if (currentTarget == 0.99f) {
		testCountp = testCountp % BADGENUM;
		testOut1 = testOutpArr[testCountp] = cnn->GetOutputEl(0) * 100.0f;
		int sum = 0;
		for (int i = 0; i < BADGENUM; i++)
			sum += testOutpArr[i];
		testOut = sum / BADGENUM;
		testCountp++;
	}
	else {
		testCountn = testCountn % BADGENUM;
		testOut1 = testOutnArr[testCountn] = cnn->GetOutputEl(0) * 100.0f;
		int sum = 0;
		for (int i = 0; i < BADGENUM; i++)
			sum += testOutnArr[i];
		testOut = sum / BADGENUM;
		testCountn++;
	}
}

void ImageRecognition::SetLearningNum(UINT texNum, UINT ppmNum) {
	learTexNum = texNum;
	ppmPosNum = ppmNum;
	for (UINT i = 0; i < learTexNum; i++) {
		D3D12_RESOURCE_DESC texdesc = GetTexture(i)->GetDesc();
		UINT width = texdesc.Width;
		UINT height = texdesc.Height;
		UINT sepNum = (width / Width) * (height / Height);
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

	BYTE *posImage = new BYTE[posNum * Width * Height];
	BYTE *negaImage = new BYTE[negaNum * Width * Height];
	int negaInd = 0;
	int posInd = 0;

	for (UINT i = 0; i < learTexNum; i++) {
		D3D12_SUBRESOURCE_DATA texResource;
		GetTextureUp(i)->Map(0, nullptr, reinterpret_cast<void**>(&texResource));
		unsigned char *ptex = (unsigned char*)texResource.pData;
		D3D12_RESOURCE_DESC texdesc = GetTexture(i)->GetDesc();
		UINT Wid = texdesc.Width;//画像サイズ
		UINT Hei = texdesc.Height;
		UINT sepW = Wid / Width;//画像個数
		UINT sepH = Hei / Height;
		UINT pixWidNum = Wid * 4;

		for (UINT y = 0; y < sepH; y++) {
			for (UINT x = 0; x < sepW; x++) {
				UINT heist = y * Height;
				UINT widst = x * Width;
				for (UINT hei = heist; hei < Height + heist; hei++) {
					for (UINT wid = widst; wid < Width + widst; wid++) {
						UINT pInd = pixWidNum * hei + wid * 4;
						UINT pt = (ptex[pInd + 0] + ptex[pInd + 1] + ptex[pInd + 2]) / 3;
						if (i < TextureLoader::GetlearningCorrectFaceFirstInd()) {
							negaImage[negaInd++] = pt;
						}
						else {
							posImage[posInd++] = pt;
						}
					}
				}
			}
		}

		GetTextureUp(i)->Unmap(0, nullptr);
	}

	//ppmData付け足す
	memcpy(&posImage[posInd], ppm, sizeof(BYTE) * ppmPosNum * Width * Height);

	//学習データシャッフル
	BYTE *tmp = new BYTE[Width * Height];
	size_t tmpSize = sizeof(BYTE) * Width * Height;
	for (UINT i = 0; i < posNum; i++) {
		int ind1 = (rand() % posNum) * Width * Height;
		int ind2 = (rand() % posNum) * Width * Height;
		memcpy(tmp, &posImage[ind1], tmpSize);
		memcpy(&posImage[ind1], &posImage[ind2], tmpSize);
		memcpy(&posImage[ind2], tmp, tmpSize);
	}
	for (UINT i = 0; i < negaNum; i++) {
		int ind1 = (rand() % negaNum) * Width * Height;
		int ind2 = (rand() % negaNum) * Width * Height;
		memcpy(tmp, &negaImage[ind1], tmpSize);
		memcpy(&negaImage[ind1], &negaImage[ind2], tmpSize);
		memcpy(&negaImage[ind2], tmp, tmpSize);
	}
	ARR_DELETE(tmp);

	//トレーニングデータ, テストデータを分ける
	posTraNum = posNum * RateOftrainImage;
	posTestNum = posNum - posTraNum;
	negaTraNum = negaNum * RateOftrainImage;
	negaTestNum = negaNum - negaTraNum;

	posImageTrain = new BYTE[posTraNum * Width * Height];
	posImageTest = new BYTE[posTestNum * Width * Height];
	negaImageTrain = new BYTE[negaTraNum * Width * Height];
	negaImageTest = new BYTE[negaTestNum * Width * Height];

	size_t posTraSize = sizeof(BYTE) * posTraNum * Width * Height;
	size_t negaTraSize = sizeof(BYTE) * negaTraNum * Width * Height;
	size_t posTestSize = sizeof(BYTE) * posTestNum * Width * Height;
	size_t negaTestSize = sizeof(BYTE) * negaTestNum * Width * Height;

	memcpy(posImageTrain, posImage, posTraSize);
	memcpy(negaImageTrain, negaImage, negaTraSize);
	memcpy(posImageTest, &posImage[posTraNum * Width * Height], posTestSize);
	memcpy(negaImageTest, &negaImage[negaTraNum * Width * Height], negaTestSize);
	ARR_DELETE(posImage);
	ARR_DELETE(negaImage);

	//LearningImagebyteContrastAdjustment(posImageTrain, posTraNum);
	LearningImagebyteContrastAdjustment(posImageTest, posTestNum);
}

void ImageRecognition::LearningImagebyteContrastAdjustment(BYTE *arr, UINT imageNum) {

	for (UINT k = 0; k < imageNum; k++) {
		BYTE max = 0;
		BYTE min = 255;
		for (UINT i = 0; i < Width * Height; i++) {
			UINT ind = Width * Height * k + i;
			if (arr[ind] > max)max = arr[ind];
			if (arr[ind] < min)min = arr[ind];
		}
		for (UINT i = 0; i < Width * Height; i++) {
			UINT ind = Width * Height * k + i;
			arr[ind] = (float)(arr[ind] - min) / (max - min) * 255;
		}
	}
}

void ImageRecognition::LearningByteImage() {

	int byteInd = 0;
	if (++positivef > 1)positivef = 0;

	if (positivef == 0) {
		currentTarget = 0.99f;
		cnn->SetTargetEl(0.99f, 0);
	}
	else {
		currentTarget = 0.01f;
		cnn->SetTargetEl(0.01f, 0);
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
		UINT imInd = byteInd * Width * Height;

		int rev = rand() % 2;
		for (UINT i = 0; i < Width * Height; i++) {

			UINT pixX = i % Width;
			if (rev == 0)pixX = Width - (i % Width) - 1;
			UINT pixY = i / Height;

			float el;
			if (positivef == 0) {
				el = ((float)posImageTrain[i + imInd] / 255.0f * 0.99f) + 0.01f;
			}
			else {
				el = ((float)negaImageTrain[i + imInd] / 255.0f * 0.99f) + 0.01f;
			}

			if (positivef == 0)pixIn[0][pixY][pixX] = ((UINT)(el * 255.0f) << 16) + ((UINT)(el * 255.0f) << 8) + ((UINT)(el * 255.0f));

			cnn->FirstInput(el, i, k);
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

	for (UINT i = 0; i < Width * Height; i++) {
		float el;
		UINT imInd = byteInd * Width * Height;
		if (positivef == 0) {
			el = ((float)posImageTest[i + imInd] / 255.0f * 0.99f) + 0.01f;
		}
		else {
			el = ((float)negaImageTest[i + imInd] / 255.0f * 0.99f) + 0.01f;
		}

		cnn->FirstInput(el, i);
	}
}

void ImageRecognition::searchPixel() {
	sp[spInd]->Sp->SetPixel(sp[spInd]->spPix);
	sp[spInd]->Sp->SeparationTexture();
	UINT seaNum = sp[spInd]->SearchMaxNum;
	float* oneImage = new float[Width * Height];

	/*	float max = 0.0f;
		float min = 1.0f;
		for (int i = 0; i < seaNum * Width * Height; i++) {
			if (sp[spInd]->Sp->GetOutputEl(i) > max)max = sp[spInd]->Sp->GetOutputEl(i);
			if (sp[spInd]->Sp->GetOutputEl(i) < min)min = sp[spInd]->Sp->GetOutputEl(i);
		}*/
	UINT cnt = 0;
	UINT seacnt = 0;
	for (UINT k = 0; k < seaNum; k++) {
		if (!sp[spInd]->Searchflg[k]) {
			sp[spInd]->SearchOutInd[k] = -1;
			cnt += (Width * Height);
			continue;
		}
		sp[spInd]->SearchOutInd[k] = k;
		for (UINT i = 0; i < Width * Height; i++)oneImage[i] = sp[spInd]->Sp->GetOutputEl(cnt++);
		//test
		float max = 0.0f;
		float min = 1.0f;
		for (int i = 0; i < Width * Height; i++) {
			if (oneImage[i] > max)max = oneImage[i];
			if (oneImage[i] < min)min = oneImage[i];
		}
		//test
		searchPixelContrastAdjustment(oneImage, max, min);
		for (UINT i = 0; i < Width * Height; i++) {
			float el = oneImage[i];
			UINT pixX = i % Width;
			UINT pixY = i / Width;
			pixIn[seacnt][pixY][pixX] = ((UINT)(el * 255.0f) << 16) + ((UINT)(el * 255.0f) << 8) + ((UINT)(el * 255.0f));
			cnn->FirstInput(el, i, seacnt);
		}
		seacnt++;
	}
	sp[spInd]->SearchNum = seacnt;
	ARR_DELETE(oneImage);
}

void ImageRecognition::searchPixelContrastAdjustment(float *arr, float max, float min) {

	for (UINT i = 0; i < Width * Height; i++) {
		arr[i] = (arr[i] - min) / (max - min);
	}
}

void ImageRecognition::InputTexture(int Tno) {
	spInd++;
	if (spInd > 2)spInd = 0;
	D3D12_SUBRESOURCE_DATA texResource;
	GetTextureUp(Tno)->Map(0, nullptr, reinterpret_cast<void**>(&texResource));
	UCHAR* ptex = (UCHAR*)texResource.pData;
	for (UINT i = 0; i < InW * InH; i++) {
		UCHAR tmp = (ptex[i * 4] + ptex[i * 4 + 1] + ptex[i * 4 + 2]) / 3;
		sp[spInd]->spPix[i] = ((float)tmp / 255.0f * 0.99f) + 0.01f;
	}
	GetTextureUp(Tno)->Unmap(0, nullptr);

	sp[spInd]->Sp->SetPixel3ch(GetTexture(Tno));
	cnn->SetPixel3ch(GetTexture(Tno));
	searchPixel();
}

void ImageRecognition::InputPixel(BYTE* pix) {
	spInd++;
	if (spInd > 2)spInd = 0;
	for (UINT i = 0; i < InW * InH; i++) {
		BYTE tmp = (pix[i * 4] + pix[i * 4 + 1] + pix[i * 4 + 2]) / 3;
		sp[spInd]->spPix[i] = ((float)tmp / 255.0f * 0.99f) + 0.01f;
	}

	sp[spInd]->Sp->SetPixel3ch(pix);
	cnn->SetPixel3ch(pix);
	searchPixel();
}

void ImageRecognition::NNDraw() {
	cnn->TrainingDraw();
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
	/*sp[spInd]->dsp[0].Update(50.0f, 200.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 300.0f, 200.0f);
	sp[spInd]->dsp[0].CopyResource(sp[spInd]->Sp->GetNNTextureResource(), sp[spInd]->Sp->GetNNTextureResourceStates());
	sp[spInd]->dsp[0].Draw();*/
	sp[spInd]->dsp[1].Update(350.0f, 200.0f, 0.8f, 1.0f, 1.0f, 1.0f, 1.0f, 300.0f, 200.0f);
	sp[spInd]->dsp[1].CopyResource(sp[spInd]->Sp->GetOutputResource(), sp[spInd]->Sp->GetNNTextureResourceStates());
	sp[spInd]->dsp[1].Draw();
	cnn->GradCAMDraw();
}

void ImageRecognition::textDraw(UINT stateNum, float x, float y) {

	if (stateNum == 0)return;
	switch (stateNum) {
	case 1:
		DxText::GetInstance()->UpDateText(L"test誤差 ", 600.0f, 370.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateValue((int)(cnn->GetcrossEntropyErrorTest() * 100.0f), 710.0f, 370.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateText(L"誤差 ", 600.0f, 385.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DxText::GetInstance()->UpDateValue((int)(cnn->GetcrossEntropyError() * 100.0f), 710.0f, 385.0f, 15.0f, 2, { 1.0f, 1.0f, 1.0f, 1.0f });
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

int ImageRecognition::Gettestout1() {
	return testOut1;
}

float ImageRecognition::Getcurrtar() {
	return currentTarget;
}

int ImageRecognition::Geterrer() {
	return errer;
}

float ImageRecognition::GetcrossEntropyError() {
	return cnn->GetcrossEntropyError();
}

float ImageRecognition::GetcrossEntropyErrorTest() {
	return cnn->GetcrossEntropyErrorTest();
}

void ImageRecognition::SaveData() {
	cnn->SaveData();
}

void ImageRecognition::LoadData() {
	cnn->LoadData();
}

void ImageRecognition::SaveDataSet() {
	FILE *fp = nullptr;
	FILE *fp2 = nullptr;
	UINT oneNum = Width * Height;
	size_t oneSize = oneNum * sizeof(BYTE);

	fp = fopen("datasetByte/dataset.da", "wb");
	BYTE *save = new BYTE[(posTraNum + posTestNum + negaTraNum + negaTestNum) * oneNum];

	memcpy(save, posImageTrain, posTraNum * oneSize);
	memcpy(&save[posTraNum * oneNum], posImageTest, posTestNum * oneSize);
	memcpy(&save[(posTraNum + posTestNum) * oneNum], negaImageTrain, negaTraNum * oneSize);
	memcpy(&save[(posTraNum + posTestNum + negaTraNum) * oneNum], negaImageTest, negaTestNum * oneSize);
	fwrite(save, (posTraNum + posTestNum + negaTraNum + negaTestNum) * oneSize, 1, fp);

	fclose(fp);
	ARR_DELETE(save);

	fp2 = fopen("datasetByte/datasetnum.da", "wb");
	UINT datasetNum[6];
	datasetNum[0] = posTraNum;
	datasetNum[1] = posTestNum;
	datasetNum[2] = negaTraNum;
	datasetNum[3] = negaTestNum;
	datasetNum[4] = posNum;
	datasetNum[5] = negaNum;
	fwrite(datasetNum, sizeof(UINT) * 6, 1, fp2);
	fclose(fp2);
}

bool ImageRecognition::LoadDataSet() {
	FILE *fp = nullptr;
	FILE *fp2 = nullptr;
	UINT oneNum = Width * Height;
	size_t oneSize = oneNum * sizeof(BYTE);
	fp = fopen("datasetByte/dataset.da", "rb");
	fp2 = fopen("datasetByte/datasetnum.da", "rb");
	if (!fp || !fp2) return false;

	UINT datasetNum[6];
	fread(datasetNum, sizeof(UINT) * 6, 1, fp2);
	posTraNum = datasetNum[0];
	posTestNum = datasetNum[1];
	negaTraNum = datasetNum[2];
	negaTestNum = datasetNum[3];
	posNum = datasetNum[4];
	negaNum = datasetNum[5];
	fclose(fp2);

	BYTE *load = new BYTE[(posTraNum + posTestNum + negaTraNum + negaTestNum) * oneNum];
	fread(load, (posTraNum + posTestNum + negaTraNum + negaTestNum) * oneSize, 1, fp);

	posImageTrain = new BYTE[posTraNum * oneNum];
	posImageTest = new BYTE[posTestNum * oneNum];
	negaImageTrain = new BYTE[negaTraNum * oneNum];
	negaImageTest = new BYTE[negaTestNum * oneNum];

	memcpy(posImageTrain, load, posTraNum * oneSize);
	memcpy(posImageTest, &load[posTraNum * oneNum], posTestNum * oneSize);
	memcpy(negaImageTrain, &load[(posTraNum + posTestNum) * oneNum], negaTraNum * oneSize);
	memcpy(negaImageTest, &load[(posTraNum + posTestNum + negaTraNum) * oneNum], negaTestNum * oneSize);

	fclose(fp);
	ARR_DELETE(load);
	return true;
}