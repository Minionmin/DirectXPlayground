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

    // Direct3D�̏�����


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

    while (Window::pInstance_->bRunning_)
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
