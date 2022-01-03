//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　    TextureBinaryLoaderクラス                               **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_TextureBinaryLoader_Header
#define Class_TextureBinaryLoader_Header

#include "../../../Common/Direct3DWrapper/Dx12Process.h"
#include "../../../Common/SearchFile\SearchFile.h"
#include "../../../Common/TextureLoader/TextureLoader.h"

class TextureBinaryLoader {

private:
	static Texture* tex;
	static SearchFile* sf;
	static int texNum;
	static int learningImageNum;
	static int learningCorrectFaceFirstInd;
	static float* target;

	TextureBinaryLoader() {}
	static void TextureDecode(char* Bpass);

public:
	static void TextureLoad();
	static void DeleteTextureStruct();
	static int GetLearningImageNum();
	static float* GetLearningTarget();
	static int GetTestImageNum();
	static int GetlearningCorrectFaceFirstInd();
};

#endif
