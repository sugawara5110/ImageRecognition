//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　　　    Main                                                **//
//**                                                                                     **//
//*****************************************************************************************//

#include "../../../Common/Window/Win.h"
#include "../../../Common/Direct3DWrapper/Dx12Process.h"
#include "../../../Common/Direct3DWrapper/DxText.h"
#include "ImageRecognition.h"
#include "TextureLoader.h"
#include "../../../Common/DirectShowWrapper\Camera.h"
#pragma comment(lib,"winmm.lib")

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
	//DirectX12ラッパー
	Dx12Process *dx;
	//文字入力
	DxText *text;

	Createwindow(&hWnd, hInstance, nCmdShow, 800, 600, L"ImageDetection");

	//Dx12Processオブジェクト生成
	Dx12Process::InstanceCreate();
	//Dx11Processオブジェクト取得
	dx = Dx12Process::GetInstance();
	dx->Initialize(hWnd);
	Camera *cam = nullptr;
	//test
	//Movie mov("aaa.avi");
	bool camOn = false;
	DxText::InstanceCreate();
	text = DxText::GetInstance();
	TextureLoader::TextureLoad();
	Control *control;
	control = Control::GetInstance();
	int learningImageNum = TextureLoader::GetLearningImageNum();
	float *target = TextureLoader::GetLearningTarget();

	dx->Bigin(0);
	Dx12Process::GetInstance()->GetTexture(0);
	dx->End(0);
	dx->WaitFenceCurrent();

	UINT *input = nullptr;
	ImageRecognition *nn = nullptr;;
	int cnt = 0;

	UINT state = 0;//0:タイトル, 1:学習モード, 2:検出モード
	UINT select = 0;
	bool enter = false;
	bool cancel = false;
	bool drawOn = false;
	float br0, br1, br2;
	float threshold = 0.0f;
	while (1) {//アプリ実行中ループ
		if (!DispatchMSG(&msg))break;
		Directionkey key = control->Direction();
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
					threshold = 0.99f;
					break;
				case 2:
					state = 2;
					camOn = true;
					cam = new Camera();
					threshold = 0.99f;
					break;
				}
				enter = false;
				dx->Bigin(0);
				input = new UINT[2];
				input[0] = 200;
				input[1] = 1;
				nn = new ImageRecognition(512, 320, 64, 64, input, 2, 4, 'D', searchOn, threshold);
				nn->SetTarget(target);
				nn->SetLearningNum(learningImageNum);
				dx->End(0);
				dx->WaitFenceCurrent();
				if (state == 2)nn->LoadData();
			}
			DxText::GetInstance()->UpDateText(L"学習モード ", 100.0f, 100.0f, 15.0f, { 0.3f, 0.3f, br0, 1.0f });
			DxText::GetInstance()->UpDateText(L"検出モード テクスチャテスト", 100.0f, 120.0f, 15.0f, { 0.3f, 0.3f, br1, 1.0f });
			DxText::GetInstance()->UpDateText(L"検出モード カメラテスト", 100.0f, 140.0f, 15.0f, { 0.3f, 0.3f, br2, 1.0f });
			break;
		case 1:
			//学習
			if (cnt < 3000) {
				nn->LearningTexture();
				nn->Training();
				cnt++;
			}

			if (cancel) {
				cancel = false;
				state = 0;
				nn->SaveData();
				ARR_DELETE(input);
				S_DELETE(nn);
				drawOn = false;
				cnt = 0;
				break;
			}
			DxText::GetInstance()->UpDateText(L"メニューに戻る場合はDelete", 550.0f, 500.0f, 15.0f, { 0.3f, 1.0f, 0.3f, 1.0f });
			DxText::GetInstance()->UpDateText(L"顔画像検出AI実験中 ", 100.0f, 400.0f, 60.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
			drawOn = true;
			break;
		case 2:
			//検出
			if (!camOn) {
				nn->searchPixel(learningImageNum);
			}
			else nn->InputPixel(cam->GetFrame1(512, 320));
			nn->Query();
			if (cancel) {
				cancel = false;
				state = 0;
				ARR_DELETE(input);
				S_DELETE(nn);
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

		dx->Bigin(0);
		dx->Sclear(0);
		if (state != 0 && drawOn) {
			nn->NNDraw();
			nn->PODraw();
			nn->CNDraw();
			nn->SPDraw();
			nn->INDraw(0.0f, 0.0f, 0.0f, 0.0f);
			nn->textDraw(state, 0.0f, 0.0f);
		}
		if (state == 1) {
			DxText::GetInstance()->UpDateText(L"学習回数 ", 600.0f, 480.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
			DxText::GetInstance()->UpDateValue(cnt, 670.0f, 480, 15.0f, 5, { 1.0f, 1.0f, 1.0f, 1.0f });
		}
		text->UpDate();
		text->Draw(0);
		dx->End(0);
		dx->WaitFenceCurrent();
		dx->DrawScreen();
	}
	ARR_DELETE(input);
	S_DELETE(nn);
	S_DELETE(cam);
	TextureLoader::DeleteTextureStruct();
	DxText::DeleteInstance();
	Dx12Process::DeleteInstance();
	return (int)msg.wParam;
}