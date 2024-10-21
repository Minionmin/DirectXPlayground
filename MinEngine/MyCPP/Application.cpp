#include "Application.h"

Application::Application(){}

Application::~Application(){}

bool Application::Initialize(HINSTANCE hInstance, int nShowCmd)
{
    // �E�B���h�E�̏�����
	if(!window_.Initialize(hInstance, nShowCmd, false))
	{
		return false;
	}

    // Application���I�u�U�[�o�[�ɂ��āAWindow����̃C���v�b�g���󂯎���āA�ʂ̃N���X�Ƃ̏���������
    window_.SetKeyCallback([this](WPARAM wParam) { HandleKeyInput(wParam); });
    window_.SetMouseCallback([this](UINT msg) { HandleMouseInput(msg); });

    // Direct3D�̏�����
    if (!renderer_.Initialize(window_.GetHWND())) {
        return false;
    }

	return true;
}

void Application::Run()
{
	Mainloop();
}

void Application::Mainloop()
{
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    while (Window::GetInstance()->GetRunning())
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg); // �E�B���h�E�v���V�[�W���Ƀ��b�Z�[�W�𑗂�
        }
        else {
            // �Q�[�����W�b�N
            //Update();
            //Render();
        }
    }
}

void Application::HandleKeyInput(WPARAM wParam)
{
    switch (wParam) {
    case 'S':
        renderer_.SetSolidMode(!renderer_.GetSolidMode());
        break;
    case 'W':
        renderer_.SetWireframeMode(!renderer_.GetWireframeMode());
        break;
    case 'N':
        renderer_.SetDrawNormals(!renderer_.GetDrawNormals());
        break;
    }
}

void Application::HandleMouseInput(UINT msg)
{
	switch (msg) {
    case WM_LBUTTONDOWN:
    {
    }
    case WM_RBUTTONDOWN: // �}�E�X���N���b�N�ŃJ��������]�\��Ԃɂ���
    {
        renderer_.SetCameraRotate(true);
        break;
    }
	}
}
