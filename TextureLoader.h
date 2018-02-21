//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　    TextureLoaderクラス                                     **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_TextureLoader_Header
#define Class_TextureLoader_Header

#include "./Direct3DWrapper/Dx12Process.h"
#include "SearchFile\SearchFile.h"

class TextureLoader {

private:
	static Texture *tex;
	static SearchFile *sf;
	static int texNum;
	static int learningImageNum;
	static float *target;

	TextureLoader() {}
	static void TextureDecode(char *Bpass);
	static void TextureDecode(char *Bpass, bool UpKeep);

public:
	static void TextureLoad();
	static void DeleteTextureStruct();
	static int GetLearningImageNum();
	static float *GetLearningTarget();
	static int GetTestImageNum();
};

#endif
