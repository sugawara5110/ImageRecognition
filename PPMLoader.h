//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　      PPMLoaderクラス                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_PPMLoader_Header
#define Class_PPMLoader_Header

#include "../../../Common/Direct3DWrapper/DX_3DCG/Dx12ProcessCore.h"
#include "../../../Common/SearchFile\SearchFile.h"

class PPMLoader {

private:
	SearchFile* sf = nullptr;
	UINT fileNum = 0;
	BYTE* image = nullptr;

	PPMLoader() {}

public:
	PPMLoader(wchar_t* pass, UINT outW, UINT outH);
	~PPMLoader();
	UINT GetFileNum();
	BYTE* GetImageArr();
};

#endif
