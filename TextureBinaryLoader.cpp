//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　    TextureBinaryLoaderクラス                               **//
//**                                                                                     **//
//*****************************************************************************************//

#include "TextureBinaryLoader.h"

Texture* TextureBinaryLoader::tex = NULL;
SearchFile* TextureBinaryLoader::sf = nullptr;
int TextureBinaryLoader::texNum = 0;
int TextureBinaryLoader::learningImageNum = 0;
int TextureBinaryLoader::learningCorrectFaceFirstInd = 0;
float* TextureBinaryLoader::target = nullptr;

void TextureBinaryLoader::TextureDecode(char* Bpass) {
	tex[texNum].texName = Bpass;
}

void TextureBinaryLoader::TextureLoad() {

	tex = new Texture[150];
	target = new float[150];
	texNum = 0;

	sf = new SearchFile(3);
	char** str = new char* [2];
	str[0] = "jpg";
	str[1] = "png";
	sf->Search(L"LearningImage/Incorrect/*", 0, str, 2);
	sf->Search(L"LearningImage/CorrectFace/*", 1, str, 2);
	sf->Search(L"TestImage/*", 2, str, 2);
	delete[]str;
	str = nullptr;
	UINT tarCnt = 0;
	for (UINT k = 0; k < 3; k++) {
		for (UINT j = 0; j < sf->GetFileNum(k); j++) {
			TextureDecode(sf->GetFileName(k, j));
			if (k == 0)target[tarCnt++] = 0.01f;
			if (k == 1)target[tarCnt++] = 0.99f;
			if (k == 1 && j == 0)learningCorrectFaceFirstInd = texNum;
			if (k == 2 && j == 0)learningImageNum = texNum;
			texNum++;
		}
	}
	TextureLoader::GetTexture2(texNum, tex, Dx12Process::GetInstance());
}

void TextureBinaryLoader::DeleteTextureStruct() {
	ARR_DELETE(tex);
	ARR_DELETE(target);
	S_DELETE(sf);
}

int TextureBinaryLoader::GetLearningImageNum() {
	return learningImageNum;
}

float* TextureBinaryLoader::GetLearningTarget() {
	return target;
}

int TextureBinaryLoader::GetTestImageNum() {
	return texNum - learningImageNum;
}

int TextureBinaryLoader::GetlearningCorrectFaceFirstInd() {
	return learningCorrectFaceFirstInd;
}