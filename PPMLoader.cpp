//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　      PPMLoaderクラス                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#include "PPMLoader.h"

PPMLoader::PPMLoader(wchar_t *pass, UINT outW, UINT outH) {

	sf = new SearchFile(1);
	char **str = new char*[1];
	str[0] = "ppm";
	sf->Search(pass, 0, str, 1);
	delete[]str;
	str = nullptr;
	fileNum = sf->GetFileNum(0);
	image = new BYTE[fileNum * outW * outH];

	BYTE *tmpimage;
	size_t size;
	long offset;
	UINT fileCount = 0;

	for (UINT k = 0; k < fileNum; k++) {
		FILE *fp = nullptr;
		char line[200] = { 0 };//1行読み込み用
		int inW = 0;
		int inH = 0;
		int pix = 0;
		char *name = sf->GetFileName(0, k);
		fopen_s(&fp, name, "rt");
		fgets(line, sizeof(line), fp);//1行飛ばす
		fgets(line, sizeof(line), fp);
		if (line[0] == '#')fgets(line, sizeof(line), fp);//コメントだった場合
		sscanf_s(line, "%d %d", &inW, &inH);
		UINT inNum = inW * 3 * inW;
		tmpimage = new BYTE[inNum];
		size = sizeof(BYTE) * inNum;
		offset = inNum;
		fgets(line, sizeof(line), fp);
		sscanf_s(line, "%d", &pix);
		fseek(fp, -offset, SEEK_END);
		//ここからpixdata, RGBの順に1byteずつ1ピクセル3byte
		fread(tmpimage, size, 1, fp);
		//サイズ変換, グレースケール変換
		for (UINT y = 0; y < outH; y++) {
			for (UINT x = 0; x < outW; x++) {
				UINT inHeiInd = (UINT)((float)inH / outH * y) * inW * 3;
				UINT inWidInd = (float)inW / outW * x * 3;
				UINT inInd = inHeiInd + inWidInd;
				BYTE gray = (tmpimage[inInd] + tmpimage[inInd + 1] + tmpimage[inInd + 2]) / 3;//grayscale
				image[outH * outW * fileCount + outW * y + x] = gray;
			}
		}
		fileCount++;
		fclose(fp);
		ARR_DELETE(tmpimage);
	}
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