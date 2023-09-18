//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　　　    Main                                                **//
//**                                                                                     **//
//*****************************************************************************************//

#include "../../../Common/Window/Win.h"
#include "../../../Common/Direct3DWrapperOption/DxText.h"
#include "ImageRecognition.h"
#include "TextureBinaryLoader.h"
#include "../../../Common/DirectShowWrapper\Camera.h"
#include "../../../CNN/Graph.h"
#include "../../../PPMLoader/PPMLoader.h"
#include "../../../T_float/T_float.h"
#pragma comment(lib,"winmm.lib")
#define COUNT 80000

//-------------------------------------------------------------
// アプリケーションのエントリポイント
// 引数
//		hInstance     : 現在のインスタンスのハンドル
//		hPrevInstance : 以前のインスタンスのハンドル(win16での別インスタンス確認用win32では常にNULL)
//		lpCmdLine	  : コマンドラインパラメータ
//		nCmdShow	  : ウィンドウの表示状態
// 戻り値
//		成功したら0以外の値
//-------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	HWND hWnd;
	MSG msg;

	Createwindow(&hWnd, hInstance, nCmdShow, 900, 600, L"ImageDetection");

	Dx_Device::InstanceCreate();
	Dx_Device::GetInstance()->createDevice();
	Dx_Device::GetInstance()->reportLiveDeviceObjectsOn();
	Dx_CommandManager::InstanceCreate();
	Dx_SwapChain::InstanceCreate();

	Dx_TextureHolder::InstanceCreate();
	Dx_TextureHolder* dx = Dx_TextureHolder::GetInstance();

	Dx_Device* dev = Dx_Device::GetInstance();
	dev->dxrCreateResource();
	Dx_SwapChain* sw = Dx_SwapChain::GetInstance();

	Dx_CommandManager::setNumResourceBarrier(1024);

	sw->Initialize(hWnd, 900, 800);

	sw->setPerspectiveFov(55, 1, 10000);
	Dx_Light::Initialize();
	Dx_ShaderHolder::CreateShaderByteCode();

	Dx_CommandManager* cMa = Dx_CommandManager::GetInstance();
	Dx_CommandListObj* d = cMa->getGraphicsComListObj(0);
	ID3D12GraphicsCommandList* CList = d->getCommandList();


	//dx->reportLiveDeviceObjectsOn();
	Camera* cam = nullptr;
	//test
	//Movie mov("aaa.avi");
	bool camOn = false;
	//文字入力
	DxText::InstanceCreate();
	DxText* text = DxText::GetInstance();
	TextureBinaryLoader::TextureLoad();
	Control* control;
	PPMLoader* ppm = nullptr;
	control = Control::GetInstance();
	DxInput* di = DxInput::GetInstance();
	di->create(hWnd);
	di->SetWindowMode(true);
	int learningImageNum = TextureBinaryLoader::GetLearningImageNum();
	float* target = TextureBinaryLoader::GetLearningTarget();
	/*
	dx->Bigin(0);
	Dx12Process::GetInstance()->GetTexture(0);
	dx->End(0);
	dx->WaitFenceCurrent();
	*/
	UINT* input = nullptr;
	ImageRecognition* nn = nullptr;;
	int cnt = 0;
	Graph* graph[2] = { nullptr };
	UINT state = 0;//0:タイトル, 1:学習モード, 2:検出モード
	UINT select = 0;
	bool enter = false;
	bool cancel = false;
	bool drawOn = false;
	float br0, br1, br2;
	float threshold = 0.0f;
	UINT testNum = 0;
	UINT testCnt = 0;
	float camTheta = 0;
	while (1) {//アプリ実行中ループ

		//実験
		/*dx->Bigin(0);
		DxConvolution *cn = nullptr;
		cn = new DxConvolution(2, 2, 1, true, 1, 3, 2);
		cn->SetdropThreshold(0.0f);
		cn->ComCreate(ReLU);
		dx->End(0);
		dx->WaitFenceCurrent();
		float in[] = {
			1,1,
			1,1, };
		cn->Input(in, 0, 0);

		cn->Query();
		float f1[9];
		for (int i = 0; i < 9; i++) {
			f1[i] = cn->OutputFilter(0, i);
			float f = f1[i];
		}

		float o1[16];
		for (int i = 0; i < 16; i++) {
			o1[i] = cn->OutputEl(0, i);
			float o = o1[i];
		}

		float inerr[] = {
			1,1,1,1,
			1,1,1,1,
			1,1,1,1,
			1,1,1,1 };
		cn->InputError(inerr, 0, 0);
		cn->Training();
		float o1er[4];
		for (int i = 0; i < 4; i++) {
			o1er[i] = cn->GetErrorEl(0, i);
			float oer = o1er[i];
		}

		S_DELETE(cn);*/
		//実験

		if (!DispatchMSG(&msg))break;
		Directionkey key = control->Direction();
		T_float::GetTime(hWnd);
		switch (key) {
		case UP:
			if (select == 0)break;
			select--;
			break;
		case DOWN:
			if (select == 2)break;
			select++;
			break;
		case ENTER:
			enter = true;
			break;
		case CANCEL:
			cancel = true;
			break;
		}
		switch (state) {
		case 0:
			switch (select) {
			case 0:
				br0 = 1.0f;
				br1 = 0.3f;
				br2 = 0.3f;
				break;
			case 1:
				br0 = 0.3f;
				br1 = 1.0f;
				br2 = 0.3f;
				break;
			case 2:
				br0 = 0.3f;
				br1 = 0.3f;
				br2 = 1.0f;
				break;
			}

			if (enter) {
				bool searchOn = true;
				switch (select) {
				case 0:
					state = 1;
					searchOn = false;
					threshold = 0.0f;
					break;
				case 1:
					state = 2;
					threshold = 0.9f;
					break;
				case 2:
					state = 2;
					camOn = true;
					cam = new Camera();
					threshold = 0.9f;
					break;
				}
				enter = false;
				d->Bigin();
				input = new UINT[2];
				input[0] = 64;
				input[1] = 1;
				nn = new ImageRecognition(512, 256, 64, 64, input, 2, 20, searchOn, threshold);
				nn->SetTarget(target);
				if (state == 1) {
					if (!nn->LoadDataSet()) {
						ppm = new PPMLoader(L"../../gazou/faceData/*", 64, 64, GRAYSCALE);
						nn->SetLearningNum(learningImageNum, ppm->GetFileNum());
						nn->CreateLearningImagebyte(0.7f, ppm->GetImageArr());
						//nn->SetLearningNum(learningImageNum, 0);
						//nn->CreateLearningImagebyte(0.7f, nullptr);
					}
					graph[0] = new Graph();
					graph[0]->CreateGraph(100, 218, 256, 128, 256, 256);
					graph[1] = new Graph();
					graph[1]->CreateGraph(357, 218, 256, 128, 256, 256);
				}
				d->End();
				cMa->RunGpu();
				cMa->WaitFence();
				if (state == 2)nn->LoadData();
			}
			DxText::GetInstance()->UpDateText(L"学習モード ", 100.0f, 100.0f, 15.0f, { 0.3f, 0.3f, br0, 1.0f });
			DxText::GetInstance()->UpDateText(L"検出モード テクスチャテスト", 100.0f, 120.0f, 15.0f, { 0.3f, 0.3f, br1, 1.0f });
			DxText::GetInstance()->UpDateText(L"検出モード カメラテスト", 100.0f, 140.0f, 15.0f, { 0.3f, 0.3f, br2, 1.0f });
			break;
		case 1:
			//学習
			if (cnt < COUNT) {
				nn->LearningByteImage();
				nn->LearningDecay((float)cnt / (float)COUNT, 3.0f);
				nn->Training();
				nn->TestByteImage();
				nn->Test();
				cnt++;
			}

			if (cancel) {
				cancel = false;
				state = 0;
				nn->SaveData();
				nn->SaveDataSet();
				ARR_DELETE(input);
				S_DELETE(nn);
				S_DELETE(graph[0]);
				S_DELETE(graph[1]);
				drawOn = false;
				cnt = 0;
				break;
			}
			DxText::GetInstance()->UpDateText(L"メニューに戻る場合はDelete", 550.0f, 550.0f, 15.0f, { 0.3f, 1.0f, 0.3f, 1.0f });
			DxText::GetInstance()->UpDateText(L"顔画像検出AI実験中 ", 130.0f, 370.0f, 30.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
			DxText::GetInstance()->UpDateText(L"Training ", 130.0f, 348.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
			DxText::GetInstance()->UpDateText(L"Test ", 390.0f, 348.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
			drawOn = true;
			break;
		case 2:
			//検出
			if (!camOn) {
				nn->InputTexture(learningImageNum + testNum);
			}
			else nn->InputPixel(cam->GetFrame1(512, 256));

			testCnt++;
			if (testCnt > 20) {
				testCnt = 0;
				testNum++;
			}
			if (testNum >= TextureBinaryLoader::GetTestImageNum())testNum = 0;
			nn->QueryGradCAM();
			if (cancel) {
				cancel = false;
				state = 0;
				ARR_DELETE(input);
				S_DELETE(nn);
				S_DELETE(graph[0]);
				S_DELETE(graph[1]);
				S_DELETE(cam);
				camOn = false;
				drawOn = false;
				cnt = 0;
				break;
			}
			DxText::GetInstance()->UpDateText(L"メニューに戻る場合はDelete", 550.0f, 500.0f, 15.0f, { 0.3f, 1.0f, 0.3f, 1.0f });
			DxText::GetInstance()->UpDateText(L"顔画像検出AI実験中 ", 100.0f, 400.0f, 60.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
			drawOn = true;
			break;
		}
		using namespace CoordTf;
		MATRIX camThetaZ;
		VECTOR3 cam1 = { 0, -170, 0 };
		MatrixRotationZ(&camThetaZ, camTheta * 360.0f);
		VectorMatrixMultiply(&cam1, &camThetaZ);
		sw->Cameraset(cam1, { 0, 0, 0 });
		if ((camTheta += 0.01f) > 360.0f)camTheta = 0.0f;

		d->Bigin();
		sw->BiginDraw(0, true);
		if (state != 0 && drawOn) {
			if (state == 1) {
				nn->NNDraw();
			}
			nn->SPDraw();
			nn->INDraw(0, 0.0f, 0.0f, 0.0f, 0.0f);
			nn->textDraw(state, 0.0f, 0.0f);
			if (state == 1) {
				float tmw = (float)cnt / (float)COUNT * 255.0f;
				/*float tmh = (float)nn->Getcurrout() / 100.0f * 255.0f;
				float tmhtes1 = (float)nn->Gettestout1() / 100.0f * 255.0f;
				float tmhtes = (float)nn->Gettestout() / 100.0f * 255.0f;
				if (nn->Getcurrtar() >= 0.5f) {
					graph[0]->SetData((int)tmw, (int)tmh, 0xffffffff);
					graph[1]->SetData((int)tmw, (int)tmhtes1, 0xff808080);
					graph[1]->SetData((int)tmw, (int)tmhtes, 0xffffffff);
				}
				else {
					graph[0]->SetData((int)tmw, (int)tmh, 0xff0000ff);
					graph[1]->SetData((int)tmw, (int)tmhtes1, 0xff00ff00);
					graph[1]->SetData((int)tmw, (int)tmhtes, 0xff0000ff);
				}*/

				float cerr = nn->GetcrossEntropyError() * 100.0f;
				float cerrTest = nn->GetcrossEntropyErrorTest() * 100.0f;
				if (cerr > 255.0f)cerr = 255.0f;
				if (cerrTest > 255.0f)cerrTest = 255.0f;
				graph[0]->SetData((int)tmw, (int)cerr, 0xffffffff);
				graph[1]->SetData((int)tmw, (int)cerrTest, 0xff0000ff);
				graph[0]->Draw(0);
				graph[1]->Draw(0);
			}
		}
		if (state == 1) {
			DxText::GetInstance()->UpDateText(L"学習回数 ", 600.0f, 510.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
			DxText::GetInstance()->UpDateValue(cnt, 670.0f, 510, 15.0f, 5, { 1.0f, 1.0f, 1.0f, 1.0f });
		}
		text->UpDate();
		text->Draw(0);
		sw->EndDraw(0);
		d->End();
		cMa->RunGpu();
		cMa->WaitFence();
		sw->DrawScreen();
	}
	ARR_DELETE(input);
	S_DELETE(nn);
	S_DELETE(cam);
	S_DELETE(graph[0]);
	S_DELETE(graph[1]);
	S_DELETE(ppm);
	TextureBinaryLoader::DeleteTextureStruct();
	DxInput::DeleteInstance();
	Control::DeleteInstance();
	DxText::DeleteInstance();
	Dx_SwapChain::DeleteInstance();
	Dx_TextureHolder::DeleteInstance();
	Dx_CommandManager::DeleteInstance();
	Dx_Device::DeleteInstance();
	return (int)msg.wParam;
}