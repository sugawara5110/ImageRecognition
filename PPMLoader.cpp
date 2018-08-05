//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　      PPMLoaderクラス                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#include "PPMLoader.h"

PPMLoader::PPMLoader(UINT inW, UINT inH, UINT outW, UINT outH) {

	sf = new SearchFile(1);
	char **str = new char*[1];
	str[0] = "ppm";
	sf->Search(L"LearningImage/CorrectFacePPM/*", 0, str, 1);
	delete[]str;
	str = nullptr;
	fileNum = sf->GetFileNum(0);
	image = new BYTE[fileNum * outW * outH];

	BYTE *tmpimage = new BYTE[inW * 3 * inH];
	size_t size = sizeof(BYTE) * inW * 3 * inH;
	long offset = inW * 3 * inH;

	for (UINT k = 0; k < fileNum; k++) {
		FILE *fp = nullptr;
		char *name = sf->GetFileName(0, k);
		fopen_s(&fp, name, "rt");
		fseek(fp, -offset, SEEK_END);
		//ここからpixdata, RGBの順に1byteずつ1ピクセル3byte
		fread(tmpimage, size, 1, fp);
		//サイズ変換, グレースケール変換
		for (UINT y = 0; y < outH; y++) {
			for (UINT x = 0; x < outW; x++) {

				UINT inHeiInd = inW * 3 * (inH / outH * y);
				UINT inWidInd = inW / outW * x * 3;
				UINT inInd = inHeiInd + inWidInd;
				BYTE gray = (tmpimage[inInd] + tmpimage[inInd + 1] + tmpimage[inInd + 2]) / 3;//grayscale
				image[outH * outW * k + outW * y + x] = gray;
			}
		}
		fclose(fp);
	}
	ARR_DELETE(tmpimage);
}

PPMLoader::~PPMLoader() {
	S_DELETE(sf);
	ARR_DELETE(image);
}

UINT PPMLoader::GetFileNum() {
	return fileNum;
}

BYTE *PPMLoader::GetImageArr() {
	return image;
}