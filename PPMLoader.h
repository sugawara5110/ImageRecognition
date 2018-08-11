//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　      PPMLoaderクラス                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_PPMLoader_Header
#define Class_PPMLoader_Header

#include "../../../Common/Direct3DWrapper/Dx12Process.h"
#include "../../../Common/SearchFile\SearchFile.h"

class PPMLoader {

private:
	SearchFile * sf = nullptr;
	UINT fileNum = 0;
	BYTE *image = nullptr;

	PPMLoader() {}

public:
	PPMLoader(UINT outW, UINT outH);
	~PPMLoader();
	UINT GetFileNum();
	BYTE *GetImageArr();
};

#endif
