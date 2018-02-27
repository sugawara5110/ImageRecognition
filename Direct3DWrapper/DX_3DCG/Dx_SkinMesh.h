//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　        SkinMeshクラス                                      **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_SkinMesh_Header
#define Class_SkinMesh_Header

#include "Dx12ProcessCore.h"
#include <fbxsdk.h>

class SkinMesh_sub {

protected:
	friend SkinMesh;

	FbxScene *m_pmyScene = NULL;
	float end_frame, current_frame;

	bool centering, offset;
	float cx, cy, cz;
	MATRIX rotZYX;
	float connect_step;

	SkinMesh_sub();
	~SkinMesh_sub();
	bool Create(CHAR *szFileName);
};

class SkinMesh :public Common {

protected:
	friend SkinMesh_sub;
	friend Dx12Process;
	static FbxManager *m_pSdkManager;
	static FbxImporter *m_pImporter;
	//InitFBX排他処理用
	static volatile bool stInitFBX_ON, stSetNewPose_ON;
	static bool ManagerCreated;//マネージャー生成フラグ

	int                        com_no = 0;
	ID3DBlob                   *vs = nullptr;
	ID3DBlob                   *vsB = nullptr;
	ID3DBlob                   *hs = nullptr;
	ID3DBlob                   *ds = nullptr;
	ID3DBlob                   *ps = nullptr;
	ID3DBlob                   *psB = nullptr;
	bool alpha = FALSE;
	bool blend = FALSE;

	D3D_PRIMITIVE_TOPOLOGY primType_draw, primType_drawB;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvHeap = nullptr;
	int texNum;//テクスチャー数

    //コンスタントバッファOBJ
	UploadBuffer<CONSTANT_BUFFER> *mObjectCB0 = nullptr;
	UploadBuffer<CONSTANT_BUFFER2> *mObjectCB1 = nullptr;
	UploadBuffer<SHADER_GLOBAL_BONES> *mObject_BONES = nullptr;
	CONSTANT_BUFFER cb[2];
	SHADER_GLOBAL_BONES sgb[2];
	int sw = 0;
	//UpLoadカウント
	int upCount = 0;
	//初回Up終了
	bool UpOn = FALSE;
	//DrawOn
	bool DrawOn = FALSE;

	std::unique_ptr<VertexView> Vview = nullptr;
	std::unique_ptr<IndexView[]> Iview = nullptr;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSO = nullptr;//パイプラインOBJ
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSO_B = nullptr;//パイプラインOBJ(バンプマップ)

	//メッシュ関連	
	DWORD *m_pdwNumVert;//メッシュ毎の頂点数
	DWORD VerAllpcs;   //全頂点数
	MY_VERTEX_S *pvVB;//使用後保持するか破棄するかフラグで決める,通常は破棄
	bool pvVB_delete_f;

	int MateAllpcs;  //全マテリアル数
	MY_MATERIAL_S *m_pMaterial;

	//一時格納用
	DWORD *m_pMaterialCount;//メッシュ毎のマテリアルカウント
	DWORD *pdwNumFace; //メッシュ毎のポリゴン数
	int *IndexCount34Me;  //4頂点ポリゴン分割前のメッシュ毎のインデックス数
	int IndexCount34MeAll;
	int *IndexCount3M;  //4頂点ポリゴン分割後のマテリアル毎のインデックス数
	int **pIndex;      //メッシュ毎のインデックス配列(4頂点ポリゴン分割後)

	//ボーン
	int m_iNumBone;
	BONE *m_BoneArray;

	//FBX
	SkinMesh_sub *fbx;
	FbxCluster **m_ppCluster;//ボーン情報
	char *m_pClusterName;
	FbxNode **m_ppNodeArray;//各Nodeへのポインタ配列
	int NodeArraypcs;
	FbxNode **m_ppSubAnimationBone;//その他アニメーションボーンポインタ配列
	MATRIX *m_pLastBoneMatrix;
	int AnimLastInd;
	float BoneConnect;

	static std::mutex mtx;
	static void Lock() { mtx.lock(); }
	static void Unlock() { mtx.unlock(); }

	void DestroyFBX();
	FbxScene* GetScene(int p);
	int SearchNodeCount(FbxNode *pnode, FbxNodeAttribute::EType SearchType);
	FbxNode *SearchNode(FbxNode *pnode, FbxNodeAttribute::EType SearchType, int Ind);
	HRESULT InitFBX(CHAR* szFileName, int p);
	void CreateIndexBuffer(int cnt, int *pIndex, int IviewInd);
	void CreateIndexBuffer2(int *pIndex, int IviewInd);
	HRESULT ReadSkinInfo(MY_VERTEX_S *pvVB);
	MATRIX GetCurrentPoseMatrix(int index);
	void MatrixMap_Bone(SHADER_GLOBAL_BONES *sbB);
	void GetTexture();
	bool SetNewPoseMatrices(float time, int ind);
	void CreateRotMatrix(float thetaZ, float thetaY, float thetaX, int ind);
	void CbSwap();

public:
	//共通で使うマネージャー生成(外部で生成解放を行う)
	static void CreateManager();
	static void DeleteManager();

	SkinMesh();
	~SkinMesh();

	void SetCommandList(int no);
	void SetState(bool alpha, bool blend);
	void ObjCentering(bool f, int ind);
	void ObjCentering(float x, float y, float z, float thetaZ, float thetaY, float thetaX, int ind);
	void ObjOffset(float x, float y, float z, float thetaZ, float thetaY, float thetaX, int ind);
	void SetConnectStep(int ind, float step);
	void Vertex_hold();
	HRESULT GetFbx(CHAR* szFileName);
	void GetBuffer(float end_frame);
	void SetVertex();
	void SetDiffuseTextureName(char *textureName, int materialIndex);
	void SetNormalTextureName(char *textureName, int materialIndex);
	void CreateFromFBX(bool disp);
	void CreateFromFBX();
	HRESULT GetFbxSub(CHAR* szFileName, int ind);
	HRESULT GetBuffer_Sub(int ind, float end_frame);
	void CreateFromFBX_SubAnimation(int ind);
	bool Update(float time, float x, float y, float z, float r, float g, float b, float a, float thetaZ, float thetaY, float thetaX, float size);
	bool Update(int ind, float time, float x, float y, float z, float r, float g, float b, float a, float thetaZ, float thetaY, float thetaX, float size, float disp = 1.0f);
	void DrawOff();
	void Draw();
	VECTOR3 GetVertexPosition(int verNum, float adjustZ, float adjustY, float adjustX, float thetaZ, float thetaY, float thetaX, float scale);
};

#endif