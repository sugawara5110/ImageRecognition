//*****************************************************************************************//
//**                                                                                     **//
//**                   �@�@�@�@�@    Main                                                **//
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
// �A�v���P�[�V�����̃G���g���|�C���g
// ����
//		hInstance     : ���݂̃C���X�^���X�̃n���h��
//		hPrevInstance : �ȑO�̃C���X�^���X�̃n���h��(win16�ł̕ʃC���X�^���X�m�F�pwin32�ł͏��NULL)
//		lpCmdLine	  : �R�}���h���C���p�����[�^
//		nCmdShow	  : �E�B���h�E�̕\�����
// �߂�l
//		����������0�ȊO�̒l
//-------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	HWND hWnd;
	MSG msg;
	//DirectX12���b�p�[
	Dx12Process *dx;
	//��������
	DxText *text;

	Createwindow(&hWnd, hInstance, nCmdShow, 800, 600, L"ImageDetection");

	//Dx12Process�I�u�W�F�N�g����
	Dx12Process::InstanceCreate();
	//Dx11Process�I�u�W�F�N�g�擾
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

	UINT state = 0;//0:�^�C�g��, 1:�w�K���[�h, 2:���o���[�h
	UINT select = 0;
	bool enter = false;
	bool cancel = false;
	bool drawOn = false;
	float br0, br1, br2;
	float threshold = 0.0f;
	while (1) {//�A�v�����s�����[�v
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
			DxText::GetInstance()->UpDateText(L"�w�K���[�h ", 100.0f, 100.0f, 15.0f, { 0.3f, 0.3f, br0, 1.0f });
			DxText::GetInstance()->UpDateText(L"���o���[�h �e�N�X�`���e�X�g", 100.0f, 120.0f, 15.0f, { 0.3f, 0.3f, br1, 1.0f });
			DxText::GetInstance()->UpDateText(L"���o���[�h �J�����e�X�g", 100.0f, 140.0f, 15.0f, { 0.3f, 0.3f, br2, 1.0f });
			break;
		case 1:
			//�w�K
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
			DxText::GetInstance()->UpDateText(L"���j���[�ɖ߂�ꍇ��Delete", 550.0f, 500.0f, 15.0f, { 0.3f, 1.0f, 0.3f, 1.0f });
			DxText::GetInstance()->UpDateText(L"��摜���oAI������ ", 100.0f, 400.0f, 60.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
			drawOn = true;
			break;
		case 2:
			//���o
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
			DxText::GetInstance()->UpDateText(L"���j���[�ɖ߂�ꍇ��Delete", 550.0f, 500.0f, 15.0f, { 0.3f, 1.0f, 0.3f, 1.0f });
			DxText::GetInstance()->UpDateText(L"��摜���oAI������ ", 100.0f, 400.0f, 60.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
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
			DxText::GetInstance()->UpDateText(L"�w�K�� ", 600.0f, 480.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
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