//*****************************************************************************************//
//**                                                                                     **//
//**                   �@�@�@�@�@    Main                                                **//
//**                                                                                     **//
//*****************************************************************************************//

#include "Win.h"
#include "./Direct3DWrapper/Dx12Process.h"
#include "./Direct3DWrapper/DxText.h"
#include "ImageRecognition.h"
#include "TextureLoader.h"
#include "DirectShowWrapper\Camera.h"
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

	Createwindow(&hWnd, hInstance, nCmdShow);

	//Dx12Process�I�u�W�F�N�g����
	Dx12Process::InstanceCreate();
	//Dx11Process�I�u�W�F�N�g�擾
	dx = Dx12Process::GetInstance();
	dx->Initialize(hWnd);
	Camera *cam = nullptr;
	bool camOn = false;
	DxText::InstanceCreate();
	text = DxText::GetInstance();
	TextureLoader::TextureLoad();
	Control *control;
	control = Control::GetInstance();
	int learningImageNum = TextureLoader::GetLearningImageNum();
	int testImageNum = TextureLoader::GetTestImageNum();
	if (testImageNum > 16)testImageNum = 16;
	float *target = TextureLoader::GetLearningTarget();

	dx->Bigin(0);
	Dx12Process::GetInstance()->GetTexture(0);
	dx->End(0);
	dx->WaitFenceCurrent();

	UINT *input = nullptr;
	ImageRecognition *nn = nullptr;;

	int loop = 0;
	int cnt = 0;

	UINT state = 0;//0:�^�C�g��, 1:�w�K���[�h, 2:���o���[�h
	UINT select = 0;
	bool enter = false;
	bool cancel = false;
	bool drawOn = false;
	float br0, br1, br2;
	int testnum = 1;
	int loopstep;
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

				switch (select) {
				case 0:
					state = 1;
					testnum = 1;
					break;
				case 1:
					state = 2;
					testnum = testImageNum;
					break;
				case 2:
					state = 2;
					testnum = 1;
					camOn = true;
					cam = new Camera();
					break;
				}
				enter = false;
				dx->Bigin(0);
				input = new UINT[2];
				input[0] = 200;
				input[1] = 1;
				nn = new ImageRecognition(64, 64, input, 2, 2, 'D', testnum);
				dx->End(0);
				dx->WaitFenceCurrent();
			}
			DxText::GetInstance()->UpDateText(L"�w�K���[�h ", 100.0f, 100.0f, 15.0f, { 0.3f, 0.3f, br0, 1.0f });
			DxText::GetInstance()->UpDateText(L"���o���[�h �e�N�X�`���e�X�g", 100.0f, 120.0f, 15.0f, { 0.3f, 0.3f, br1, 1.0f });
			DxText::GetInstance()->UpDateText(L"���o���[�h �J�����e�X�g", 100.0f, 140.0f, 15.0f, { 0.3f, 0.3f, br2, 1.0f });
			break;
		case 1:
			//�w�K
			if (cnt < learningImageNum * 2) {
				if (target[loop] == 0.01f)loopstep = 1;
				else loopstep = 2;
				for (int i = 0; i < 4; i += loopstep) {
					nn->SetTargetEl(target[loop], 0);
					nn->InputTexture(loop, i);
					nn->Training();
				}
				loop++;
				loop = loop % learningImageNum;
				cnt++;
			}
			if (cnt == learningImageNum * 3) {
				nn->SaveData();
			}
			if (cancel) {
				cancel = false;
				state = 0;
				ARR_DELETE(input);
				S_DELETE(nn);
				drawOn = false;
				loop = 0;
				cnt = 0;
				break;
			}
			DxText::GetInstance()->UpDateText(L"���j���[�ɖ߂�ꍇ��Delete", 550.0f, 500.0f, 15.0f, { 0.3f, 1.0f, 0.3f, 1.0f });
			drawOn = true;
			break;
		case 2:
			//���o
			loop++;
			loop = loop % testImageNum;
			nn->LoadData();
			if (!camOn)
				nn->InputTexture(learningImageNum + loop, 0);
			else nn->InputPixel(cam->GetFrame(64, 64), 64, 64);
			nn->Query();
			if (cancel) {
				cancel = false;
				state = 0;
				ARR_DELETE(input);
				S_DELETE(nn);
				S_DELETE(cam);
				camOn = false;
				drawOn = false;
				loop = 0;
				break;
			}
			DxText::GetInstance()->UpDateText(L"���j���[�ɖ߂�ꍇ��Delete", 550.0f, 500.0f, 15.0f, { 0.3f, 1.0f, 0.3f, 1.0f });
			drawOn = true;
			break;
		}

		dx->Bigin(0);
		dx->Sclear(0);
		if (state != 0 && drawOn) {
			nn->NNDraw();
			nn->PODraw();
			nn->CNDraw();
			if (!camOn) {
				nn->INDraw(0.0f, 0.0f, 0.0f, 0.0f);
				nn->textDraw(state, 0.0f, 0.0f);
			}
			else {
				nn->INDraw(150.0f, -250.0f, 200.0f, 200.0f);
				nn->textDraw(state, 150.0f, -200.0f);
			}
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