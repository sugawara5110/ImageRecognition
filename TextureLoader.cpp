//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　    TextureLoaderクラス                                     **//
//**                                                                                     **//
//*****************************************************************************************//

#include "TextureLoader.h"
#include <time.h>
#include <stdlib.h>

Texture *TextureLoader::tex = NULL;
SearchFile *TextureLoader::sf = nullptr;
int TextureLoader::texNum = 0;
int TextureLoader::learningImageNum = 0;
float *TextureLoader::target = nullptr;

void TextureLoader::TextureDecode(char *Bpass) {
	TextureDecode(Bpass, FALSE);
}

void TextureLoader::TextureDecode(char *Bpass, bool UpKeep) {
	tex[texNum].texName = Bpass;
	tex[texNum].UpKeep = UpKeep;
}

void TextureLoader::TextureLoad() {

	tex = new Texture[250];
	target = new float[250];
	texNum = 0;

	sf = new SearchFile(3);
	char **str = new char*[2];
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
			if (k == 2 && j == 0)learningImageNum = texNum;
			texNum++;
		}
	}

	srand((unsigned)time(NULL));
	for (int i = 0; i < learningImageNum * 100; i++) {
		int rnd1 = rand() % learningImageNum;
		int rnd2 = rand() % learningImageNum;
		Texture tmp;
		float tmpf;
		tmp = tex[rnd1];
		tmpf = target[rnd1];
		tex[rnd1] = tex[rnd2];
		target[rnd1] = target[rnd2];
		tex[rnd2] = tmp;
		target[rnd2] = tmpf;
	}

	Dx12Process::GetInstance()->SetTextureBinary(tex, texNum);
}

void TextureLoader::DeleteTextureStruct() {
	ARR_DELETE(tex);
	ARR_DELETE(target);
	S_DELETE(sf);
}

int TextureLoader::GetLearningImageNum() {
	return learningImageNum;
}

float *TextureLoader::GetLearningTarget() {
	return target;
}

int TextureLoader::GetTestImageNum() {
	return texNum - learningImageNum;
}